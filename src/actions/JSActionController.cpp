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

#ifndef DOXYGEN_SKIP

// (Doxygen can't handle EM_JS declarations.)

/**
 * JavaScript functions to service actions.
 *
 * The JS function signatures are defined as @c extern C++ in the @ref echomap::js namespace. They are called using the
 * standard C++ calling convention from JSActionController member functions, and typically invoke an exported C-linkage
 * free function as a callback following completion of the action.
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

EM_JS(void,
      complete_signal_load,
      (std::size_t project_id,
       std::size_t signal_id),
      {
          if (Module.echomapOpenWAVFileChooserForExistingSignal) {
              Module.echomapOpenWAVFileChooserForExistingSignal(project_id, signal_id);
          } else {
              console.error("Module.echomapOpenWAVFileChooserForExistingSignal is not installed.");
          }
      });

#pragma clang diagnostic pop

} // namespace js

#endif

void JSActionController::select_project_file_impl()
{
    js::select_project_file();
}

void JSActionController::complete_signal_load_impl(
        const std::size_t project_id,
        const std::size_t signal_id
)
{
    js::complete_signal_load(project_id, signal_id);
}

void JSActionController::notify_project_file_impl(
        const std::filesystem::path& path
)
{
    notify_project_file(path);
}

void JSActionController::notify_complete_signal_load_impl(
        const std::size_t project_id,
        const std::size_t signal_id,
        const std::filesystem::path& path
)
{
    notify_complete_signal_load(project_id, signal_id, path);
}

} // namespace echomap

/**
 * Services the @ref ProjectFileAction callback for Emscripten.
 *
 * @param path The file-system path (in the Wasm VFS) selected in the prompt.
 * @return Zero status to indicate success; non-zero to indicate failure.
 *
 * @ingroup ProjectFileAction
 */
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

/**
 * Services the @ref CompletePartialSignalLoad callback for Emscripten.
 *
 * @param project_id The ID of the Project that owns the destination Signal.
 * @param signal_id The ID of the destination Signal.
 * @param path The path derived from the prompt.
 *
 * @return Zero status to indicate success; non-zero to indicate failure.
 *
 * @ingroup CompletePartialSignalLoad
 */
extern "C" EMSCRIPTEN_KEEPALIVE int echomap_on_signal_load_complete(
        const std::size_t project_id,
        const std::size_t signal_id,
        const char* const path
) noexcept
{
    using namespace echomap;

    if (path == nullptr)
        return 2;

    try {
        JSActionController::notify_complete_signal_load(project_id, signal_id, path);
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
