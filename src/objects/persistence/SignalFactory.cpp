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

#include <ranges>

#include "../../errors/ConfigurationError.hpp"
#include "../Signal.hpp"

namespace WebCFD
{

std::vector<std::unique_ptr<Signal>> SignalFactory::load_wave_file(
        const char* const file_path
)
{
    drwav drwav_info;
    if (!drwav_init_file(&drwav_info, file_path, nullptr))
        throw ConfigurationError("Cannot open WAV file at " + std::string(file_path));

    const auto typed_path = std::filesystem::path(file_path);
    std::vector<std::unique_ptr<Signal>> signals;
    signals.resize(drwav_info.channels);

    std::size_t channel_num = 1;
    for (auto& channel : signals) {
        const auto formatted_name = std::format("{}#{}", typed_path.stem().c_str(), channel_num);
        channel = std::make_unique<Signal>(formatted_name, Signal::Source(typed_path, channel_num));
        ++channel_num;
    }

    try {
        load_wave_file_into_channels(
                drwav_info,
                file_path,
                signals | std::views::transform([](const std::unique_ptr<Signal>& signal) -> Signal& {
                    return *signal;
                })
        );
    } catch (const std::runtime_error&) {
        drwav_uninit(&drwav_info);
        throw;
    }

    drwav_uninit(&drwav_info);
    return signals;
}

void SignalFactory::load_wave_file(
        const char* const file_path,
        mutable_signal_range auto&& channels
)
{
    drwav drwav_info;
    if (!drwav_init_file(&drwav_info, file_path, nullptr))
        throw ConfigurationError("Cannot open WAV file at " + std::string(file_path));

    try {
        load_wave_file_into_channels(
                drwav_info,
                file_path,
                channels
        );
    } catch (const std::runtime_error&) {
        drwav_uninit(&drwav_info);
        throw;
    }

    drwav_uninit(&drwav_info);
}

void SignalFactory::load_wave_file_into_channels(
        drwav& drwav_info,
        const std::string_view file_path,
        mutable_signal_range auto&& channels
)
{
    const auto channel_count = channels.size();
    assert(drwav_info.channels <= channel_count);

    for (auto& channel : channels)
        channel.reserve_samples(drwav_info.totalPCMFrameCount);

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

        for (drwav_uint64 frame_idx = 0; frame_idx < frame_count; ++frame_idx) {
            std::size_t channel_idx = 0;
            for (auto& channel : channels) {
                /*
                 * The audio data is uniformly spaced, so we can infer the time values by taking the current frame
                 * offset for the chunk (total frames - remaining frames) and adding the current frame index.
                 */
                channel.emplace_sample(
                        Signal::ExternalSampleTag{},
                        drwav_info.totalPCMFrameCount - remaining_frames + frame_idx,
                        interleaved[frame_idx * drwav_info.channels + channel_idx]
                );
                ++channel_idx;
            }
        }

        remaining_frames -= frame_count;
    }

    if (remaining_frames != 0)
        throw ConfigurationError("Cannot read entire WAV file at " + std::string(file_path) + ". Is it corrupted?");
}

} // namespace WebCFD
