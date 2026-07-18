/**
 * @file
 *
 * JSActionController specification
 *
 * @author Oliver Dixon
 * @date 2026-07-18
 */

#ifndef ECHOMAP_JSACTIONCONTROLLER_HPP
#define ECHOMAP_JSACTIONCONTROLLER_HPP

#include "ActionControllerBase.hpp"

extern "C" int echomap_on_project_file_picked(const char* path) noexcept;

namespace echomap
{

/**
 * @todo Document
 */
class JSActionController : public ActionControllerBase<JSActionController>
{
    friend ActionControllerBase;
    friend int ::echomap_on_project_file_picked(const char* path) noexcept;

    static void select_project_file_impl();
    static void notify_project_file_impl(const std::filesystem::path& path);
};

} // namespace echomap

#endif // ECHOMAP_JSACTIONCONTROLLER_HPP
