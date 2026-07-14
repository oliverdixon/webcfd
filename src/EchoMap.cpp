/**
 * @file
 * @brief EchoMap class implementation
 * @author Oliver Dixon
 * @date 2026-05-05
 */

#include <imgui_impl_glfw.h>
#include <imgui_impl_wgpu.h>
#include <imgui_internal.h>
#include <implot.h>
#include <implot3d.h>

#include "tasks/LoadProjectResult.hpp"
#include "tasks/LoadProjectTask.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "EchoMap.hpp"
#include "Logger.hpp"
#include "RobotoMedium.hpp"
#include "SurfaceFactory.hpp"
#include "errors/ConfigurationError.hpp"
#include "objects/Project.hpp"
#include "panels/ChannelMappingPanel.hpp"
#include "panels/MenuPanel.hpp"
#include "panels/ProjectPanel.hpp"
#include "panels/SensorGeometryPanel.hpp"
#include "panels/SignalWaveformPanel.hpp"
#include "panels/SignalDFTPanel.hpp"

#if defined(__EMSCRIPTEN__) and !defined(__EMSCRIPTEN_PTHREADS__)
#warning "The Emscripten application will be single-threaded."
#endif

namespace
{

template <class T>
inline constexpr bool lightweight_task_v = std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>;

template <class T> struct all_lightweight_task : std::false_type
{};

template <class... Ts>
struct all_lightweight_task<std::variant<Ts...>> : std::bool_constant<(lightweight_task_v<Ts> && ...)>
{};

template <class T> inline constexpr bool all_lightweight_task_v = all_lightweight_task<std::remove_cvref_t<T>>::value;

} // namespace

namespace echomap
{

EchoMap::EchoMap() :
    worker{[] {
#ifndef __EMSCRIPTEN__
        glfwPostEmptyEvent();
#endif
    }},
    dockspace_id(ImHashStr("MainDockSpace"))
{
    static_assert(all_lightweight_task_v<LightweightTask>);

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

    surface = SurfaceFactory::create_surface(instance, window);

    if (const auto adapter_wait = instance.WaitAny(request_adapter(), operation_timeout);
        adapter_wait != wgpu::WaitStatus::Success || !adapter)
        throw ConfigurationError("WebGPU adapter request did not complete");

    if (const auto device_wait = instance.WaitAny(request_device(), operation_timeout);
        device_wait != wgpu::WaitStatus::Success || !device)
        throw ConfigurationError("WebGPU device request did not complete");

    surface.GetCapabilities(adapter, &surface_capabilities);
    configure_surface(surface, device, surface_capabilities, viewport_width, viewport_height);

    setup_imgui();

    panels.push_back(std::make_unique<MenuPanel>());
    panels.push_back(std::make_unique<ProjectPanel>());
    panels.push_back(std::make_unique<SignalWaveformPanel>(worker));
    panels.push_back(std::make_unique<SensorGeometryPanel>(*this));
    panels.push_back(std::make_unique<ChannelMappingPanel>(*this));
    panels.push_back(std::make_unique<SignalDFTPanel>(worker, *this));

    // TODO remove: test async project load.
    worker.submit(std::make_unique<LoadProjectTask>("../resources/ExampleProject.json"));
}

void EchoMap::run_event_loop()
{
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(&EchoMap::render_shim, this, 0, true);
#else
    render();
    instance.ProcessEvents();

    while (!glfwWindowShouldClose(window)) {
        if (forced_frames > 0) {
            glfwPollEvents();
            --forced_frames;
        } else
            glfwWaitEvents();

        render();
        instance.ProcessEvents();
    }
#endif
}

EchoMap::~EchoMap() noexcept
{
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplWGPU_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot3D::DestroyContext();
        ImPlot::DestroyContext();
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

void EchoMap::update_wav_file(
        const char* const path
)
{
    throw std::runtime_error("Unimplemented");
}

GLFWwindow* EchoMap::create_window(
        // ReSharper disable once CppDFAConstantParameter
        const int width,
        // ReSharper disable once CppDFAConstantParameter
        const int height
)
{
    if (!glfwInit())
        throw ConfigurationError("glfwInit failed");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    const auto window = glfwCreateWindow(width, height, "EchoMap", nullptr, nullptr);

    if (!window) {
        glfwTerminate();
        throw ConfigurationError("glfwCreateWindow failed");
    }

    return window;
}

void EchoMap::configure_surface(
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

wgpu::Future EchoMap::request_adapter() noexcept
{
    const wgpu::RequestAdapterOptions options{.compatibleSurface = surface};

    return instance.RequestAdapter(
            &options,
            wgpu::CallbackMode::WaitAnyOnly,
            [this](const wgpu::RequestAdapterStatus status, wgpu::Adapter new_adapter, const wgpu::StringView message) {
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

wgpu::Future EchoMap::request_device() noexcept
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

void EchoMap::render() noexcept
{
    // Process any events that have arrived since the last cycle.
    process_lightweight_tasks();
    process_worker_results();

    // Handle system/graphics changes.
    if (!handle_window_resize())
        return;

    wgpu::SurfaceTexture surface_texture{};
    surface.GetCurrentTexture(&surface_texture);

    switch (surface_texture.status) {
    case wgpu::SurfaceGetCurrentTextureStatus::Error:
    case wgpu::SurfaceGetCurrentTextureStatus::Lost:
    case wgpu::SurfaceGetCurrentTextureStatus::Outdated:
        surface.Unconfigure();
        configure_surface(surface, device, surface_capabilities, viewport_width, viewport_height);
        return;

    case wgpu::SurfaceGetCurrentTextureStatus::Timeout:
        return;

    case wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal:
    case wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal:
        break;
    }

    const wgpu::TextureView surface_view = surface_texture.texture.CreateView();

    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    if (!dockspace_configured)
        dockspace_configured = true;

    ImGui::DockSpaceOverViewport(dockspace_id, viewport, ImGuiDockNodeFlags_None);

    // Draw the panels and express any applicable error state.
    for (const auto& panel : panels)
        panel->draw();

    error_modal.draw();
    ImGui::Render();

    // Set up a command encoder for the render and allow Dear ImGui panels to provide work.
    const wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // TODO no panels use GPU yet.

    // Provide the framebuffer to the WebGPU driver.
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

    // Submit batched work to the GPU.
    device.GetQueue().Submit(1, &commands);

#ifndef __EMSCRIPTEN__
    // ReSharper disable once CppExpressionWithoutSideEffects
    surface.Present();
#endif
}

// ReSharper disable once CppMemberFunctionMayBeConst - Not semantically constant
void EchoMap::setup_imgui()
{
    if (!device)
        throw ConfigurationError("Cannot initialise ImGui: WebGPU device is null");

    // Bring up the Dear ImGui context and initialise the GLFW backend.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImPlot3D::CreateContext();

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

    // Configure the WebGPU backend for Dear ImGui.
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
     * identified by the CSS selector <code>#canvas</code>. Dear ImGui provides the helper
     * ImGui_ImplGlfw_InstallEmscriptenCallbacks to interface with the DOM and trigger re-draws as needed.
     */
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
}

// ReSharper disable once CppDFAUnreachableFunctionCall
bool EchoMap::handle_window_resize() noexcept
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

void EchoMap::process_lightweight_tasks()
{
    while (!lightweight_tasks.empty()) {
        const auto task_hint = static_cast<void*>(&lightweight_tasks.back());
        const auto task_position = lightweight_tasks.size() - 1;

        LOG_F_DEBUG("Consuming lightweight task with hint {} (#{}).", task_hint, task_position);

        try {
            std::visit(
                    [this, hint = task_hint, position = task_position]<typename T>(T&& task) {
                        if (project == nullptr) {
                            /*
                             * Lightweight tasks aren't necessarily dependent on an active project being defined, but
                             * currently they are...
                             */
                            LOG_F_DEBUG("Dropping lightweight task with hint {} (#{}).", hint, position);
                            return;
                        }

                        using TaskT = std::decay_t<T>;

                        if constexpr (std::is_same_v<TaskT, AddChannelMappingTask>)
                            project->add_association(task.signal_id, task.sensor_id);
                        else if constexpr (std::is_same_v<TaskT, ModifySensorColourTask>)
                            project->get_mutable_sensor(task.sensor_id).set_colour(std::move(task.colour));
                        else if constexpr (std::is_same_v<TaskT, ModifySensorPositionTask>)
                            project->get_mutable_sensor(task.sensor_id).set_position(std::move(task.position));
                    },
                    lightweight_tasks.back()
            );
        } catch (const std::exception& exception) {
            error_modal.raise_error(exception.what());
            LOG_F_ERROR(
                    "LWT with hint {} (#{}) was responsible for error: {}",
                    task_hint,
                    task_position,
                    exception.what()
            );
        }

        lightweight_tasks.pop_back();
    }
}

void EchoMap::process_worker_results()
{
    while (const auto result = worker.try_get_result())
        try {
            result->despatch(*this);
        } catch (const std::exception& exception) {
            Logger::log(Logger::Level::Error, exception.what(), std::source_location::current());
        }
}

void EchoMap::update_panel_project() const
{
    for (const auto& panel : panels)
        panel->set_active_project(project.get());
}

std::unique_ptr<Project> EchoMap::take_project(
        const bool update_ui
) noexcept
{
    auto taken = std::move(project);
    if (update_ui)
        update_panel_project();
    return std::move(taken);
}

void EchoMap::put_project(
        std::unique_ptr<Project> new_project
) noexcept
{
    if (project != new_project) {
        /*
         * We know that the memory addresses differ (i.e. project.get() != new_project.get()). Therefore, there is an
         * effective difference if and only if:
         *
         *  1. The old project is nullptr, thus the new project is non-vacuous;
         *  2. The old project is non-null, thus the new project is vacuous;
         *  3. Both the old and new projects are non-null, so we can compare their object IDs.
         */
        const auto effectively_different =
                project == nullptr || new_project == nullptr || project->get_id() != new_project->get_id();

        project = std::move(new_project);

        if (effectively_different) {
            // Invalidate project-dependent state.
            lightweight_tasks.clear();
            worker.clear();
            update_panel_project();
        }
    }
}

void EchoMap::submit_lightweight_task(
        LightweightTask task
)
{
    lightweight_tasks.emplace_back(std::move(task));

    /*
     * The address is just a "hint" (as opposed to an ID) because the queue might be re-allocated. It's a best-guess
     * effort to quickly discriminate on lightweight tasks without adding bloat to their structures.
     */
    LOG_F_DEBUG(
            "Scheduling lightweight task with hint {} at position {}.",
            static_cast<void*>(&lightweight_tasks.back()),
            lightweight_tasks.size() - 1
    );
}

void EchoMap::increment_forced_frames(
        const unsigned int count
) noexcept
{
    forced_frames += count;
}

void EchoMap::handle(
        LoadProjectResult& result
)
{
    put_project(result.take_project());

    for (const auto& panel : panels)
        panel->handle(result);
}

void EchoMap::handle(
        DownsampleResult& result
)
{
    for (const auto& panel : panels)
        panel->handle(result);
}

void EchoMap::handle(
        DFTResult& result
)
{
    for (const auto& panel : panels)
        panel->handle(result);
}

} // namespace echomap
