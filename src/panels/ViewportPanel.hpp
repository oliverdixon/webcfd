/**
 * @file
 * @brief WebCFD main viewport panel specification
 * @author Oliver Dixon
 * @date 2026-06-20
 */

#ifndef WEBCFD_VIEWPORTPANEL_HPP
#define WEBCFD_VIEWPORTPANEL_HPP

#include <implot.h>

#include <string>

#include "../Project.hpp"
#include "IPanel.hpp"

namespace WebCFD
{

class ViewportPanel final : public IPanel
{
public:
    ViewportPanel();

    explicit ViewportPanel(Project * project);

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

    void draw() noexcept override;

private:
    void draw_signal_waveforms() const noexcept;
    void draw_channel_mappings() const noexcept;

    const Signal * get_downsampled_signal(const Signal& signal) const;

    mutable std::unordered_map<Signal::id_type, Signal> downsampled_waveforms;
    mutable ImPlotRect max_bounding_box{
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::lowest(),
        -1.0,
        1.0
    };

    const std::string panel_name = "Manager";
    static constexpr float default_downsample_factor = 50.0f;

    ImPlotSpec plotting_spec;
    Project * active_project = nullptr;
};

} // namespace WebCFD

#endif // WEBCFD_VIEWPORTPANEL_HPP
