/**
 * @file
 *
 * FilesystemCombo implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-23
 */

#include "FilesystemCombo.hpp"

#include <imgui.h>

#include <algorithm>
#include <cstring>

#include "../EchoMap.hpp"

namespace echomap
{

FilesystemCombo::FilesystemCombo(
        EchoMap* const app
) :
    app(app)
{
    std::error_code error_code;
    const auto cwd = std::filesystem::current_path(error_code);

    if (error_code || cwd.empty() || !std::filesystem::is_directory(cwd))
        throw std::runtime_error("Could not get the current working directory. Is the filesystem readable?");

    update_root(cwd);
}

bool FilesystemCombo::operator()(
        std::filesystem::path& selected_path
)
{
    bool changed = false;

    ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());

    const std::string preview = selected_path.empty() ? std::string{current_root.data()} : selected_path.string();

    if (ImGui::BeginCombo("##FilesystemCombo", preview.c_str(), ImGuiComboFlags_HeightLarge)) {
        if (ImGui::IsWindowAppearing()) {
            ImGui::SetKeyboardFocusHere();
            invalidate_cache();
        }

        changed = draw_combo_body(selected_path);

        ImGui::EndCombo();
        app->increment_forced_frames();
    }

    return changed;
}

FilesystemCombo::Entry::Entry(
        const std::filesystem::path& path,
        const bool is_directory_hint
) :
    path(path),
    name(path.filename().string()),
    is_directory(is_directory_hint),
    imgui_id((is_directory ? name + '/' : name) + "##" + path.string()),
    flags(is_directory ? ImGuiSelectableFlags_NoAutoClosePopups : ImGuiSelectableFlags_None)
{
    assert(is_directory == std::filesystem::is_directory(path));
}

bool FilesystemCombo::Entry::draw() const noexcept
{
    return ImGui::Selectable(imgui_id.c_str(), false, flags);
}

void FilesystemCombo::invalidate_cache()
{
    cache.reset();
}

FilesystemCombo::EntryCache::EntryCache(
        const std::filesystem::path& directory
) :
    directory(directory)
{
    std::error_code error_code;

    std::filesystem::directory_iterator iterator{
            directory,
            std::filesystem::directory_options::skip_permission_denied |
                    std::filesystem::directory_options::follow_directory_symlink,
            error_code
    };

    const std::filesystem::directory_iterator end{};

    while (!error_code && iterator != end) {
        const std::filesystem::directory_entry& directory_entry = *iterator;
        std::error_code entry_error_code;
        const std::filesystem::file_status status = directory_entry.status(entry_error_code);

        if (!entry_error_code) {
            // ReSharper disable CppTooWideScopeInitStatement
            const bool is_directory = std::filesystem::is_directory(status);
            const bool is_regular_file = std::filesystem::is_regular_file(status);
            // ReSharper restore CppTooWideScopeInitStatement

            if (is_directory || is_regular_file)
                entries.emplace_back(directory_entry.path(), is_directory);
        }

        iterator.increment(error_code);
    }

    std::ranges::sort(entries);
}

void FilesystemCombo::update_root(
        const std::filesystem::path& path
)
{
    const std::string path_string = path.string();
    const std::size_t length = std::min(current_root.size() - 1, path_string.size());

    std::memcpy(current_root.data(), path_string.data(), length);
    current_root[length] = '\0';
}

bool FilesystemCombo::draw_combo_body(
        std::filesystem::path& selected_path
)
{
    ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
    const bool enter_pressed =
            ImGui::InputText("##path", current_root.data(), current_root.size(), ImGuiInputTextFlags_EnterReturnsTrue);
    const BrowseTarget target = get_browse_target();

    bool changed = false;

    if (enter_pressed) {
        accept_path(selected_path, target.typed_path.empty() ? target.directory : target.typed_path);
        changed = true;
        ImGui::CloseCurrentPopup();
    }

    ImGui::Separator();

    if (!std::filesystem::is_directory(target.directory)) {
        ImGui::TextDisabled("Not a readable directory.");
        return changed;
    }

    draw_parent_entry(target.directory);

    if (!cache || cache->directory != target.directory)
        // If we haven't already cached the children of this directory, do it now.
        cache.emplace(target.directory);

    draw_cached_entries(target.filter, selected_path);
    return changed;
}

FilesystemCombo::BrowseTarget FilesystemCombo::get_browse_target() const
{
    // TODO this isn't good to call in the render loop...

    std::error_code error_code;
    const std::string typed_string = current_root.data();
    auto const typed_path = typed_string.empty() ? std::filesystem::path{} : std::filesystem::path{typed_string};
    auto lookup_path = typed_path;

    if (lookup_path.empty())
        lookup_path = std::filesystem::current_path(error_code);
    else if (lookup_path.is_relative())
        lookup_path = std::filesystem::absolute(lookup_path, error_code);

    if (error_code || lookup_path.empty())
        lookup_path = ".";

    error_code.clear();

    if (std::filesystem::is_directory(lookup_path, error_code))
        return {.typed_path = lookup_path, .directory = lookup_path, .filter = {}};

    std::filesystem::path directory = lookup_path.parent_path();

    if (directory.empty()) {
        error_code.clear();
        directory = std::filesystem::current_path(error_code);

        if (error_code || directory.empty())
            directory = ".";
    }

    return {.typed_path = lookup_path, .directory = directory, .filter = lookup_path.filename().string()};
}

void FilesystemCombo::draw_parent_entry(
        const std::filesystem::path& directory
)
{
    const std::filesystem::path parent = directory.parent_path();

    if (parent.empty() || parent == directory)
        return;

    if (ImGui::Selectable("../", false, ImGuiSelectableFlags_NoAutoClosePopups))
        update_root(parent);
}

bool FilesystemCombo::draw_cached_entries(
        const std::string_view filter,
        std::filesystem::path& selected_path
)
{
    bool has_visible_entries = false;
    bool changed = false;

    for (const auto& entry : cache->entries) {
        if (!filter.empty() && !entry.name.contains(filter))
            continue;

        has_visible_entries = true;

        if (entry.draw()) {
            if (entry.is_directory) {
                update_root(entry.path);
            } else {
                selected_path = entry.path;
                update_root(entry.path.parent_path());
                changed = true;
                ImGui::CloseCurrentPopup();
            }
        }
    }

    if (!has_visible_entries)
        ImGui::TextDisabled("No matching entries.");

    return changed;
}

void FilesystemCombo::accept_path(
        std::filesystem::path& selected_path,
        const std::filesystem::path& path
)
{
    selected_path = path;

    if (std::filesystem::is_directory(path))
        update_root(path);
    else if (path.has_parent_path())
        update_root(path.parent_path());
    else
        update_root("."); // TODO ??
}

} // namespace echomap
