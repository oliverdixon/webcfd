/**
 * @file
 * @brief Wave file implementation
 * @author Oliver Dixon
 * @date 2026-06-24
 */

#include "WAVData.hpp"

#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

#include <limits>
#include <ranges>

#include "ConfigurationError.hpp"

namespace WebCFD {

WAVData::WAVData(
        const char* const file_path
)
{
    drwav drwav_info;
    if (!drwav_init_file(&drwav_info, file_path, nullptr))
        throw ConfigurationError("Cannot open WAV file at " + std::string(file_path));

    data.resize(drwav_info.channels);
    for (auto& channel : data)
        channel.reserve(drwav_info.totalPCMFrameCount);

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

        if (drwav_read_pcm_frames_f32(&drwav_info, frame_count, interleaved.data()) != frame_count) {
            // We couldn't read the expected number of frames. drwav_init_file must've provided the wrong count.
            drwav_uninit(&drwav_info);
            throw ConfigurationError("Cannot read WAV file at " + std::string(file_path) + ". Is it corrupted?");
        }

        for (drwav_uint64 frame_idx = 0; frame_idx < frame_count; ++frame_idx) {
            std::size_t channel_idx = 0;
            for (auto& channel : data) {
                /*
                 * The audio data is uniformly spaced, so we can infer the time values by taking the current frame
                 * offset for the chunk (total frames - remaining frames) and adding the current frame index.
                 */
                channel.emplace_back(drwav_info.totalPCMFrameCount - remaining_frames + frame_idx,
                    interleaved[frame_idx * drwav_info.channels + channel_idx]);
                ++channel_idx;
            }
        }

        remaining_frames -= frame_count;
    }

    drwav_uninit(&drwav_info);
    if (remaining_frames != 0)
        throw ConfigurationError("Cannot read entire WAV file at " + std::string(file_path) + ". Is it corrupted?");
}

WAVData::WAVData(
        const WAVData& other,
        const std::uint64_t downsampled_pcm_frame_count
)
{
    data.resize(other.data.size());
    for (auto [our_channel, their_channel] : std::ranges::views::zip(data, other.data))
        our_channel = downsample_and_copy(their_channel, downsampled_pcm_frame_count);
}

WAVData::WAVData(
        const WAVData& other,
        const float downsampling_factor
) :
    WAVData(other, static_cast<std::uint64_t>(static_cast<float>(other.get_sample_rate()) / downsampling_factor))
{
}

std::uint64_t WAVData::get_sample_rate() const
{
    if (data.empty())
        throw std::runtime_error("No channels exist in the data. The effective sample rate is undefined.");

    // The WAV standard requires that all channels share a sample rate, so we can just look at any channel.
    const auto& first_channel = data.front();

    if (first_channel.empty())
        throw std::runtime_error("No data exists in the channels. The effective sample rate is undefined.");

    return first_channel.size();
}

std::vector<WAVData::AudioChannel>::iterator WAVData::begin()
{
    return data.begin();
}

std::vector<WAVData::AudioChannel>::iterator WAVData::end()
{
    return data.end();
}

std::vector<WAVData::AudioChannel>::const_iterator WAVData::begin() const
{
    return data.begin();
}

std::vector<WAVData::AudioChannel>::const_iterator WAVData::end() const
{
    return data.end();
}

std::vector<WAVData::AudioChannel>::const_iterator WAVData::cbegin() const noexcept
{
    return data.cbegin();
}

std::vector<WAVData::AudioChannel>::const_iterator WAVData::cend() const noexcept
{
    return data.cend();
}

WAVData::AudioChannel WAVData::downsample_and_copy(
        const AudioChannel& source,
        const size_t threshold
)
{
    AudioChannel dest;

    // We don't need to assert for the post-condition on these first two trivial cases.

    if (threshold == 0 || source.size() == 0)
        return dest;

    if (threshold >= source.size()) {
        dest.reserve(source.size());
        std::ranges::copy_n(source.begin(), source.size(), std::back_inserter(dest));
        return dest;
    }

    dest.reserve(threshold);

    if (threshold == 1 || threshold == 2) {
        assert(dest.size() == threshold);
        dest = source;
        return dest;
    }

    const auto bucket_size = static_cast<double>(source.size() - 2) / static_cast<double>(threshold - 2);
    std::size_t fixed_point_idx = 0;

    dest.push_back(source.front()); // // Always add the first point.

    for (std::size_t i = 0; i < threshold - 2; ++i) {
        float average_time = 0.0f;
        float average_amplitude = 0.0f;

        // Calculate the point-average for the next bucket, containing our fixed point.

        const std::size_t average_range_start = (i + 1) * bucket_size + 1;
        const auto average_range_end = std::min(static_cast<std::size_t>((i + 2) * bucket_size + 1), source.size());
        const auto average_range_length = average_range_end - average_range_start;

        for (std::size_t range_idx = average_range_start; range_idx < average_range_end; ++range_idx) {
            average_time += source[average_range_start].time;
            average_amplitude += source[average_range_start].amplitude;
        }

        average_time /= average_range_length;
        average_amplitude /= average_range_length;

        // Get the range for the current bucket and compute triangle areas over the three buckets.

        const std::size_t range_lower = i * bucket_size + 1;
        const std::size_t range_upper = (i + 1) * bucket_size + 1;

        const float fixed_point_time = source[fixed_point_idx].time;
        const float fixed_point_amplitude = source[fixed_point_idx].time;
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
    return dest;
}

} // WebCFD
