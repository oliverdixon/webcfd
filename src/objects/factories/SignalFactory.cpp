/**
 * @file
 * @brief Wave file implementation
 * @author Oliver Dixon
 * @date 2026-06-24
 */

#include "SignalFactory.hpp"

#define DR_WAV_IMPLEMENTATION
// ReSharper disable once CppUnusedIncludeDirective
#include <dr_wav.h>

#include <algorithm>
#include <cmath>
#include <ranges>

#include "../../Logger.hpp"
#include "../../errors/ConfigurationError.hpp"
#include "../Signal.hpp"

namespace echomap
{

SignalFactory::SignalFactory() :
    // Cannot use make_unique here since Signal c'tor is private, and SignalFactory is "just a friend".
    target(std::unique_ptr<Signal>(new Signal()))
{
}

bool SignalFactory::operator==(
        const SignalFactory& other
) const
{
    if (target.get() == other.target.get())
        return true;

    if (target == nullptr)
        return false; // We're null; the other isn't.

    if (other.target == nullptr)
        return false; // The other is null; we're not.

    // Safe to dereference; compare by IDs.
    return *target == *other.target;
}

bool SignalFactory::operator<(
        const SignalFactory& other
) const
{
    if (target == nullptr || other.target == nullptr || !target->observe_source().has_value() ||
        !other.target->observe_source().has_value())
        return false;

    // Safe to deference; attempt to compare by channel and then path.
    const auto& us = *target->observe_source(); // NOLINT(*-identifier-length)
    const auto& them = *target->observe_source();

    return us < them;
}

std::vector<std::unique_ptr<Signal>> SignalFactory::load_wave_file(
        const char* const file_path
)
{
    drwav drwav_info;
    if (drwav_init_file(&drwav_info, file_path, nullptr) == 0u)
        throw ConfigurationError("Cannot open WAV file at " + std::string(file_path));

    const auto typed_path = std::filesystem::path(file_path);
    std::vector<std::unique_ptr<Signal>> signals;
    signals.resize(drwav_info.channels);

    std::size_t channel_num = 1;
    for (auto& channel : signals) {
        const auto formatted_name = std::format("{}#{}", typed_path.stem().c_str(), channel_num);
        channel = std::unique_ptr<Signal>(new Signal(formatted_name, Signal::Source(typed_path, channel_num)));
        ++channel_num;
    }

    try {
        std::vector<Signal*> signal_ptrs;
        signal_ptrs.reserve(signals.size());
        std::ranges::transform(
                signals,
                std::back_inserter(signal_ptrs),
                [](const std::unique_ptr<Signal>& signal) -> Signal* {
                    return signal.get();
                }
        );

        load_wave_file_into_channels(drwav_info, file_path, signal_ptrs);
    } catch (const std::runtime_error&) {
        drwav_uninit(&drwav_info);
        throw;
    }

    drwav_uninit(&drwav_info);
    return signals;
}

void SignalFactory::load_wave_file(
        const char* const file_path,
        const std::span<SignalFactory* const> channel_factories
)
{
    drwav drwav_info;
    if (drwav_init_file(&drwav_info, file_path, nullptr) == 0u)
        throw ConfigurationError("Cannot open WAV file at " + std::string(file_path));

    assert(channel_factories.size() >= drwav_info.channels);

    try {
        /*
         * For convenience of callers, this function takes a span of factories responsible for constructing the signals
         * for each of the channel slots, rather than the signals themselves. But for portability and simplicity, our
         * internal functions need the signals directly. Hence, we cheaply construct a span-compliant collection of the
         * mutating signal pointers, accessible to us as private member variables.
         */
        std::vector<Signal*> channels;
        channels.reserve(channel_factories.size());
        for (auto* const factory : channel_factories)
            channels.push_back(factory->target == nullptr ? nullptr : factory->target.get());
        load_wave_file_into_channels(drwav_info, file_path, channels);
    } catch (const std::runtime_error&) {
        drwav_uninit(&drwav_info);
        throw;
    }

    drwav_uninit(&drwav_info);
}

std::unique_ptr<Signal> SignalFactory::downsample(
        const Signal& source,
        const float downsample_factor,
        const std::string_view name
)
{
    auto sample_count = static_cast<std::uint64_t>(static_cast<float>(source.get_sample_count()) / downsample_factor);
    if (static_cast<float>(sample_count) < downsample_factor)
        sample_count = static_cast<std::uint64_t>(downsample_factor);

    auto downsampled = name.empty() ? lttb_downsample(
                                              source,
                                              sample_count,
                                              std::format("{} ({}x downsampled)", source.get_name(), downsample_factor)
                                      )
                                    : lttb_downsample(source, sample_count, name);

    LOG_F_DEBUG(
            "Created {} as {}x-LTTB variant of {} with {} samples.",
            downsampled->get_name(),
            downsample_factor,
            source.get_name(),
            downsampled->get_sample_count()
    );

    return downsampled;
}

std::unique_ptr<Signal> SignalFactory::take_signal() noexcept
{
    auto signal = std::move(target);
    target = std::unique_ptr<Signal>(new Signal()); // NOLINT(*-unhandled-exception-at-new)
    return signal;
}

const Signal& SignalFactory::observe_signal() const noexcept
{
    return *target;
}

void SignalFactory::emplace_sample(
        const Signal::Sample::AmplitudeT amplitude
) const
{
    target->emplace_sample(amplitude);
}

void SignalFactory::emplace_sample(
        const Signal::Sample& sample
) const
{
    target->emplace_sample(sample);
}

void SignalFactory::emplace_sample(
        const Signal::Sample::TimeT time,
        const Signal::Sample::AmplitudeT amplitude
) const
{
    target->emplace_sample(time, amplitude);
}

void SignalFactory::emplace_sample_from_source(
        const Signal::Sample::AmplitudeT amplitude
) const
{
    target->emplace_sample_from_source(amplitude);
}

void SignalFactory::emplace_sample_from_source(
        const Signal::Sample& sample
) const
{
    target->emplace_sample_from_source(sample);
}

void SignalFactory::emplace_sample_from_source(
        const Signal::Sample::TimeT time,
        const Signal::Sample::AmplitudeT amplitude
) const
{
    target->emplace_sample_from_source(time, amplitude);
}

void SignalFactory::set_signal_name(
        const std::string_view name
) const
{
    target->set_name(name);
}

void SignalFactory::set_time_offset(
        const Signal::Sample::TimeT time_offset
) const noexcept
{
    target->set_time_offset(time_offset);
}

void SignalFactory::set_sample_rate(
        const std::size_t sample_rate
) const noexcept
{
    target->set_sample_rate(sample_rate);
}

void SignalFactory::set_source(
        const std::filesystem::path& path,
        const std::size_t channel
) const
{
    target->set_source(path, channel);
}

void SignalFactory::load_wave_file_into_channels(
        drwav& drwav_info,
        const std::string_view file_path,
        std::span<Signal* const> signal_ptrs
)
{
    assert(drwav_info.channels <= std::ranges::size(signal_ptrs));

    for (auto* const channel : signal_ptrs)
        if (channel != nullptr) {
            channel->reserve_samples(drwav_info.totalPCMFrameCount);
            channel->set_sample_rate(drwav_info.sampleRate);
        }

    /*
     * Dr_WAV provides audio data as amplitudes uniformly interleaved across the channels. That is, for a stereo signal,
     * data is provided in the pattern L0, R0, L1, R1, ..., L(N-1), R(N-1). We receive the interleaved data in chunks of
     * a fixed size and iteratively de-interleave it into our AudioPoint channels until all frames from the source file
     * have been consumed.
     */
    constexpr drwav_uint64 chunk_frame_count = 8192;
    std::vector<float> interleaved(chunk_frame_count * drwav_info.channels);
    drwav_uint64 remaining_frames = drwav_info.totalPCMFrameCount;

    while (remaining_frames > 0) {
        const auto frame_count = std::min(remaining_frames, chunk_frame_count);
        if (drwav_read_pcm_frames_f32(&drwav_info, frame_count, interleaved.data()) != frame_count)
            // We couldn't read the expected number of frames. drwav_init_file must've provided the wrong count.
            throw ConfigurationError("Cannot read WAV file at " + std::string(file_path) + ". Is it corrupted?");

        for (drwav_uint64 frame_idx = 0; frame_idx < frame_count; ++frame_idx)
            for (drwav_uint16 channel_idx = 0; channel_idx < drwav_info.channels; ++channel_idx) {
                if (auto* const destination = std::ranges::begin(signal_ptrs)[channel_idx]; destination != nullptr)
                    /*
                     * The audio data is uniformly spaced, so we can infer the time values by taking the current frame
                     * offset for the chunk (total frames - remaining frames) and adding the current frame index.
                     */
                    destination->emplace_sample_from_source(interleaved[frame_idx * drwav_info.channels + channel_idx]);
            }

        remaining_frames -= frame_count;
    }

    if (remaining_frames != 0)
        throw ConfigurationError("Cannot read entire WAV file at " + std::string(file_path) + ". Is it corrupted?");

    for (auto* const channel : signal_ptrs) {
        // Assert that any signal being constructed by these means should have an extant FS source.
        assert(channel->fs_source.has_value());
        channel->fs_source->is_loaded = true;
        LOG_F_DEBUG(
                "Loaded signal \"{}\" with {} samples at {} Hz, starting at {} s.",
                channel->get_name(),
                channel->get_sample_count(),
                channel->get_sample_rate(),
                channel->get_time_offset()
        );
    }
}

std::unique_ptr<Signal> SignalFactory::lttb_downsample(
        const Signal& source,
        const size_t threshold,
        const std::string_view name
)
{
    const auto source_size = source.get_sample_count();

    // We don't need to assert for the post-condition on these trivial cases.

    if (threshold == 0 || source_size == 0)
        // Base case: the user requested zero samples (empty signal), or there were no samples available in the source.
        return std::unique_ptr<Signal>(new Signal(name));

    if (threshold >= source_size)
        // Base case: the destination wants more samples than are available. Just copy the source signal.
        return std::make_unique<Signal>(source, name);

    auto downsampled = std::unique_ptr<Signal>(new Signal(name));
    downsampled->reserve_samples(threshold);

    if (threshold == 1) {
        // Base case: the user only wants one sample. We choose the first by convention.
        downsampled->emplace_sample(source.get_time_at_index(0), source[0]);
        return downsampled;
    }

    if (threshold == 2) {
        // Base case: the user only wants two samples. We choose the first and last by necessity.
        downsampled->emplace_sample(source.get_time_at_index(0), source[0]);
        downsampled->emplace_sample(source.get_time_at_index(source_size - 1), source[source_size - 1]);
        return downsampled;
    }

    const auto bucket_size =
            static_cast<unsigned int>(static_cast<double>(source_size - 2) / static_cast<double>(threshold - 2));
    std::size_t fixed_point_idx = 0;

    // Always add the first point.
    downsampled->emplace_sample(source.get_time_at_index(0), source[0]);

    for (std::size_t dst_point_idx = 0; dst_point_idx < threshold - 2; ++dst_point_idx) {
        Signal::Sample::TimeT average_time = 0.0f;
        Signal::Sample::AmplitudeT average_amplitude = 0.0f;

        // Calculate the point-average for the next bucket, containing our fixed point.

        const auto average_range_start = static_cast<std::size_t>(std::floor((dst_point_idx + 1) * bucket_size)) + 1;
        const auto average_range_end =
                std::min(static_cast<std::uint64_t>(std::floor((dst_point_idx + 2) * bucket_size)) + 1, source_size);
        const auto average_range_length = average_range_end - average_range_start;

        for (auto range_idx = average_range_start; range_idx < average_range_end; ++range_idx) {
            average_time += source.get_time_at_index(range_idx);
            average_amplitude += source[range_idx];
        }

        average_time /= static_cast<Signal::Sample::TimeT>(average_range_length);
        average_amplitude /= static_cast<Signal::Sample::AmplitudeT>(average_range_length);

        // Store the sample data at the fixed point.
        const auto fp_time = source.get_time_at_index(fixed_point_idx);
        const auto fp_amplitude = source[fixed_point_idx];

        // Get the range for the current bucket and compute triangle areas over the three buckets.
        const auto range_lower = static_cast<std::size_t>(std::floor(dst_point_idx * bucket_size)) + 1;
        const auto range_upper = std::min(
                static_cast<std::uint64_t>(std::floor((dst_point_idx + 1) * bucket_size)) + 1,
                source_size - 1
        );

        // (C++ note: we need to combine Sample::TimeT and Sample::AmplitudeT here, so float seems like a safe choice.)
        auto max_area = std::numeric_limits<float>::lowest();
        auto next_fixed_point_idx = range_lower;

        // Calculate triangle area formed by the vertices in the adjacent buckets, tracking the maximum.
        for (auto range_idx = range_lower; range_idx < range_upper; ++range_idx) {
            const float area = std::abs(
                    (fp_time - average_time) * (source[range_idx] - fp_amplitude) -
                    (fp_time - source.get_time_at_index(range_idx)) * (average_amplitude - fp_amplitude)
            );

            if (area > max_area) {
                max_area = area;
                next_fixed_point_idx = range_idx;
            }
        }

        /*
         * Pick the point from the bucket to include in the downsampled data, and set the index as our next starting
         * point.
         */
        downsampled->emplace_sample(source.get_time_at_index(next_fixed_point_idx), source[next_fixed_point_idx]);

        fixed_point_idx = next_fixed_point_idx;
    }

    // Always add the last point.
    downsampled->emplace_sample(source.get_time_at_index(source_size - 1), source[source_size - 1]);

    assert(downsampled->get_sample_count() == threshold);
    return downsampled;
}

} // namespace echomap
