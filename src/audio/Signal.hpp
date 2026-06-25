/**
 * @file
 * @brief Audio signal class specification
 * @author Oliver Dixon
 * @date 2026-06-24
 */

#ifndef WEBCFD_SIGNAL_HPP
#define WEBCFD_SIGNAL_HPP

#include <cstdint>
#include <vector>

#include "../Object.hpp"

namespace WebCFD
{

/**
 * A single channel of discretely sampled audio data.
 *
 * @invariant All samples are stored contiguously in memory: iterators model <code>contiguous_iterator</code> and the
 *  internal representation models <code>contiguous_range</code>. This eases integration with C APIs, for example:
 *  <pre>
 *     ImPlot::PlotLine(
 *         "f(x)",
 *         &downsampled_channel.begin()->time,
 *         &downsampled_channel.begin()->amplitude,
 *         static_cast<int>(downsampled_channel.get_sample_count()),
 *         plotting_spec
 *      );
 *  </pre>
 *
 * @invariant The time series is monotonically increasing. This is enforced with exceptions at runtime.
 */
class Signal : public Object<Signal>
{
public:
    /**
     * A PCM float-32 sampled audio point at an explicit time offset.
     */
    struct Sample
    {
        float time;      /**< Time, in ms. */
        float amplitude; /**< Amplitude at the time, normalised in the range [-1, 1]. */
    };

    /**
     * Creates an empty optionally named Signal.
     *
     * @param name Optional display name.
     */
    explicit Signal(std::string_view name = {});

    /**
     * Downsamples an existing Signal instance across all channels to the given number of samples.
     *
     * @param source The existing Signal to downsample.
     * @param sample_count The desired number of samples in the downsampled data.
     * @param name Optional display name.
     */
    Signal(
            const Signal& source,
            std::uint64_t sample_count,
            std::string_view name = {}
    );

    /**
     * Downsamples an existing Signal instance across all channels by the given factor.
     *
     * @param source The existing Signal to downsample.
     * @param downsample_factor The factor by which the number of samples should be reduced during downsampling.
     * @param name Optional display name.
     */
    Signal(
            const Signal& source,
            float downsample_factor,
            std::string_view name = {}
    );

    /**
     * Add a sample to the end of the channel sample data.
     *
     * @param sample Sample to insert
     * @throws std::runtime_error if the sample would violate the monotonically increasing invariant.
     */
    void add_sample(const Sample& sample);

    /**
     * Emplace a sample to the back of the channel sample data.
     *
     * @param time Time of the sample to insert
     * @param amplitude Amplitude of the sample to insert
     * @throws std::runtime_error if the sample would violate the monotonically increasing invariant.
     */
    void emplace_sample(
            uint64_t time,
            float amplitude
    );

    /**
     * Reserves memory to store the given number of total samples in the channel.
     *
     * @param count The number of Sample objects to pre-allocate.
     */
    void reserve_samples(std::size_t count);

    [[nodiscard]] std::uint64_t get_sample_count() const;

    [[nodiscard]] std::vector<Sample>::const_iterator begin() const;
    [[nodiscard]] std::vector<Sample>::const_iterator end() const;
    [[nodiscard]] std::vector<Sample>::const_iterator cbegin() const noexcept;
    [[nodiscard]] std::vector<Sample>::const_iterator cend() const noexcept;

private:
    /**
     * Downsample the data points of a Signal to the given threshold.
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
     * @param source_channel The original Signal to be downsampled.
     * @param threshold The number of samples in the downsampled data.
     * @return The downsampled Signal.
     * @post The number of samples in the returned signal matches the threshold parameter.
     */
    void downsample_and_copy(
            const Signal& source_channel,
            size_t threshold
    );

    std::vector<Sample> samples;
};

} // namespace WebCFD

#endif // WEBCFD_SIGNAL_HPP
