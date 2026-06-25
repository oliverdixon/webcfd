/**
 * @file
 * @brief Wave file specification
 * @author Oliver Dixon
 * @date 2026-06-24
 */

#ifndef WEBCFD_WAVDATA_H
#define WEBCFD_WAVDATA_H

#include <memory>
#include <vector>

#include "Signal.hpp"

namespace WebCFD
{

class WAVDataLoader
{
public:
    /**
     * Loads a WAV file from the file system.
     *
     * @param file_path The location of the WAV on the local file system.
     * @throws ConfigurationError The WAV file could not be loaded.
     * @returns Series of owned Signal objects, one for each channel.
     */
     static std::vector<std::unique_ptr<Signal>> load_wave_file(const char * file_path);
};

} // namespace WebCFD

#endif // WEBCFD_WAVDATA_H
