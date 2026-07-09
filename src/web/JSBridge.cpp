/**
 * @file
 * @brief JavaScript-C++ bridge implementation
 * @author Oliver Dixon
 * @date 2026-06-24
 */

#ifdef __EMSCRIPTEN__

#include "JSBridge.hpp"

#include <emscripten/em_macros.h>

#include "../errors/ConfigurationError.hpp"
#include "../Logger.hpp"

namespace echomap
{

EchoMap * JSBridge::instance = nullptr;

void JSBridge::bind(
        EchoMap* const new_instance
) noexcept
{
    instance = new_instance;
}

void JSBridge::unbind() noexcept
{
    instance = nullptr;
}

int JSBridge::notify_wav_file_picked(
        const char * const path
) noexcept
{
    if (!preamble())
        return 1;

    if (path == nullptr) {
        LOG_ERROR("File picker callback received no path.");
        return 2;
    }

    try {
        instance->update_wav_file(path);
        return 0;
    } catch (const ConfigurationError& error) {
        Logger::log_f(Logger::Level::Error, error.where(), "Could not load path {} due to error: {}.", path,
            error.what());
        return 3;
    }
}

bool JSBridge::preamble() noexcept
{
    if (instance == nullptr) {
        LOG_ERROR("No EchoMap app instance is registered");
        return false;
    }

    return true;
}

} // namespace echomap

/**
 * Action to handle a new wave file being selected on the DOM. This is exported and called from the JS.
 *
 * @param path Path of the selected file in the Wasm file system.
 * @return Numerical status code; non-zero on failure.
 * @see JSBridge::notify_wav_file_picked
 */
extern "C" EMSCRIPTEN_KEEPALIVE int echomap_on_wav_file_picked(const char * const path)
{
    return echomap::JSBridge::notify_wav_file_picked(path);
}

#endif // __EMSCRIPTEN__
