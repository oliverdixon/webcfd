/**
 * @file
 *
 * NativeFileChooser specification
 *
 * @author Oliver Dixon
 * @date 2026-07-22
 */

#ifndef ECHOMAP_NATIVEFILECHOOSER_HPP
#define ECHOMAP_NATIVEFILECHOOSER_HPP

#include <filesystem>

#include "../utility/FilesystemCombo.hpp"
#include "IPanel.hpp"

namespace echomap
{

class EchoMap;

/**
 * @todo Document
 */
class NativeFileChooser : public IPanel
{
public:
    explicit NativeFileChooser(EchoMap* app);

    void draw() noexcept override;

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

    void change_active_project(const Project* new_project) override;

    static const char* get_imgui_stable_name() noexcept;

private:
    constexpr static ImVec2 default_modal_size{500.0f, 0.0f};

    std::filesystem::path expected_extension = ".json";
    std::filesystem::path chosen_path;
    FilesystemCombo file_combo;

    std::string panel_name;
    bool should_open = true;
    EchoMap* app;
};

} // namespace echomap

#endif // ECHOMAP_NATIVEFILECHOOSER_HPP
