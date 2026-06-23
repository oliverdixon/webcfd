//
// Created by owd on 6/23/26.
//

#ifndef WEBCFD_JSBRIDGE_H
#define WEBCFD_JSBRIDGE_H

#ifdef __EMSCRIPTEN__

#include "WebCFD.hpp"

namespace WebCFD
{

class JSBridge
{
public:
    static void bind(WebCFD* new_instance) noexcept;

    static void unbind() noexcept;

    static int notify_wav_file_picked(const char * path) noexcept;

private:
    static WebCFD * instance;
};

} // namespace WebCFD

#endif // __EMSCRIPTEN__

#endif // WEBCFD_JSBRIDGE_H
