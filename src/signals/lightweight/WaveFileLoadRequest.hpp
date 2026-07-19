/**
 * @file
 *
 * WaveFileLoadRequest specification
 *
 * @author Oliver Dixon
 * @date 2026-07-19
 */

#ifndef ECHOMAP_WAVEFILELOADREQUEST_HPP
#define ECHOMAP_WAVEFILELOADREQUEST_HPP

#include <filesystem>

#include "../../objects/Project.hpp"

namespace echomap
{

/**
 * A lightweight task indicating that the wave file at the specified location should be loaded.
 */
struct WaveFileLoadRequest
{
    /**
     * Create a request to load a wave file and associate its sample data with an existing Signal and Project.
     *
     * @param project_id The ID of the Project within which the destination Signal is located.
     * @param channel_map The mappings between the 1-based channel number in the wave file, and the ID of the
     *  corresponding Signal into which the channel data should be emplaced.
     * @param path The path of the wave file on the filesystem.
     */
    explicit WaveFileLoadRequest(
            const Project::id_type project_id,
            std::vector<std::pair<
                    std::uint16_t,
                    Signal::id_type>> channel_map,
            std::filesystem::path path
    ) :
        project_id(project_id),
        channel_map(std::move(channel_map)),
        path(std::move(path))
    {
    }

    Project::id_type project_id;
    std::vector<std::pair<std::uint16_t, Signal::id_type>> channel_map;
    std::filesystem::path path;
};

} // namespace echomap

#endif // ECHOMAP_WAVEFILELOADREQUEST_HPP
