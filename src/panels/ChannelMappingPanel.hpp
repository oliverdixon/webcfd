//
// Created by owd on 07/07/2026.
//

#ifndef ECHOMAP_CHANNELMAPPINGPANEL_HPP
#define ECHOMAP_CHANNELMAPPINGPANEL_HPP

#include <sigc++/scoped_connection.h>

#include <string>

#include "IPanel.hpp"

namespace echomap
{

class Signal;
class Sensor;
class EchoMap;
class WorkerResultDespatcher;

/**
 * Provides a panel for defining mappings between Signal and Sensor objects.
 */
class ChannelMappingPanel final : public IPanel
{
public:
    explicit ChannelMappingPanel(
            WorkerResultDespatcher& despatcher,
            EchoMap& app,
            const Project* initial_project = nullptr
    );

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

    void draw() noexcept override;

private:
    void draw_new_channel_mapping() noexcept;
    void draw_existing_channel_mapping() const noexcept;

    const std::string panel_name = "Channel Mapping";

    struct AddChannelMappingRowCache
    {
        const Signal * signal = nullptr;
        const Sensor * sensor = nullptr;
    } new_entry_cache;

    EchoMap& app;
    const Project* active_project = nullptr;
    std::vector<sigc::scoped_connection> connections;
};

} // namespace echomap

#endif // ECHOMAP_CHANNELMAPPINGPANEL_HPP
