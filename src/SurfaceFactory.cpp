/**
 * @file
 * @brief WebCFD WebGPU-GLFW Surface factory implementation
 * @author Oliver Dixon
 * @date 2026-07-05
 */

#include <GLFW/glfw3.h>

#include "SurfaceFactory.hpp"

#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32 1
#define GOT_SUPPORTED_PLATFORM
#endif

#if defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA 1
#define GOT_SUPPORTED_PLATFORM
#endif

#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#define GLFW_EXPOSE_NATIVE_X11 1
#define GLFW_EXPOSE_NATIVE_WAYLAND 1
#define GOT_SUPPORTED_PLATFORM
#endif

#if defined(__EMSCRIPTEN__)
#define GOT_SUPPORTED_PLATFORM
#endif

#if !defined(GOT_SUPPORTED_PLATFORM)
#error "No supported platforms for the SurfaceFactory."
#error "Supported platforms are: Emscripten; Wayland; X11; Win32; Cocoa (Apple)."
#endif

#if !defined(__EMSCRIPTEN__)
#include <GLFW/glfw3native.h>
#endif

#if defined(__APPLE__)

// If we're on an Apple platform, define C++ shims to despatch messages through the Objective C interface.

#include <objc/message.h>
#include <objc/objc.h>
#include <objc/runtime.h>

namespace
{

template <typename T, typename... Args>
T objc_call(id obj, const char * const sel, Args... args) {
    using FuncPtr = T (*)(id, SEL, Args...);
    return reinterpret_cast<FuncPtr>(objc_msgSend)(obj, sel_registerName(sel), args...);
}

template <typename T, typename... Args>
T objc_call(const char *clazz, const char * const sel, Args... args) {
    return objc_call<T>(reinterpret_cast<id>(objc_getClass(clazz)), sel, args...);
}

}

#endif

namespace WebCFD
{

wgpu::Surface SurfaceFactory::create_surface(
        const wgpu::Instance& instance,
        GLFWwindow* window
)
{
    const auto chainedDescriptor = get_surface_descriptor(window);
    if (chainedDescriptor == nullptr)
        throw std::runtime_error("Could not create and bind a Surface to the given window.");

    wgpu::SurfaceDescriptor descriptor;
    descriptor.nextInChain = chainedDescriptor.get();

    return instance.CreateSurface(&descriptor);
}

std::unique_ptr<wgpu::ChainedStruct> SurfaceFactory::get_surface_descriptor(
        GLFWwindow* const window
)
{
    // Verify that the window is valid in the runtime.
    if (glfwGetWindowAttrib(window, GLFW_CLIENT_API) != GLFW_NO_API)
        return nullptr;

#if defined(__EMSCRIPTEN__)
    auto desc = std::make_unique<wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector>();
    desc->selector = "#canvas";
    return desc;
#else
    // Use the GLFW_EXPOSE_NATIVE_* flags to determine which GLFW platforms we can handle.

    // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
    switch (glfwGetPlatform()) {
#if defined(GLFW_EXPOSE_NATIVE_WAYLAND)
    case GLFW_PLATFORM_WAYLAND: {
        auto desc = std::make_unique<wgpu::SurfaceSourceWaylandSurface>();
        desc->display = glfwGetWaylandDisplay();
        desc->surface = glfwGetWaylandWindow(window);
        return desc;
    }
#endif
#if defined(GLFW_EXPOSE_NATIVE_X11)
    case GLFW_PLATFORM_X11: {
        auto desc = std::make_unique<wgpu::SurfaceSourceXlibWindow>();
        desc->display = glfwGetX11Display();
        desc->window = glfwGetX11Window(window);
        return desc;
    }
#endif
#if defined(GLFW_EXPOSE_NATIVE_WIN32)
    case GLFW_PLATFORM_WIN32: {
        auto desc = std::make_unique<wgpu::SurfaceSourceWindowsHWND>();
        desc->hwnd = glfwGetWin32Window(window);
        desc->hinstance = GetModuleHandle(nullptr);
        return desc;
    }
#endif
#if defined(GLFW_EXPOSE_NATIVE_COCOA)
    case GLFW_PLATFORM_COCOA: {
        /*
         * On Apple platforms, more work is required to configure the Metal layer on the Cocoa Surface through the
         * Objective-C messaging APIs.
         */
        auto ns_window = glfwGetCocoaWindow(window);
        CFRetain(ns_window);

        auto view = objc_call<id>(ns_window, "contentView");
        CFRetain(view);

        objc_call<void, BOOL>(view, "setWantsLayer:", YES);
        auto layer = objc_call<id>("CAMetalLayer", "layer");
        auto scale_factor = objc_call<CGFloat>(ns_window, "backingScaleFactor");
        objc_call<void, CGFloat>(layer, "setContentsScale:", scale_factor);
        objc_call<void, id>(view, "setLayer:", layer);

        auto desc = std::make_unique<wgpu::SurfaceSourceMetalLayer>();
        desc->layer = layer;
        CFRelease(view);
        CFRelease(ns_window);

        return desc;
    }
#endif
    }
#endif

    // No handlers were enabled. This should be caught be a compile-time check.
    // ReSharper disable once CppDFAUnreachableCode
    return nullptr;
}

} // namespace WebCFD
