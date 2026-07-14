//
// Created by owd on 07/07/2026.
//

#ifndef ECHOMAP_CHANNELMAPPINGPANEL_HPP
#define ECHOMAP_CHANNELMAPPINGPANEL_HPP

#include "IPanel.hpp"

namespace echomap
{

class Signal;
class Sensor;
class EchoMap;

/**
 * Provides a panel for defining mappings between Signal and Sensor objects.
 */
class ChannelMappingPanel final : public IPanel
{
public:
    explicit ChannelMappingPanel(EchoMap& app, const Project* initial_project = nullptr);

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

    void draw() noexcept override;

    void set_active_project(const Project* new_active_project) noexcept override;

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
};

} // namespace echomap

#endif // ECHOMAP_CHANNELMAPPINGPANEL_HPP
