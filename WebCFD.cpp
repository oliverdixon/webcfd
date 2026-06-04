//
// Created by owd on 04/06/2026.
//

#include <iostream>

#include <dawn/webgpu_cpp_print.h>
#include <GLFW/glfw3.h>
#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_glfw.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

constexpr std::uint32_t kWidth = 512;
constexpr std::uint32_t kHeight = 512;

wgpu::Instance instance;
wgpu::Adapter adapter;
wgpu::Device device;
wgpu::Surface surface;
wgpu::TextureFormat format;
wgpu::RenderPipeline pipeline;

constexpr char shaderCode[] = R"(
    @vertex fn vertexMain(@builtin(vertex_index) i : u32) ->
      @builtin(position) vec4f {
        const pos = array(vec2f(0, 1), vec2f(-1, -1), vec2f(1, -1));
        return vec4f(pos[i], 0, 1);
    }
    @fragment fn fragmentMain() -> @location(0) vec4f {
        return vec4f(1, 0, 0, 1);
    }
)";

void configure_surface()
{
    wgpu::SurfaceCapabilities capabilities;
    surface.GetCapabilities(adapter, &capabilities);
    format = capabilities.formats[0];

    const wgpu::SurfaceConfiguration config{
        .device = device,
        .format = format,
        .usage = wgpu::TextureUsage::RenderAttachment,
        .width = kWidth,
        .height = kHeight,
        .alphaMode = capabilities.alphaModes[0],
        .presentMode = wgpu::PresentMode::Fifo
    };

    surface.Configure(&config);

    wgpu::ShaderSourceWGSL wgsl{{.code = shaderCode}};
    const wgpu::ShaderModuleDescriptor shaderModuleDescriptor{.nextInChain = &wgsl};
    const wgpu::ShaderModule shaderModule =
            device.CreateShaderModule(&shaderModuleDescriptor);

    wgpu::ColorTargetState colorTargetState{.format = format};
    wgpu::FragmentState fragmentState{
        .module = shaderModule, .targetCount = 1, .targets = &colorTargetState
    };

    const wgpu::RenderPipelineDescriptor descriptor{
        .vertex = {.module = shaderModule},
        .fragment = &fragmentState
    };

    pipeline = device.CreateRenderPipeline(&descriptor);
}

void init()
{
    static constexpr auto kTimedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
    constexpr wgpu::InstanceDescriptor instanceDesc{
        .requiredFeatureCount = 1,
        .requiredFeatures = &kTimedWaitAny
    };

    instance = wgpu::CreateInstance(&instanceDesc);

    const wgpu::Future adapter_req = instance.RequestAdapter(
        nullptr, wgpu::CallbackMode::WaitAnyOnly,
        [](const wgpu::RequestAdapterStatus status, wgpu::Adapter new_adapter, const wgpu::StringView message)
        {
            if (status != wgpu::RequestAdapterStatus::Success) {
                std::cout << "RequestAdapter: " << message << "\n";
                exit(0);
            }

            adapter = std::move(new_adapter);
        }
    );

    instance.WaitAny(adapter_req, std::numeric_limits<std::uint64_t>::max());

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

    const wgpu::Future device_req = adapter.RequestDevice(
        &desc, wgpu::CallbackMode::WaitAnyOnly,
        [](const wgpu::RequestDeviceStatus status, wgpu::Device new_device, const wgpu::StringView message)
        {
            if (status != wgpu::RequestDeviceStatus::Success) {
                std::cout << "RequestDevice: " << message << "\n";
                exit(0);
            }

            device = std::move(new_device);
        }
    );

    instance.WaitAny(device_req, std::numeric_limits<std::uint64_t>::max());
}

void render()
{
    wgpu::SurfaceTexture surfaceTexture;
    surface.GetCurrentTexture(&surfaceTexture);

    wgpu::RenderPassColorAttachment attachment{
        .view = surfaceTexture.texture.CreateView(),
        .loadOp = wgpu::LoadOp::Clear,
        .storeOp = wgpu::StoreOp::Store
    };

    const wgpu::RenderPassDescriptor renderpass{
        .colorAttachmentCount = 1,
        .colorAttachments = &attachment
    };

    const wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    const wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderpass);
    pass.SetPipeline(pipeline);
    pass.Draw(3);
    pass.End();

    const wgpu::CommandBuffer commands = encoder.Finish();
    device.GetQueue().Submit(1, &commands);
}

void start()
{
    if (!glfwInit())
        return;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *const window = glfwCreateWindow(kWidth, kHeight, "WebGPU window", nullptr, nullptr);

    surface = wgpu::glfw::CreateSurfaceForWindow(instance, window);
    configure_surface();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(render, 0, false);
#else
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        render();
        // ReSharper disable once CppExpressionWithoutSideEffects
        surface.Present();
        instance.ProcessEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
#endif
}

int main()
{
    init();
    start();

    return 0;
}
