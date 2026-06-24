/**
 * @file
 * @brief JavaScript-C++ bridge specification
 * @author Oliver Dixon
 * @date 2026-06-24
 */

#ifndef WEBCFD_JSBRIDGE_H
#define WEBCFD_JSBRIDGE_H

#ifdef __EMSCRIPTEN__

#include "WebCFD.hpp"

namespace WebCFD
{

/**
 * A set of static noexcept bindings for the JS runtime to call through exported functions with C linkage.
 *
 * <p>
 *  The JS bridge provides a static interface with C linkage helpers for JS APIs to manipulate the active WebCFD
 *  instance. Upon creating the active WebCFD instance, the application controller should JSBridge::bind the instance
 *  into the static context. The helpers can then forward requests from the JS runtime into the instance.
 * </p>
 * <p>
 *  This is minimally invasive glue to preserve OO structure within the core application, but make non-static instances
 *  accessible to the browser. Of course, it is only applicable when targeting Wasm (we use __EMSCRIPTEN__ as a litmus
 *  test).
 * </p>
 */
class JSBridge
{
public:
    /**
     * Bind a WebCFD instance to the static bridge context.
     *
     * Any existing binding is silently overwritten.
     *
     * @param new_instance A mutable raw pointer to the instance to bind.
     */
    static void bind(WebCFD* new_instance) noexcept;

    /**
     * Unbind the existing WebCFD instance from the static bridge context.
     */
    static void unbind() noexcept;

    /**
     * Notifies any bound instance that a new wave file has been selected and loaded into the Wasm file system.
     *
     * @param path Path of the accessible file within the Wasm file system.
     * @return A numerical error code (details are unimportant) indicating the result. Non-zero indicates failure.
     */
    static int notify_wav_file_picked(const char * path) noexcept;

private:
    /**
     * Performs common sanity-checks prior to calling on the bound instance.
     *
     * @return Should the request continue?
     */
    static bool preamble() noexcept;

    static WebCFD * instance;
};

} // namespace WebCFD

#endif // __EMSCRIPTEN__

#endif // WEBCFD_JSBRIDGE_H
