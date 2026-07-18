/**
 * @file
 *
 * JSActionController implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-18
 */

#include "JSActionController.hpp"

#include <emscripten/em_js.h>

#include "../Logger.hpp"
#include "../errors/ConfigurationError.hpp"

namespace echomap
{

/**
 * @c EM_JS free functions in the @ref echomap::web namespace to generate JavaScript targets.
 */
namespace js
{

#pragma clang diagnostic push // Ignore unknown attributes for used Emscripten JS linkage.
#pragma clang diagnostic ignored "-Wunknown-attributes"

EM_JS(void,
      select_project_file,
      (),
      {
          if (Module.echomapOpenProjectFileChooser) {
              Module.echomapOpenProjectFileChooser();
          } else {
              console.error("Module.echomapOpenProjectFileChooser is not installed.");
          }
      });

#pragma clang diagnostic pop

} // namespace js

void JSActionController::select_project_file_impl()
{
    js::select_project_file();
}

void JSActionController::notify_project_file_impl(
        const std::filesystem::path& path
)
{
    notify_project_file(path);
}

} // namespace echomap

extern "C" EMSCRIPTEN_KEEPALIVE int echomap_on_project_file_picked(
        const char* const path
) noexcept
{
    using namespace echomap;

    if (path == nullptr)
        return 2;

    try {
        JSActionController::notify_project_file_impl(path);
        return 0;
    } catch (const ConfigurationError& error) {
        LOG_F_ERROR("Could not load path {} due to error: {}", path, error.what());
        return 3;
    } catch (const std::exception& error) {
        LOG_F_ERROR("Could not load path {} due to unexpected error: {}", path, error.what());
        return 4;
    } catch (...) {
        LOG_F_ERROR("Could not load path {} due to unknown error.", path);
        return 5;
    }
}
