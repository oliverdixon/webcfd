//
// Created by owd on 6/23/26.
//

#ifdef __EMSCRIPTEN__

#include "JSBridge.hpp"

#include <emscripten/em_macros.h>

#include "Logger.hpp"

namespace WebCFD
{

WebCFD * JSBridge::instance = nullptr;

void JSBridge::bind(
        WebCFD* const new_instance
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
    if (instance == nullptr) {
        Logger::log(Logger::Level::Error, "No WebCFD app instance is registered,", std::source_location::current());
        return 1;
    }

    if (path == nullptr) {
        Logger::log(Logger::Level::Error, "File picker callback received no path.", std::source_location::current());
        return 2;
    }

    Logger::log_f(Logger::Level::Info, std::source_location::current(), "Received new file {}", path);
    // TODO process

    return 0;
}

} // namespace WebCFD

extern "C" EMSCRIPTEN_KEEPALIVE int webcfd_on_wav_file_picked(const char * const path)
{
    return WebCFD::JSBridge::notify_wav_file_picked(path);
}

#endif // __EMSCRIPTEN__
