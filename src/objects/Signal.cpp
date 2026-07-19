/**
 * @file
 * @brief Audio signal class implementation
 * @author Oliver Dixon
 * @date 2026-06-24
 */

#include "Signal.hpp"

#include <cassert>
#include <cmath>

#include "../Logger.hpp"
#include "factories/SignalFactory.hpp"

namespace echomap
{

template <> constexpr std::string_view Object<Signal>::class_name = "Signal";

Signal::Signal(
    const std::string_view name,
    const std::optional<Source>& source
) :
    Object(name),
    fs_source(source)
{
}

bool Signal::Source::operator<(
        const Source& other
) const
{
    if (channel < other.channel)
        return true;

    return path < other.path;
}

std::uint64_t Signal::get_sample_count() const noexcept
{
    return samples.size();
}

const std::optional<Signal::Source>& Signal::observe_source() const noexcept
{
    return fs_source;
}

Signal::Sample::TimeT Signal::get_time_offset() const noexcept
{
    return timing_baseline.time_offset;
}

std::size_t Signal::get_sample_rate() const noexcept
{
    return timing_baseline.sample_rate;
}

decltype(Signal::samples)::const_iterator Signal::begin() const
{
    return samples.begin();
}

decltype(Signal::samples)::const_iterator Signal::end() const
{
    return samples.end();
}

decltype(Signal::samples)::const_iterator Signal::cbegin() const noexcept
{
    return samples.cbegin();
}

decltype(Signal::samples)::const_iterator Signal::cend() const noexcept
{
    return samples.cend();
}

Signal::Sample::AmplitudeT Signal::operator[](
    const std::size_t index
) const noexcept
{
    return samples[index];
}

std::span<const Signal::Sample::AmplitudeT> Signal::amplitudes() const noexcept
{
    return samples;
}

bool Signal::is_uniformly_sampled() const noexcept
{
    return !time_offsets.has_value();
}

Signal::Sample::TimeT Signal::get_time_at_index(
    const std::size_t index
) const noexcept
{
    assert(index < samples.size());
    const Sample::TimeT baseline_time =
        timing_baseline.time_offset + static_cast<Sample::TimeT>(index) * timing_baseline.sample_rate_r;
    return time_offsets.has_value() ? baseline_time + (*time_offsets)[index] : baseline_time;
}

Signal::Signal(
    const Signal& old_signal
) :
    Object(CopyTag{},
           old_signal),
    samples(old_signal.samples),
    timing_baseline(old_signal.timing_baseline),
    fs_source(old_signal.fs_source),
    time_offsets(old_signal.time_offsets)
{
}

Signal::Signal(
    const Signal& old_signal,
    const std::string_view new_name
) :
    Object(CopyTag{},
           old_signal,
           new_name),
    samples(old_signal.samples),
    timing_baseline(old_signal.timing_baseline),
    fs_source(old_signal.fs_source),
    time_offsets(old_signal.time_offsets)
{
}

void Signal::emplace_sample(
    const Sample::AmplitudeT amplitude
)
{
    assert(amplitude >= normalised_range.first && amplitude <= normalised_range.second);
    samples.emplace_back(amplitude);
    if (time_offsets.has_value())
        time_offsets->emplace_back(0);

    if (fs_source.has_value())
        fs_source->dirty = true;
}

void Signal::emplace_sample(
    const Sample::TimeT time,
    const Sample::AmplitudeT amplitude
)
{
    assert(amplitude >= normalised_range.first && amplitude <= normalised_range.second);
    samples.emplace_back(amplitude);
    emplace_time(time);

    if (fs_source.has_value())
        fs_source->dirty = true;
}

void Signal::emplace_sample(
    const Sample& sample
)
{
    emplace_sample(sample.time, sample.amplitude);
}

void Signal::emplace_sample_from_source(
    const Sample::AmplitudeT amplitude
)
{
    assert(amplitude >= normalised_range.first && amplitude <= normalised_range.second);
    samples.emplace_back(amplitude);
    if (time_offsets.has_value())
        time_offsets->emplace_back(0);
}

void Signal::emplace_sample_from_source(
    const Sample::TimeT time,
    const Sample::AmplitudeT amplitude
)
{
    assert(amplitude >= normalised_range.first && amplitude <= normalised_range.second);
    samples.emplace_back(amplitude);
    emplace_time(time);
}

void Signal::emplace_sample_from_source(
    const Sample& sample
)
{
    return emplace_sample_from_source(sample.time, sample.amplitude);
}

void Signal::reserve_samples(
    const std::size_t count
)
{
    samples.reserve(count);

    if (time_offsets.has_value())
        time_offsets->reserve(count);
}

void Signal::set_source(
    const std::filesystem::path& path,
    const std::size_t channel
)
{
    fs_source = Source(path, channel);
}

void Signal::set_time_offset(
    const Sample::TimeT new_time_offset
) noexcept
{
    timing_baseline.time_offset = new_time_offset;
}

void Signal::set_sample_rate(
    const std::size_t new_sample_rate
) noexcept
{
    timing_baseline.sample_rate = new_sample_rate;
    timing_baseline.sample_rate_r = new_sample_rate == 0 ? 0 : 1.0f / static_cast<float>(new_sample_rate);
}

void Signal::emplace_time(
    const Sample::TimeT given_time
)
{
    assert(!samples.empty());

    constexpr Sample::TimeT epsilon = 1.0e-6f;
    const auto expected =
        timing_baseline.time_offset + static_cast<float>(samples.size() - 1) * timing_baseline.sample_rate_r;

    // Check the time derived from the baseline, as if we're continuing with a uniformly sampled signal.
    if (const auto offset = given_time - expected; std::abs(offset) <= epsilon) {
        /*
         * If the given time matches what we expect, we don't need to do anything. Only update the explicit offsets
         * (with an offset of zero) if they're already there.
         */
        if (time_offsets.has_value())
            time_offsets->emplace_back(0.0f);
    } else {
        /*
         * If the given time doesn't match what we expect, then a variably sampled entry has been introduced. We need to
         * consider:
         *
         *  1. if insertion of the time would violate the class invariant, such that time values are monotonically
         *     increasing, then an exception is due; or
         *
         *  2. if the time preserves the invariant, but is the first non-uniform entry, then we need to create offset
         *     entries (which will have offset of zero, since they followed the uniform pattern by construction), and
         *     emplace our offset on the end.
         *
         *  3. if the time preserves the invariant, and is being emplaced into an already-variable signal, then we
         *     simply note the offset.
         */

        if (samples.size() >= 2)
            if (const auto previous_time = get_time_at_index(samples.size() - 2); given_time <= previous_time + epsilon)
                // Case 1.
                throw std::runtime_error(
                    std::format(
                        "Signal {} rejected out-of-order sample at time {}s (previous sample was accepted at "
                        "time {}s).",
                        get_name(),
                        given_time,
                        previous_time
                    )
                );

        if (time_offsets.has_value())
        // Case 3.
            time_offsets->emplace_back(offset);
        else {
            // Case 2.
            time_offsets.emplace(samples.size(), 0.0f);
            time_offsets->back() = offset;
        }
    }
}

} // namespace echomap
