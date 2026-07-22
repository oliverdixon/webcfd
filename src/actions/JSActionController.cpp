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

#include "../errors/ConfigurationError.hpp"
#include "../utility/Logger.hpp"

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
      register_vfs_mapping,
      (std::size_t project_id,
       const char * external),
      {
          if (Module.echomapOpenVFSFileMapper) {
              const externalPath = UTF8ToString(external);
              Module.echomapOpenVFSFileMapper(project_id, externalPath);
          } else {
              console.error("Module.echomapOpenVFSFileMapper is not installed.");
          }
      });

#pragma clang diagnostic pop

} // namespace js

#endif

void JSActionController::select_project_file_impl()
{
    js::select_project_file();
}

void JSActionController::register_vfs_mapping_impl(
        const std::size_t project_id,
        const std::filesystem::path& external
)
{
    js::register_vfs_mapping(project_id, external.c_str());
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
        JSActionController::notify_project_file(path);
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
 * Services the @ref RegisterVFSMapping callback for Emscripten.
 *
 * @param project_id The ID of the Project that owns the destination Signal.
 * @param external The path of the external file being mapped into the VFS.
 * @param internal The path of the VFS file.
 *
 * @return Zero status to indicate success; non-zero to indicate failure.
 *
 * @ingroup RegisterVFSMapping
 */
extern "C" EMSCRIPTEN_KEEPALIVE int echomap_on_register_vfs_mapping(
        const std::size_t project_id,
        const char* const external,
        const char* const internal
) noexcept
{
    using namespace echomap;

    if (external == nullptr || internal == nullptr)
        return 2;

    try {
        JSActionController::notify_vfs_mapping(project_id, external, internal);
        return 0;
    } catch (const ConfigurationError& error) {
        LOG_F_ERROR("Could not load path {} due to error: {}", internal, error.what());
        return 3;
    } catch (const std::exception& error) {
        LOG_F_ERROR("Could not load path {} due to unexpected error: {}", internal, error.what());
        return 4;
    } catch (...) {
        LOG_F_ERROR("Could not load path {} due to unknown error.", internal);
        return 5;
    }
}
