/**
 * @file
 * @brief WebCFD class implementation
 * @author Oliver Dixon
 * @date 2026-05-05
 */

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_wgpu.h>
#include <dawn/webgpu_cpp_print.h>
#include <imgui.h>
#include <webgpu/webgpu_glfw.h>

#include "ParametersPanel.hpp"
#include "RenderPanel.hpp"
#include "RobotoMedium.hpp"
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif

#include "ConfigurationError.hpp"
#include "Logger.hpp"
#include "WebCFD.hpp"

namespace WebCFD
{

WebCFD::WebCFD()
{
    static constexpr auto timed_wait_any = wgpu::InstanceFeatureName::TimedWaitAny;
    constexpr wgpu::InstanceDescriptor instance_desc{.requiredFeatureCount = 1, .requiredFeatures = &timed_wait_any};

    instance = wgpu::CreateInstance(&instance_desc);
    window = create_window(static_cast<int>(viewport_width), static_cast<int>(viewport_height));

    int actual_width;
    int actual_height;
    glfwGetFramebufferSize(window, &actual_width, &actual_height);
    assert(actual_width >= 0);
    assert(actual_height >= 0);
    viewport_width = static_cast<std::uint32_t>(actual_width);
    viewport_height = static_cast<std::uint32_t>(actual_height);

    surface = wgpu::glfw::CreateSurfaceForWindow(instance, window);

    if (const auto adapter_wait = instance.WaitAny(request_adapter(), operation_timeout);
            adapter_wait != wgpu::WaitStatus::Success || !adapter)
        throw ConfigurationError("WebGPU adapter request did not complete");

    if (const auto device_wait = instance.WaitAny(request_device(), operation_timeout);
            device_wait != wgpu::WaitStatus::Success || !device)
        throw ConfigurationError("WebGPU device request did not complete");

    surface.GetCapabilities(adapter, &surface_capabilities);
    configure_surface(surface, device, surface_capabilities, viewport_width, viewport_height);

    setup_imgui();
}

void WebCFD::run_event_loop()
{
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(&WebCFD::render_shim, this, 0, true);
#else
    while (!glfwWindowShouldClose(window)) {
        render();
        // ReSharper disable once CppExpressionWithoutSideEffects
        surface.Present();
        instance.ProcessEvents();
    }
#endif
}

WebCFD::~WebCFD() noexcept
{
    for (auto& panel : panels)
        panel.reset();

    if (ImGui::GetCurrentContext()) {
        ImGui_ImplWGPU_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    if (surface) {
        surface.Unconfigure();
        surface = nullptr;
    }

    device = nullptr;
    adapter = nullptr;
    instance = nullptr;

    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();
}

GLFWwindow* WebCFD::create_window(
        // ReSharper disable once CppDFAConstantParameter
        const int width,
        // ReSharper disable once CppDFAConstantParameter
        const int height
)
{
    if (!glfwInit())
        throw ConfigurationError("glfwInit failed");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    const auto window = glfwCreateWindow(width, height, "WebCFD", nullptr, nullptr);

    if (!window) {
        glfwTerminate();
        throw ConfigurationError("glfwCreateWindow failed");
    }

    return window;
}

void WebCFD::configure_surface(
        const wgpu::Surface& surface,
        const wgpu::Device& device,
        const wgpu::SurfaceCapabilities& capabilities,
        const std::uint32_t viewport_width,
        const std::uint32_t viewport_height
) noexcept
{
    const wgpu::SurfaceConfiguration config{
            .device = device,
            // ReSharper disable once CppDFAConstantConditions
            // ReSharper disable once CppDFAUnreachableCode
            .format = capabilities.formats != nullptr ? capabilities.formats[0] : wgpu::TextureFormat::RGBA8Snorm,
            .usage = wgpu::TextureUsage::RenderAttachment,
            .width = viewport_width,
            .height = viewport_height,
            // ReSharper disable once CppDFAConstantConditions
            // ReSharper disable once CppDFAUnreachableCode
            .alphaMode =
                    capabilities.alphaModes != nullptr ? capabilities.alphaModes[0] : wgpu::CompositeAlphaMode::Opaque,
            .presentMode = wgpu::PresentMode::Fifo
    };

    surface.Configure(&config);
}

wgpu::Future WebCFD::request_adapter() noexcept
{
    const wgpu::RequestAdapterOptions options{.compatibleSurface = surface};

    return instance.RequestAdapter(
            &options,
            wgpu::CallbackMode::WaitAnyOnly,
            [this](const wgpu::RequestAdapterStatus status, wgpu::Adapter new_adapter, const wgpu::StringView message)
            {
                if (status != wgpu::RequestAdapterStatus::Success)
                    Logger::log_f(
                            Logger::Level::Error,
                            std::source_location::current(),
                            "WebGPU adapter request failed {}: {}.",
                            std::to_underlying(status),
                            std::string_view(message)
                    );
                else
                    adapter = std::move(new_adapter);
            }
    );
}

wgpu::Future WebCFD::request_device() noexcept
{
    wgpu::DeviceDescriptor desc{};
    desc.SetDeviceLostCallback(
            wgpu::CallbackMode::AllowSpontaneous,
            [](const wgpu::Device&, const wgpu::DeviceLostReason reason, const wgpu::StringView message) {
                if (reason == wgpu::DeviceLostReason::Destroyed)
                    return;

                Logger::log_f(
                        Logger::Level::Warning,
                        std::source_location::current(),
                        "Device lost because of reason {}: {}.",
                        std::to_underlying(reason),
                        std::string_view(message)
                );
            }
    );

    desc.SetUncapturedErrorCallback(
            [](const wgpu::Device&, const wgpu::ErrorType error_type, const wgpu::StringView message) {
                Logger::log_f(
                        Logger::Level::Error,
                        std::source_location::current(),
                        "WebGPU error {}: {}",
                        std::to_underlying(error_type),
                        std::string_view(message)
                );
            }
    );

    return adapter.RequestDevice(
            &desc,
            wgpu::CallbackMode::WaitAnyOnly,
            [this](const wgpu::RequestDeviceStatus status, wgpu::Device new_device, const wgpu::StringView message) {
                if (status != wgpu::RequestDeviceStatus::Success)
                    Logger::log_f(
                            Logger::Level::Error,
                            std::source_location::current(),
                            "WebGPU device request failed: {}",
                            std::string_view(message)
                    );
                else
                    device = std::move(new_device);
            }
    );
}

void WebCFD::render() noexcept
{
    glfwPollEvents();

    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Step 1.  Allow Dear ImGui panels to render their state to the frame, updating window size if necessary.
    if (!handle_window_resize())
        return;
    ImGui::DockSpaceOverViewport();
    for (const auto& panel : panels)
        panel->draw();
    ImGui::Render();

    // Step 2.  Set up a command encoder for the render and allow Dear ImGui panels to provide work.
    const wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    for (const auto& panel : panels)
        panel->update_gpu(encoder);

    // Step 3.  Provide the framebuffer to the WebGPU driver.

    wgpu::SurfaceTexture surface_texture{};
    surface.GetCurrentTexture(&surface_texture);

    const wgpu::TextureView surface_view = surface_texture.texture.CreateView();
    wgpu::RenderPassColorAttachment attachment{
            .view = surface_view,
            .loadOp = wgpu::LoadOp::Clear,
            .storeOp = wgpu::StoreOp::Store,
            .clearValue = {0.1, 0.1, 0.1, 1.0}
    };

    const wgpu::RenderPassDescriptor pass_descriptor{.colorAttachmentCount = 1, .colorAttachments = &attachment};
    const wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&pass_descriptor);

    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), pass.Get());
    pass.End();
    const wgpu::CommandBuffer commands = encoder.Finish();

    // Step 4.  Submit batched work to the GPU.
    device.GetQueue().Submit(1, &commands);
}

// ReSharper disable once CppMemberFunctionMayBeConst - Not semantically constant
void WebCFD::setup_imgui()
{
    if (!device)
        throw ConfigurationError("Cannot initialise ImGui: WebGPU device is null");

    // Step 1.  Bring up the Dear ImGui context and initialise the GLFW backend.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.Fonts->AddFontFromMemoryCompressedTTF(
            data::RobotoMedium_compressed_data,
            std::size(data::RobotoMedium_compressed_data),
            14
    );
    ImGui::StyleColorsLight();

    if (!ImGui_ImplGlfw_InitForOther(window, true))
        throw ConfigurationError("ImGui_ImplGlfw_InitForOther failed");

    // Step 2.  Configure the WebGPU backend for Dear ImGui.
    ImGui_ImplWGPU_InitInfo init_info{};
    init_info.Device = device.Get();
    // ReSharper disable once CppDFAConstantConditions
    // ReSharper disable once CppDFAUnreachableCode
    init_info.RenderTargetFormat = static_cast<WGPUTextureFormat>(std::to_underlying(
            surface_capabilities.formats ? surface_capabilities.formats[0] : wgpu::TextureFormat::RGBA8Snorm
    ));

    if (!ImGui_ImplWGPU_Init(&init_info))
        throw ConfigurationError("ImGui_ImplWGPU_Init failed");

#ifdef __EMSCRIPTEN__
    /*
     * If we're targeting WebAssembly, the window dimensions reported by GLFW should match the size of the canvas
     * identified by the CSS selector <code>#canvas</code>.
     */
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif

    // Step 3.  Construct and register panels to appear on the WebCFD application UI.
    panels.emplace_back(std::make_unique<ParametersPanel>(*parameters));
    panels.emplace_back(std::make_unique<RenderPanel>(device, viewport_width, viewport_height, *parameters));
}

// ReSharper disable once CppDFAUnreachableFunctionCall
bool WebCFD::handle_window_resize() noexcept
{
    int fb_width;
    int fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);

    if (fb_width <= 0 || fb_height <= 0)
        // The window is too small or collapsed.
        return false;

    const auto new_width = static_cast<std::uint32_t>(fb_width);
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto new_height = static_cast<std::uint32_t>(fb_height);

    if (new_width != viewport_width || new_height != viewport_height) {
        // The window has been resized, so update the WebGPU surface.

        viewport_width = new_width;
        viewport_height = new_height;

        surface.Unconfigure();
        configure_surface(surface, device, surface_capabilities, viewport_width, viewport_height);

        // The window size has changed and the surface re-configured accordingly.
        return true;
    }

    // The window size is unchanged.
    return true;
}

} // namespace WebCFD
