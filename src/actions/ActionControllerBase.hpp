/**
 * @file
 *
 * ActionControllerBase specification
 *
 * @author Oliver Dixon
 * @date 2026-07-18
 */

#ifndef ECHOMAP_ACTIONCONTROLLERBASE_HPP
#define ECHOMAP_ACTIONCONTROLLERBASE_HPP

#include <sigc++/slot.h>

#include <filesystem>

#include "../Logger.hpp"

namespace echomap
{

/**
 * @todo Document
 */
template <typename Derived> class ActionControllerBase
{
    struct Callbacks
    {
        using ProjectFile = sigc::slot<void(const std::filesystem::path&)>;
        ProjectFile project_file_slot;

        // TODO: more callbacks...
    };

    static Callbacks callbacks;

public:
    static void rebind(
            Callbacks::ProjectFile&& select_project_file_callback_v
    )
    {
        callbacks.project_file_slot = std::move(select_project_file_callback_v);
    }

    static void rebind()
    {
        callbacks.project_file_slot.disconnect();
    }

    static void select_project_file()
    {
        Derived::select_project_file_impl();
    }

protected:
    static void notify_project_file(
            const std::filesystem::path& path
    )
    {
        if (callbacks.project_file_slot.empty())
            LOG_F_WARN("Dropping project file selection for {} since no application instance is bound.", path.c_str());
        else
            callbacks.project_file_slot(path);
    }
};

template <typename Derived> ActionControllerBase<Derived>::Callbacks ActionControllerBase<Derived>::callbacks{};

} // namespace echomap

#endif // ECHOMAP_ACTIONCONTROLLERBASE_HPP
