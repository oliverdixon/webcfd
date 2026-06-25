/**
 * @file
 * @brief Audio signal class implementation
 * @author Oliver Dixon
 * @date 2026-06-24
 */

#include "Signal.hpp"

#include <cassert>
#include <cmath>
#include <stdexcept>

namespace WebCFD
{

template <> constexpr std::string Object<Signal>::class_name = "Signal";

Signal::Signal(
        const std::string_view name
) :
    Object(name)
{
}

Signal::Signal(
        const Signal& source,
        const std::uint64_t sample_count,
        const std::string_view name
) :
    Object(name)
{
    downsample_and_copy(source, sample_count);
}

Signal::Signal(
        const Signal& source,
        const float downsample_factor,
        const std::string_view name
) :
    Signal(source,
           static_cast<std::uint64_t>(static_cast<float>(source.get_sample_count()) / downsample_factor),
           name)
{
}

void Signal::add_sample(
        const Sample& sample
)
{
    if (!samples.empty() && sample.time <= samples.back().time)
        throw std::runtime_error("Inserted sample violates monotonically increasing invariant of channel.");

    samples.push_back(sample);
}

void Signal::emplace_sample(
        const uint64_t time,
        const float amplitude
)
{
    if (!samples.empty() && time <= samples.back().time)
        throw std::runtime_error("Inserted sample violates monotonically increasing invariant of channel.");

    samples.emplace_back(time, amplitude);
}

void Signal::reserve_samples(
        const std::size_t count
)
{
    samples.reserve(count);
}

std::uint64_t Signal::get_sample_count() const
{
    return samples.size();
}

std::vector<Signal::Sample>::const_iterator Signal::begin() const
{
    return samples.begin();
}

std::vector<Signal::Sample>::const_iterator Signal::end() const
{
    return samples.end();
}

std::vector<Signal::Sample>::const_iterator Signal::cbegin() const noexcept
{
    return samples.cbegin();
}

std::vector<Signal::Sample>::const_iterator Signal::cend() const noexcept
{
    return samples.cend();
}

void Signal::downsample_and_copy(
        const Signal& source_channel,
        const size_t threshold
)
{
    auto& dest = samples;
    const auto& source = source_channel.samples;

    // We don't need to assert for the post-condition on these first two trivial cases.

    if (threshold == 0 || source.empty())
        return;

    if (threshold >= source.size()) {
        dest.reserve(source.size());
        std::ranges::copy_n(source.begin(), source.end() - source.begin(), std::back_inserter(dest));
        return;
    }

    dest.reserve(threshold);

    if (threshold == 1 || threshold == 2) {
        assert(dest.size() == threshold);
        dest = source;
        return;
    }

    const auto bucket_size =
            static_cast<std::size_t>(static_cast<double>(source.size() - 2) / static_cast<double>(threshold - 2));
    std::size_t fixed_point_idx = 0;

    dest.push_back(source.front()); // // Always add the first point.

    for (std::size_t i = 0; i < threshold - 2; ++i) {
        float average_time = 0.0f;
        float average_amplitude = 0.0f;

        // Calculate the point-average for the next bucket, containing our fixed point.

        const std::size_t average_range_start = (i + 1) * bucket_size + 1;
        const auto average_range_end = std::min((i + 2) * bucket_size + 1, source.size());
        const auto average_range_length = average_range_end - average_range_start;

        for (std::size_t range_idx = average_range_start; range_idx < average_range_end; ++range_idx) {
            average_time += source[average_range_start].time;
            average_amplitude += source[average_range_start].amplitude;
        }

        average_time /= static_cast<float>(average_range_length);
        average_amplitude /= static_cast<float>(average_range_length);

        // Get the range for the current bucket and compute triangle areas over the three buckets.

        const std::size_t range_lower = i * bucket_size + 1;
        const std::size_t range_upper = (i + 1) * bucket_size + 1;

        const auto fixed_point_time = source[fixed_point_idx].time;
        const auto fixed_point_amplitude = source[fixed_point_idx].amplitude;
        auto max_area = std::numeric_limits<float>::lowest();
        std::size_t next_fixed_point_idx = 0;

        for (std::size_t range_idx = range_lower; range_idx < range_upper; ++range_idx) {
            // Calculate triangle area formed by the vertices in the adjacent buckets, tracking the maximum.
            const float area = std::abs(
                    (fixed_point_time - average_time) * (source[range_idx].amplitude - fixed_point_amplitude) -
                    (fixed_point_time - source[range_idx].time) * (average_amplitude - fixed_point_amplitude)
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
        dest.push_back(source[next_fixed_point_idx]);
        fixed_point_idx = next_fixed_point_idx;
    }

    dest.push_back(source.back()); // Always add the last point.
    assert(dest.size() == threshold);
}

} // namespace WebCFD
