/**
 * @file
 *
 * IndividualUploadModal specification
 *
 * @author Oliver Dixon
 * @date 2026-07-17
 */

#ifndef ECHOMAP_INDIVIDUALUPLOADMODAL_HPP
#define ECHOMAP_INDIVIDUALUPLOADMODAL_HPP

#include <filesystem>
#include <string>

#include "IPanel.hpp"

namespace echomap
{

class EchoMap;
class SignalFactory;

template <class Range>
concept SignalFactoryRange =
        std::ranges::input_range<Range> && std::convertible_to<std::ranges::range_reference_t<Range>, SignalFactory*>;

/**
 * Provides a modal IPanel to map externally sourced Signal objects to separately uploaded files.
 *
 * This is most useful in browser (WebAssembly VFS) contexts where the application cannot interrogate the file system
 * directly.
 */
class IndividualUploadModal : public IPanel
{
public:
    explicit IndividualUploadModal(
            EchoMap* app,
            const Project* project
    );

    void draw() noexcept override;

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

    void reshow() noexcept;

private:
    constexpr static ImVec2 button_size{80.0f, 20.0f};
    constexpr static ImVec2 upload_button_frame_padding{0.0f, 0.0f};
    constexpr static ImVec2 default_modal_size{500.0f, 300.0f};

    void draw_preamble() const noexcept;

    bool draw_table_entry(
            const std::filesystem::path& external_path,
            const std::optional<std::filesystem::path>& vfs_path,
            SignalFactoryRange auto&& factories
    ) const noexcept;

    void draw_buttons(bool are_all_mapped);

    std::string panel_name;
    EchoMap* app;
    const Project* project;
    bool should_open = true;
};

} // namespace echomap

#endif // ECHOMAP_INDIVIDUALUPLOADMODAL_HPP
