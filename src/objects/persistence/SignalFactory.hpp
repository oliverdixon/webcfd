/**
 * @file
 * @brief Wave file specification
 * @author Oliver Dixon
 * @date 2026-06-24
 */

#ifndef WEBCFD_SIGNALFACTORY_H
#define WEBCFD_SIGNALFACTORY_H

#include <dr_wav.h>

#include <memory>
#include <vector>

namespace WebCFD
{

class Signal;


/**
 * Models a sized range of mutable Signal references.
 *
 * @tparam R The candidate type.
 */
template <typename R>
concept mutable_signal_range = std::ranges::forward_range<R> && std::ranges::sized_range<R> &&
                               std::is_lvalue_reference_v<std::ranges::range_reference_t<R>> &&
                               std::same_as<std::remove_cvref_t<std::ranges::range_reference_t<R>>, Signal>;

class SignalFactory
{
public:
    /**
     * Loads a WAV file from the file system.
     *
     * The Signal objects are constructed by this function, with default names, in the order of the channels in the wave
     * file. That is, the first Signal will represent the first channel in the file, etc.
     *
     * @param file_path The location of the WAV on the local file system.
     * @throws ConfigurationError The WAV file could not be loaded.
     * @returns Series of owned Signal objects, one for each channel.
     */
    [[nodiscard]] static std::vector<std::unique_ptr<Signal>> load_wave_file(const char* file_path);

    /**
     * Loads a WAV file from the file system.
     *
     * The Signal objects are not constructed by this function. As many constructed Signal objects as there are channels
     * in the wave file must be provided through the mutable Signal range.
     *
     * @param file_path The location of the WAV on the local file system.
     * @param channels Destination of the Signal channels.
     *
     * @throws ConfigurationError The WAV file could not be loaded.
     * @pre There are sufficient Signal objects in the destination range to store channels in the wave file.
     */
    static void load_wave_file(
            const char* file_path,
            mutable_signal_range auto&& channels
    );

private:
    /**
     * Loads the time-series sampled data into the given Signal objects.
     *
     * @param drwav_info The initialised and loaded dr_wav control structure.
     * @param file_path The location of the WAV on the local file system.
     * @param channels Destination of the Signal channels.
     *
     * @pre There are sufficient Signal objects in the destination range to store channels in the wave file.
     */
    static void load_wave_file_into_channels(
            drwav& drwav_info,
            std::string_view file_path,
            mutable_signal_range auto&& channels
    );
};

} // namespace WebCFD

#endif // WEBCFD_SIGNALFACTORY_H
