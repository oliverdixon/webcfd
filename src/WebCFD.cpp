//
// Created by owd on 04/06/2026.
//

#include <iostream>

#include <dawn/webgpu_cpp_print.h>
#include <webgpu/webgpu_glfw.h>
#include <imgui.h>
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif

#include "WebCFD.hpp"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_wgpu.h"

namespace WebCFD
{
WebCFD::WebCFD()
{
    static constexpr auto timed_wait_any = wgpu::InstanceFeatureName::TimedWaitAny;
    constexpr wgpu::InstanceDescriptor instanceDesc{
        .requiredFeatureCount = 1,
        .requiredFeatures = &timed_wait_any
    };

    instance = wgpu::CreateInstance(&instanceDesc);
    window = create_window();
    surface = wgpu::glfw::CreateSurfaceForWindow(instance, window);

    instance.WaitAny(request_adapter(), default_timeout);
    instance.WaitAny(request_device(), default_timeout);

    configure_surface();
    configure_sample_shader();
    setup_gui();
}

WebCFD::~WebCFD()
{
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplWGPU_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    pipeline = nullptr;

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

void WebCFD::start()
{
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(&WebCFD::render_shim, this, 0, true);
#else
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        render();
        // ReSharper disable once CppExpressionWithoutSideEffects
        surface.Present();
        instance.ProcessEvents();
    }
#endif
}

GLFWwindow * WebCFD::create_window()
{
    if (!glfwInit())
        throw std::runtime_error("glfwInit failed");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    const auto window = glfwCreateWindow(
        default_width,
        default_height,
        "WebCFD",
        nullptr,
        nullptr
    );

    if (!window) {
        glfwTerminate();
        throw std::runtime_error("glfwCreateWindow failed");
    }

    return window;
}

wgpu::Future WebCFD::request_adapter()
{
    const wgpu::RequestAdapterOptions options{
        .compatibleSurface = surface
    };

    return instance.RequestAdapter(
        &options, wgpu::CallbackMode::WaitAnyOnly,
        [this](const wgpu::RequestAdapterStatus status, wgpu::Adapter new_adapter, const wgpu::StringView message)
        {
            if (status != wgpu::RequestAdapterStatus::Success) {
                std::cout << "RequestAdapter: " << message << "\n";
                exit(0);
            }

            adapter = std::move(new_adapter);
        });
}

wgpu::Future WebCFD::request_device()
{
    wgpu::DeviceDescriptor desc{};
    desc.SetDeviceLostCallback(
        wgpu::CallbackMode::AllowSpontaneous,
        [](const wgpu::Device &, const wgpu::DeviceLostReason reason, const wgpu::StringView message)
        {
            if (reason == wgpu::DeviceLostReason::Destroyed)
                return;

            std::cout << "Device lost: " << reason << " - message: " << message << "\n";
        }
    );

    desc.SetUncapturedErrorCallback(
        [](const wgpu::Device &, const wgpu::ErrorType errorType, const wgpu::StringView message)
        {
            std::cout << "Error: " << errorType << " - message: " << message << "\n";
        }
    );

    return adapter.RequestDevice(
        &desc, wgpu::CallbackMode::WaitAnyOnly,
        [this](const wgpu::RequestDeviceStatus status, wgpu::Device new_device, const wgpu::StringView message)
        {
            if (status != wgpu::RequestDeviceStatus::Success) {
                std::cout << "RequestDevice: " << message << "\n";
                exit(0);
            }

            device = std::move(new_device);
        }
    );
}

void WebCFD::configure_surface()
{
    wgpu::SurfaceCapabilities capabilities;
    surface.GetCapabilities(adapter, &capabilities);
    format = capabilities.formats[0];

    const wgpu::SurfaceConfiguration config{
        .device = device,
        .format = format,
        .usage = wgpu::TextureUsage::RenderAttachment,
        .width = default_width,
        .height = default_height,
        .alphaMode = capabilities.alphaModes[0],
        .presentMode = wgpu::PresentMode::Fifo
    };

    surface.Configure(&config);
}

void WebCFD::configure_sample_shader()
{
    static constexpr char shader_code[] = R"(
        @vertex fn vertexMain(@builtin(vertex_index) i : u32) ->
          @builtin(position) vec4f {
            const pos = array(vec2f(0, 1), vec2f(-1, -1), vec2f(1, -1));
            return vec4f(pos[i], 0, 1);
        }
        @fragment fn fragmentMain() -> @location(0) vec4f {
            return vec4f(1, 0, 0, 1);
        }
    )";

    wgpu::ShaderSourceWGSL wgsl{{.code = shader_code}};
    const wgpu::ShaderModuleDescriptor module_descriptor{.nextInChain = &wgsl};
    const wgpu::ShaderModule module = device.CreateShaderModule(&module_descriptor);

    wgpu::ColorTargetState colour_target{.format = format};
    wgpu::FragmentState fragment{
        .module = module,
        .targetCount = 1,
        .targets = &colour_target
    };

    const wgpu::RenderPipelineDescriptor pipeline_descriptor{
        .vertex = {.module = module},
        .fragment = &fragment
    };

    pipeline = device.CreateRenderPipeline(&pipeline_descriptor);
}

// ReSharper disable once CppMemberFunctionMayBeConst - Not semantically constant
void WebCFD::render()
{
    wgpu::SurfaceTexture surfaceTexture;
    surface.GetCurrentTexture(&surfaceTexture);

    wgpu::RenderPassColorAttachment attachment{
        .view = surfaceTexture.texture.CreateView(),
        .loadOp = wgpu::LoadOp::Clear,
        .storeOp = wgpu::StoreOp::Store
    };

    const wgpu::RenderPassDescriptor pass_descriptor{
        .colorAttachmentCount = 1,
        .colorAttachments = &attachment
    };

    const wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    const wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&pass_descriptor);
    pass.SetPipeline(pipeline);
    pass.Draw(3);
    update_gui(pass);
    pass.End();

    const wgpu::CommandBuffer commands = encoder.Finish();
    device.GetQueue().Submit(1, &commands);
}

// ReSharper disable once CppMemberFunctionMayBeConst - Not semantically constant
void WebCFD::setup_gui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    if (!ImGui_ImplGlfw_InitForOther(window, true))
        throw std::runtime_error("ImGui_ImplGlfw_InitForOther failed");

    ImGui_ImplWGPU_InitInfo init_info{};
    init_info.Device = device.Get();
    init_info.RenderTargetFormat = static_cast<WGPUTextureFormat>(std::to_underlying(format));

    if (!ImGui_ImplWGPU_Init(&init_info))
        throw std::runtime_error("ImGui_ImplWGPU_Init failed");
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppDFAUnreachableFunctionCall
void WebCFD::update_gui(const wgpu::RenderPassEncoder &render_pass)
{
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static float f = 0.0f;
    static int counter = 0;
    static bool show_demo_window = true;
    static bool show_another_window = false;
    static auto clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImGui::Begin("Hello, world!");
    ImGui::Text("This is some useful text.");
    ImGui::Checkbox("Demo Window", &show_demo_window);
    ImGui::Checkbox("Another Window", &show_another_window);

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
    ImGui::ColorEdit3("clear color", reinterpret_cast<float *>(&clear_color));

    if (ImGui::Button("Button"))
        counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
        ImGui::GetIO().Framerate);

    ImGui::End();

    ImGui::Render();
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), render_pass.Get());
}

} // WebCFD
