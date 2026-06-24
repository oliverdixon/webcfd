/**
 * @file
 * @brief WebCFD entry point
 * @author Oliver Dixon
 * @date 2026-05-05
 */

#include "ConfigurationError.hpp"
#include "Logger.hpp"
#include "WebCFD.hpp"
#include "web/JSBridge.hpp"

/**
 * WebCFD common entry point.
 *
 * @return OS status exit code. 0 for success, 1 for failure.
 */
int main()
{
    try {
        /*
         * If we're targeting Web Assembly, dynamically allocate the application object and do not invoke the
         * destructor. If we're targeting native, statically allocate and clean up correctly.
         *
         * This is fiddly but required behaviour due to the use of native Wasm exception handling; the Emscripten
         * documentation notes that when an infinite loop is simulated, native EH cannot be used with objects that have
         * destructors on the stack. Leaking the application state from a browser context is inconsequential; clean-up
         * is performed by the Wasm runtime.
         *
         * https://emscripten.org/docs/api_reference/emscripten.h.html#c.emscripten_set_main_loop
         */
#ifdef __EMSCRIPTEN__
        auto* const application = new WebCFD::WebCFD();
        WebCFD::JSBridge::bind(application);
        application->run_event_loop();
#else
        WebCFD::WebCFD application;
        application.run_event_loop();
#endif
    } catch (const WebCFD::ConfigurationError& error) {
#ifdef __EMSCRIPTEN__
        WebCFD::JSBridge::unbind();
#endif
        WebCFD::Logger::log(WebCFD::Logger::Level::Error, error.what(), error.where());
        return 1;
    }

#ifdef __EMSCRIPTEN__
    WebCFD::JSBridge::unbind();
#endif
    return 0;
}
