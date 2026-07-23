/**
 * @file
 *
 * FilesystemCombo specification
 *
 * @author Oliver Dixon
 * @date 2026-07-23
 */

#ifndef ECHOMAP_FILESYSTEMCOMBO_HPP
#define ECHOMAP_FILESYSTEMCOMBO_HPP

#include <imgui.h>

#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace echomap
{

class EchoMap;

/**
 * Provides a platform-independent Dear ImGui combo box for browsing the file-system and selecting files.
 */
class FilesystemCombo
{
public:
    /**
     * Sets up a new FilesystemCombo with the given EchoMap application context.
     *
     * The initial location is the current working directory.
     *
     * @param app The owning application instance.
     * @throws std::runtime_error The CWD could not be interrogated.
     */
    explicit FilesystemCombo(EchoMap* app);

    /**
     * Draws the FilesystemCombo control.
     *
     * @param selected_path Output parameter for the selected file.
     * @return Has a new file been selected?
     *
     * @pre A Dear ImGui panel's draw cycle must already be underway.
     */
    bool operator()(std::filesystem::path& selected_path);

private:
    static constexpr unsigned int max_path_length = 4096; /**< Maximum number of characters in a filesystem path. */

    /**
     * A drawable and selectable representation of a file or directory on the file-system.
     */
    struct Entry
    {
        /**
         * Create a new Entry for a regular file or directory.
         *
         * @param path The path of the file to represent in the Entry.
         * @param is_directory_hint Should the Entry represent a directory?
         *
         * @pre The directory hint matches metadata from
         *  @ref std::filesystem::is_directory(const std::filesystem::path&).
         */
        Entry(const std::filesystem::path& path,
              bool is_directory_hint);

        /**
         * Draws the Entry as a Dear ImGui Selectable.
         *
         * @return Is the Entry selected?
         */
        [[nodiscard]] bool draw() const noexcept;

        /**
         * Determines the ordering between two Entry objects.
         *
         * @param lhs The LHS Entry.
         * @param rhs The RHS Entry.
         *
         * @return The @ref std::strong_ordering between the LHS and the RHS.
         */
        friend std::strong_ordering operator<=>(
                const Entry& lhs,
                const Entry& rhs
        ) noexcept
        {
            if (lhs.is_directory != rhs.is_directory)
                return lhs.is_directory ? std::strong_ordering::less : std::strong_ordering::greater;

            return lhs.name <=> rhs.name;
        }

        /**
         * Determines equality between two Entry objects.
         *
         * @return Are the two entry objects considered equal?
         */
        friend bool operator==(
                const Entry&,
                const Entry&
        ) = default;

        // NOLINTBEGIN(*-non-private-member-variables-in-classes) - Acceptable for private sub-struct.

        std::filesystem::path path; /**< The path represented by the Entry. */
        std::string name;           /**< The cached display name of the path. */
        bool is_directory;          /**< Directory sentinel flag. */

        // NOLINTEND(*-non-private-member-variables-in-classes)

    private:
        std::string imgui_id;       /**< Dear ImGui display name, including ID disambiguator. */
        ImGuiSelectableFlags flags; /**< Dear ImGui flags for rendering the Selectable. */
    };

    struct BrowseTarget
    {
        std::filesystem::path typed_path;
        std::filesystem::path directory;
        std::string filter;
    };

    /**
     * A cache of Entry objects under a single root directory.
     */
    struct EntryCache
    {
        /**
         * Creates a new EntryCache and populates it for the given root directory.
         *
         * Upon constructing the EntryCache, the (immediate) children of the given directory are packed into Entry
         * objects and collated into the cache. Exceptions may be thrown from the @ref std::filesystem API. Symbolic
         * links are followed.
         *
         * @param directory The path of the directory root to be traversed.
         */
        explicit EntryCache(const std::filesystem::path& directory);

        std::filesystem::path directory; /**< The root directory on the file-system. */
        std::vector<Entry> entries;      /**< The first-level children of the root. */
    };

    /**
     * Invalidates the EntryCache.
     */
    void invalidate_cache();

    /**
     * Copies and formats the given path into the fixed-size current directory state member variable.
     *
     * The current directory array is always NULL-terminated following this operation.
     *
     * @param path The path to the new root directory.
     */
    void update_root(const std::filesystem::path& path);

    bool draw_combo_body(std::filesystem::path& selected_path);

    [[nodiscard]] BrowseTarget get_browse_target() const;

    void draw_parent_entry(const std::filesystem::path& directory);

    bool draw_cached_entries(
            std::string_view filter,
            std::filesystem::path& selected_path
    );

    void accept_path(
            std::filesystem::path& selected_path,
            const std::filesystem::path& path
    );

    std::array<char, max_path_length> current_root{};
    std::optional<EntryCache> cache; /**< Cache of entries. */
    EchoMap* app;                    /**< Pointer to the owning application instance. */
};

} // namespace echomap

#endif // ECHOMAP_FILESYSTEMCOMBO_HPP
