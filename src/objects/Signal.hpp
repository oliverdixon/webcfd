/**
 * @file
 * @brief Audio signal class specification
 * @author Oliver Dixon
 * @date 2026-06-24
 */

#ifndef ECHOMAP_SIGNAL_HPP
#define ECHOMAP_SIGNAL_HPP

#include <cstdint>
#include <filesystem>
#include <ranges>
#include <vector>

#include "Object.hpp"

namespace echomap
{

/**
 * A single channel of discretely sampled audio data.
 *
 * <p>
 *  Signals always store their sampled time series in memory, but they optionally express an external source. This is
 *  currently a file specification on the local file system, and an internal channel number, indicating the origin of
 *  the sample data.
 * </p>
 * <p>
 *  Signal objects must be constructed by a SignalFactory. Following release from the factory, all non-metadata state
 *  is immutable, particularly the Sample time series. Trivial metadata (such as the display name from Object) may be
 *  modified at will.
 * </p>
 *
 * @invariant Amplitude samples are stored contiguously in memory. Timing information may either be inferred from the
 *  baseline sampling model or adjusted by an optional per-sample offset array. The timed samples range is a lazy
 *  logical view and is not contiguous.
 */
class Signal : public Object<Signal>
{
public:
    /**
     * A PCM float-32 sampled audio point at an explicit time offset.
     */
    struct Sample
    {
        using TimeT = float;      /**< Type for sample times */
        using AmplitudeT = float; /**< Type for sample amplitudes. */

        TimeT time;               /**< Time, in ms. */
        AmplitudeT amplitude;     /**< Normalised amplitude at the time. */
    };

private:
    std::vector<Sample::AmplitudeT> samples; /**< Amplitude sample stream. */

public:
    /**
     * Indicates an external source of a Signal on the filesystem.
     */
    struct Source
    {
        std::filesystem::path path; /**< The path (absolute or relative to CWD) of the source wave file. */
        std::size_t channel;        /**< The channel number of the Signal within the given file. */
        bool dirty = false;         /**< Does the Signal contain additional samples? */
        bool is_loaded = false;     /**< Has the Signal loaded all samples from the filesystem? */

        [[nodiscard]] bool operator<(const Source& other) const;
    };

    /**
     * The range within which the amplitude values are normalised.
     */
    static constexpr std::pair<Sample::AmplitudeT, Sample::AmplitudeT> normalised_range = {-1.0f, 1.0f};

    /**
     * Retrieves the total number of samples in the Signal stream.
     *
     * @return The number of samples detained by the Signal.
     */
    [[nodiscard]] std::uint64_t get_sample_count() const noexcept;

    /**
     * Retrieves the optional Source of the Signal.
     *
     * <p>
     *  The return value of this operation can be used to determine the origin of the Signal: if the optional is empty,
     *  the signal should be considered "embedded". Otherwise, it has an origin on the filesystem at the given
     *  SignalSource.
     * </p>
     * <p>
     *  If the Signal has an external source, the stored samples in the Signal object do not necessarily match the
     *  external file, as callers may have invoked @ref emplace_sample with arbitrary samples. The Source should only be
     *  considered a hint. This can be checked by inspecting the Source::dirty flag.
     * </p>
     *
     * @return The Source of the Signal, or an empty optional if the Signal is not externally sourced.
     */
    [[nodiscard]] const std::optional<Source>& observe_source() const noexcept;

    [[nodiscard]] Sample::TimeT get_time_offset() const noexcept;
    [[nodiscard]] std::size_t get_sample_rate() const noexcept;

    [[nodiscard]] decltype(samples)::const_iterator begin() const;
    [[nodiscard]] decltype(samples)::const_iterator end() const;
    [[nodiscard]] decltype(samples)::const_iterator cbegin() const noexcept;
    [[nodiscard]] decltype(samples)::const_iterator cend() const noexcept;

    [[nodiscard]] auto timed_samples() const
    {
        return std::views::iota(std::size_t{0}, samples.size()) |
               std::views::transform([this](const std::size_t index) -> Sample {
                   return {.time = get_time_at_index(index), .amplitude = samples[index]};
               });
    }

    [[nodiscard]] Sample::AmplitudeT operator[](std::size_t index) const noexcept;
    [[nodiscard]] std::span<const Sample::AmplitudeT> amplitudes() const noexcept;

    [[nodiscard]] bool is_uniformly_sampled() const noexcept;

    /**
     * Determines the corresponding time of the amplitude appearing at the given index in the sample array.
     *
     * @param index The index of the amplitude in the sample array.
     * @return The timestamp corresponding to the referenced sample.
     *
     * @pre The index is within the bounds of the sample array.
     */
    [[nodiscard]] Sample::TimeT get_time_at_index(std::size_t index) const noexcept;

    Signal(const Signal& old_signal);
    Signal(const Signal& old_signal,
           std::string_view new_name);

private:
    /**
     * Provides basic timing information relating to the Signal samples.
     */
    struct Baseline
    {
        Sample::TimeT time_offset = 0.0f; /**< Timestamp, in seconds, of the first sample. */
        std::size_t sample_rate = 0;      /**< Constant sample rate, in Hz, of the signal. */
        float sample_rate_r = 0.0f;       /**< Reciprocal of the sample rate; zero if sample rate is zero. */
    };

    friend class SignalFactory;

    /**
     * Creates an empty optionally named Signal.
     *
     * @param name Optional display name.
     * @param source Optional file system source path indicating the origin of the file.
     */
    explicit Signal(
            std::string_view name = {},
            const std::optional<Source>& source = {}
    );

    /**
     * Emplace a sample to the back of the channel sample data.
     *
     * The inserted amplitude sample will be associated with the next timestep, computed automatically based on the
     * defined sample rate and time offset.
     *
     * @param amplitude Amplitude of the sample to insert.
     *
     * @pre The given amplitude is within the fixed normalised range.
     */
    void emplace_sample(Sample::AmplitudeT amplitude);

    /**
     * Emplace a sample to the back of the channel sample data.
     *
     * @param time The time to associate with the amplitude, in seconds.
     * @param amplitude The amplitude to associate with the time.
     *
     * @throws std::runtime_error The time of the Sample violated the monotonically increasing invariant.
     * @pre The given amplitude is within the fixed normalised range.
     */
    void emplace_sample(
            Sample::TimeT time,
            Sample::AmplitudeT amplitude
    );

    /**
     * Emplace a sample to the back of the channel sample data.
     *
     * @param sample The Sample to insert at the back of the stream.
     *
     * @throws std::runtime_error The time of the Sample violated the monotonically increasing invariant.
     * @pre The given amplitude is within the fixed normalised range.
     */
    void emplace_sample(const Sample& sample);

    /**
     * Emplace an externally sourced sample to the back of the channel sample data.
     *
     * The inserted amplitude sample will be associated with the next timestep, computed automatically based on the
     * defined sample rate and time offset.
     *
     * @param amplitude Amplitude of the sample to insert.
     *
     * @pre The given amplitude is within the fixed normalised range.
     */
    void emplace_sample_from_source(Sample::AmplitudeT amplitude);

    /**
     * Emplace an externally sourced sample to the back of the channel sample data.
     *
     * @param time The time to associate with the amplitude, in seconds.
     * @param amplitude The amplitude to associate with the time.
     *
     * @throws std::runtime_error The time of the Sample violated the monotonically increasing invariant.
     * @pre The given amplitude is within the fixed normalised range.
     */
    void emplace_sample_from_source(
            Sample::TimeT time,
            Sample::AmplitudeT amplitude
    );

    /**
     * Emplace an externally sourced sample to the back of the channel sample data.
     *
     * @param sample The Sample to insert at the back of the stream.
     *
     * @throws std::runtime_error The time of the Sample violated the monotonically increasing invariant.
     * @pre The given amplitude is within the fixed normalised range.
     */
    void emplace_sample_from_source(const Sample& sample);

    /**
     * Reserves memory to store the given number of total samples in the channel.
     *
     * @param count The number of Sample objects to pre-allocate.
     */
    void reserve_samples(std::size_t count);

    void set_source(
            const std::filesystem::path& path,
            std::size_t channel
    );

    void set_time_offset(Sample::TimeT new_time_offset) noexcept;
    void set_sample_rate(std::size_t new_sample_rate) noexcept;

    /**
     * Implementation helper to associate the latest amplitude sample with the given time.
     *
     * @param given_time The time to associate with the latest sample.
     *
     * @throws std::runtime_error The time of the Sample violated the monotonically increasing invariant.
     */
    void emplace_time(Sample::TimeT given_time);

    Baseline timing_baseline;        /**< A baseline of timing parameters. */
    std::optional<Source> fs_source; /**< External source, if any, of the Signal Sample stream. */

    /**
     * An optional vector of explicit timestamps for each sample.
     *
     * <p>
     *  If provided, the timestamps provide a sample-by-sample indication of the time corresponding with amplitudes in
     *  the sample time series data. The scalars are specified as offsets relative to the baseline time, which is
     *  computed as @f$ b_i = t_0 + i \delta t @f$, where @f$ t_0 @f$ is the global time offset, @f$ \delta t @f$ is the
     *  reciprocal of the sample rate, and @f$ i @f$ is the integer zero-based index such that
     *  @f$ 0 < i \leq \vert S \vert @f$, where @f$ \vert S \vert @f$ denotes the size of the sample set @f$ S @f$.
     * </p>
     * <p>
     *  This is useful to support variably sampled time series, such as in the result of LTTB downsampling. Whether
     *  explicit timestamps are provided is rather opaque to the user: they can request timing information, which may be
     *  inferred from the baseline or indexed from this array as appropriate.
     * </p>
     * <p>
     *  When a new sample is emplaced, a time can be provided. If the given time is different than what would be
     *  expected from the baseline, this vector is updated with the suitable offset. If it doesn't exist, it is created.
     * </p>
     *
     * @invariant Timestamp elements are in bijective correspondence with the amplitude samples.
     */
    std::optional<std::vector<Sample::TimeT>> time_offsets;
};

template <> constexpr std::string_view Object<Signal>::class_name = "Signal";

} // namespace echomap

#endif // ECHOMAP_SIGNAL_HPP
