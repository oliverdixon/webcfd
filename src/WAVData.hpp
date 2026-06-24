/**
 * @file
 * @brief Wave file specification
 * @author Oliver Dixon
 * @date 2026-06-24
 */

#ifndef WEBCFD_WAVDATA_H
#define WEBCFD_WAVDATA_H

// ReSharper disable once CppUnusedIncludeDirective - Emscripten false +ve.
#include <cstdint>
#include <vector>

namespace WebCFD
{

/**
 * A container for wave audio file data composed of one or many channels at a common sample rate.
 */
class WAVData
{
public:
    /**
     * A PCM float-32 sampled audio point at an explicit time offset.
     */
    struct AudioPoint
    {
        float time; /**< Time, in ms. */
        float amplitude; /**< Amplitude at the time, normalised in the range [-1, 1]. */
    };

    /**
     * A single audio channel.
     */
    using AudioChannel = std::vector<AudioPoint>;

    /**
     * Loads a WAV file from the file system.
     *
     * @param file_path The location of the WAV on the local file system.
     * @throws ConfigurationError The WAV file could not be loaded.
     */
    explicit WAVData(const char * file_path);

    /**
     * Downsamples an existing WAVData instance across all channels to the given number of samples.
     *
     * @param other The existing WAVData to downsample.
     * @param downsampled_pcm_frame_count The desired number of samples in the downsampled data.
     */
    explicit WAVData(const WAVData& other, std::uint64_t downsampled_pcm_frame_count);

    /**
     * Downsamples an existing WAVData instance across all channels by the given factor.
     *
     * @param other The existing WAVData to downsample.
     * @param downsampling_factor The factor by which the number of samples should be reduced during downsampling.
     */
    explicit WAVData(const WAVData& other, float downsampling_factor);

    /**
     * Retrieves the common sample rate of the WAV file.
     *
     * @return The sample rate, in Hz, of the wave data.
     */
    [[nodiscard]] std::uint64_t get_sample_rate() const;

    [[nodiscard]] std::vector<AudioChannel>::iterator begin();

    [[nodiscard]] std::vector<AudioChannel>::iterator end();

    [[nodiscard]] std::vector<AudioChannel>::const_iterator begin() const;

    [[nodiscard]] std::vector<AudioChannel>::const_iterator end() const;

    [[nodiscard]] std::vector<AudioChannel>::const_iterator cbegin() const noexcept;

    [[nodiscard]] std::vector<AudioChannel>::const_iterator cend() const noexcept;

private:
    /**
     * Downsample the data points of an AudioChannel to the given threshold.
     *
     * <p>
     *  This helper uses the well-known Largest-Triangle Three-Buckets (LTTB) downsampling algorithm, described in
     *  detail by the Master's Thesis <a href="https://skemman.is/handle/1946/15343"><i>Downsampling Time Series for
     *  Visual Representation</i>, Sveinn Steinarsson (2013)</a>.
     * </p>
     * <p>
     *  In addition to the thesis, reference implementations in all major languages are available online. This function
     *  uses a specialised implementation for the AudioPoint structure:
     *  https://github.com/sveinn-steinarsson/flot-downsample.
     * </p>
     *
     * @param source The original AudioChannel to be downsampled.
     * @param threshold The number of samples in the downsampled data.
     * @return The downsampled AudioChannel.
     * @post The number of samples in the returned AudioChannel matches the threshold parameter.
     */
    static AudioChannel downsample_and_copy(
            const AudioChannel& source,
            size_t threshold
    );

    std::vector<AudioChannel> data;
};

} // namespace WebCFD

#endif // WEBCFD_WAVDATA_H
