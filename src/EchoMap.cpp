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

#include "signals/tasks/LoadProjectTask.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "EchoMap.hpp"
#include "Logger.hpp"
#include "RobotoMedium.hpp"
#include "SurfaceFactory.hpp"
#include "VariantHelpers.hpp"
#include "errors/ConfigurationError.hpp"
#include "errors/IgnoredWarning.hpp"
#include "objects/Project.hpp"
#include "panels/ChannelMappingPanel.hpp"
#include "panels/MenuPanel.hpp"
#include "panels/ProjectPanel.hpp"
#include "panels/SensorGeometryPanel.hpp"
#include "panels/SignalDFTPanel.hpp"
#include "panels/SignalWaveformPanel.hpp"
#include "signals/tasks/LoadSignalFileTask.hpp"

#if defined(__EMSCRIPTEN__) and !defined(__EMSCRIPTEN_PTHREADS__)
#warning "The Emscripten application will be single-threaded."
#endif

namespace echomap
{

EchoMap::EchoMap() :
    window(create_window(
            static_cast<int>(viewport_width),
            static_cast<int>(viewport_height)
    )),
    worker{[] {
#ifndef __EMSCRIPTEN__
        glfwPostEmptyEvent();
#endif
    }},
    dockspace_id(ImHashStr("MainDockSpace"))
{
    setup_subscriptions();

    static constexpr auto timed_wait_any = wgpu::InstanceFeatureName::TimedWaitAny;
    constexpr wgpu::InstanceDescriptor instance_desc{.requiredFeatureCount = 1, .requiredFeatures = &timed_wait_any};

    instance = wgpu::CreateInstance(&instance_desc);

    int actual_width = 0;
    int actual_height = 0;
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
    panels.push_back(std::make_unique<ProjectPanel>(despatcher));
    panels.push_back(std::make_unique<SignalWaveformPanel>(&worker, despatcher));
    panels.push_back(std::make_unique<SensorGeometryPanel>(despatcher, this));
    panels.push_back(std::make_unique<ChannelMappingPanel>(despatcher, this));
    panels.push_back(std::make_unique<SignalDFTPanel>(&worker, despatcher, this));
}

void EchoMap::run_event_loop()
{
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(&EchoMap::render_shim, this, 0, true);
#else
    render();
    instance.ProcessEvents();

    while (glfwWindowShouldClose(window) == 0) {
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
    if (ImGui::GetCurrentContext() != nullptr) {
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

    if (window != nullptr) {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();
}

GLFWwindow* EchoMap::create_window(
        // ReSharper disable once CppDFAConstantParameter
        const int width,
        // ReSharper disable once CppDFAConstantParameter
        const int height
)
{
    if (glfwInit() == 0)
        throw ConfigurationError("glfwInit failed");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    auto* const window = glfwCreateWindow(width, height, "EchoMap", nullptr, nullptr);

    if (window == nullptr) {
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
            .format = capabilities.formats != nullptr ? *capabilities.formats : wgpu::TextureFormat::RGBA8Snorm,
            .usage = wgpu::TextureUsage::RenderAttachment,
            .width = viewport_width,
            .height = viewport_height,
            // ReSharper disable once CppDFAConstantConditions
            // ReSharper disable once CppDFAUnreachableCode
            .alphaMode =
                    capabilities.alphaModes != nullptr ? *capabilities.alphaModes : wgpu::CompositeAlphaMode::Opaque,
            .presentMode = wgpu::PresentMode::Fifo,
    };

    surface.Configure(&config);
}

void EchoMap::setup_subscriptions()
{
    connections.emplace_back(
            despatcher.load_project_finished_channel.nominate_consumer([this](LoadProjectResult&& result) {
                if (upload_modal.has_value()) {
                    // How have we loaded a new project whilst we're still querying for sources from another one?
                    LOG_WARN("Ignoring request to change active Project since there is an active modal.");
                    return;
                }

                auto&& new_project = std::move(result).take_project();

                if (!new_project->unloaded_signals.empty()) {
                    // Raise the modal to query for the sources.
                    upload_modal = IndividualUploadModal(this, new_project.get());
                    unloaded_project = std::move(new_project);
                } else
                    change_active_project(std::move(new_project));
            })
    );

    connections.emplace_back(
            despatcher.load_signal_file_channel.nominate_consumer([this](LoadSignalFileResult&& result) {
                Project* target = nullptr;
                if (project != nullptr && result.get_project_id() == project->get_id())
                    target = project.get();
                else if (unloaded_project != nullptr && result.get_project_id() == unloaded_project->get_id())
                    target = unloaded_project.get();

                if (target == nullptr)
                    LOG_F_WARN(
                            "Dropping LoadSignalFileResult, which was intended for the unavailable Project with ID {}.",
                            result.get_project_id()
                    );
                else {
                    for (auto&& signals = std::move(result).take_signals(); auto signal : signals)
                        target->add_signal(std::move(signal));

                    if (target == unloaded_project.get())
                        change_active_project(std::move(unloaded_project));
                }
            })
    );

    connections.emplace_back(despatcher.error_channel.observe([this](const ErrorResult& error) {
        error_modal.raise_error(error.what());
        LOG_F_ERROR("Error modal raised due to error: {}", error.what());
    }));
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

    if (upload_modal.has_value())
        upload_modal->draw();

    error_modal.draw();
    ImGui::Render();

    // Set up a command encoder for the render and allow Dear ImGui panels to provide work.
    const wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // TODO no panels use GPU yet.

    // Provide the framebuffer to the WebGPU driver.
    wgpu::RenderPassColorAttachment const attachment{
            .view = surface_view,
            .loadOp = wgpu::LoadOp::Clear,
            .storeOp = wgpu::StoreOp::Store,
            .clearValue = {.r = 0.1, .g = 0.1, .b = 0.1, .a = 1.0},
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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // NOLINT(*-signed-bitwise) - Required by ImGui API.
    io.Fonts->AddFontFromMemoryCompressedTTF(
            // ReSharper disable once CppRedundantCastExpression
            static_cast<const void*>(data::RobotoMedium_compressed_data),
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
            (surface_capabilities.formats != nullptr) ? *surface_capabilities.formats : wgpu::TextureFormat::RGBA8Snorm
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
    int fb_width = 0;
    int fb_height = 0;
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
    while (!lwt_queue.empty()) {
        auto* const task_hint = static_cast<void*>(&lwt_queue.back());
        const auto task_position = lwt_queue.size() - 1;

        LOG_F_DEBUG("Consuming LWT with hint {} (#{}).", task_hint, task_position);

        try {

            // clang-format off
            std::visit(variant_helpers::Overloaded{
                [this](const AddChannelMappingTask& task) { handle_lwt(task); },
                [this](const ModifySensorColourTask& task) { handle_lwt(task); },
                [this](const ModifySensorPositionTask& task) { handle_lwt(task); },
                [this](const ProjectLoadRequest& task) { handle_lwt(task); },
                [this](const CompleteProjectLoadNotification& task) { handle_lwt(task); },
                [this](const RegisterVFSMappingNotification& task) { handle_lwt(task); },
                },
                lwt_queue.back()
            );
            // clang-format on

        } catch (const IgnoredWarning& warning) {
            LOG_F_WARN("LWT with hint {} (#{}) was dropped: {}", task_hint, task_position, warning.what());
        } catch (const std::exception& exception) {
            error_modal.raise_error(exception.what());
            LOG_F_ERROR(
                    "LWT with hint {} (#{}) was responsible for error: {}",
                    task_hint,
                    task_position,
                    exception.what()
            );
        }

        lwt_queue.pop_back();
    }
}

void EchoMap::process_worker_results()
{
    while (auto result = worker.try_get_result())
        try {
            despatcher.publish(std::move(*result));
        } catch (const std::exception& exception) {
            Logger::log(Logger::Level::Error, exception.what(), std::source_location::current());
        }
}

void EchoMap::handle_lwt(
        const AddChannelMappingTask& task
) const
{
    if (project == nullptr)
        throw IgnoredWarning("Dropping AddChannelMappingTask due to empty project.");

    project->add_association(task.signal_id, task.sensor_id);
}

void EchoMap::handle_lwt(
        const ModifySensorColourTask& task
) const
{
    if (project == nullptr)
        throw IgnoredWarning("Dropping ModifySensorColourTask due to empty project.");

    project->get_mutable_sensor(task.sensor_id).set_colour(task.colour);
}

void EchoMap::handle_lwt(
        const ModifySensorPositionTask& task
) const
{
    if (project == nullptr)
        throw IgnoredWarning("Dropping ModifySensorPositionTask due to empty project.");

    project->get_mutable_sensor(task.sensor_id).set_position(task.position);
}

void EchoMap::handle_lwt(
        const ProjectLoadRequest& task
)
{
    worker.submit(std::make_unique<LoadProjectTask>(task.path, &worker));
}

void EchoMap::handle_lwt(
        const CompleteProjectLoadNotification& task
)
{
    upload_modal.reset();

    // Validate that we have correct project loaded.

    if (unloaded_project == nullptr)
        throw IgnoredWarning("Dropping CompleteProjectLoadNotification due to empty unloaded project.");

    if (unloaded_project->get_id() != task.project_id)
        throw IgnoredWarning(
                std::format(
                        "Dropping CompleteProjectLoadNotification due to incorrect unloaded project: requested {}, but "
                        "have {}.",
                        task.project_id,
                        unloaded_project->get_id()
                )
        );

    // For each group, create a worker task to load the corresponding file.

    for (auto&& [vfs_path, factories] :
         unloaded_project->unloaded_signals | std::views::values | std::views::as_rvalue) {

        if (!vfs_path.has_value())
            throw std::runtime_error("Refusing CompleteProjectLoadNotification due to an incomplete VFS mapping.");

        worker.submit(
                std::make_unique<LoadSignalFileTask>(unloaded_project->get_id(), *vfs_path, std::move(factories))
        );
    }
}

void EchoMap::handle_lwt(
        const RegisterVFSMappingNotification& task
) const
{
    // Validate that we have correct project loaded.

    if (unloaded_project == nullptr)
        throw IgnoredWarning("Dropping RegisterVFSMappingNotification due to empty unloaded project.");

    if (unloaded_project->get_id() != task.project_id)
        throw IgnoredWarning(
                std::format(
                        "Dropping RegisterVFSMappingNotification due to incorrect unloaded project: requested {}, but "
                        "have {}.",
                        task.project_id,
                        unloaded_project->get_id()
                )
        );

    // Add it.

    const auto map_it = unloaded_project->unloaded_signals.find(task.external);

    if (map_it == unloaded_project->unloaded_signals.end())
        throw IgnoredWarning(
                std::format(
                        "Dropping RegisterVFSMappingNotification since we don't need a mapping for {}.",
                        task.external.c_str()
                )
        );

    map_it->second.first = task.internal;
}

void EchoMap::change_active_project(
        std::unique_ptr<Project> new_project
) noexcept
{
    if (new_project != nullptr)
        LOG_F_DEBUG("Changing active project to {}.", new_project->get_name());

    project = std::move(new_project);
}

void EchoMap::submit_lightweight_task(
        const LightweightTask& task
)
{
    lwt_queue.emplace_back(task);

    /*
     * The address is just a "hint" (as opposed to an ID) because the queue might be re-allocated. It's a best-guess
     * effort to quickly discriminate on lightweight tasks without adding bloat to their structures.
     */
    LOG_F_DEBUG(
            "Scheduling LWT with hint {} at position {}.",
            static_cast<void*>(&lwt_queue.back()),
            lwt_queue.size() - 1
    );
}

void EchoMap::increment_forced_frames(
        const unsigned int count
) noexcept
{
    forced_frames += count;
}

} // namespace echomap
