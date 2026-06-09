//
// Created by owd on 04/06/2026.
//

#include "ViewportRenderer.hpp"
#include "shaders/vortex.hpp"

#include <chrono>

namespace WebCFD
{

ViewportRenderer::ViewportRenderer(
        const wgpu::Device& device,
        const std::uint32_t width,
        const std::uint32_t height,
        const SimulationParameters& parameters
) :
    width(width),
    height(height),
    parameters(parameters),
    device(device)
{
    create_parameter_buffer();
    create_pipeline();
    recreate_texture();
}

void ViewportRenderer::render(
        const wgpu::CommandEncoder& command_encoder
) const
{
    if (!texture_view)
        return;

    static const auto start_time = std::chrono::steady_clock::now();

    const auto now = std::chrono::steady_clock::now();
    const float time = std::chrono::duration<float>(now - start_time).count();

    SimulationParameters frame_parameters = parameters;

    frame_parameters.viewport[0] = time;
    frame_parameters.viewport[1] = static_cast<float>(width) / static_cast<float>(height);
    frame_parameters.viewport[2] = static_cast<float>(width);
    frame_parameters.viewport[3] = static_cast<float>(height);

    device.GetQueue().WriteBuffer(
            uniform_buffer,
            0,
            &frame_parameters,
            sizeof(SimulationParameters)
    );

    wgpu::RenderPassColorAttachment attachment{
            .view = texture_view,
            .loadOp = wgpu::LoadOp::Clear,
            .storeOp = wgpu::StoreOp::Store,
            .clearValue = {0.0, 0.0, 0.0, 1.0}
    };

    const wgpu::RenderPassDescriptor pass_descriptor{
            .colorAttachmentCount = 1,
            .colorAttachments = &attachment
    };

    const wgpu::RenderPassEncoder pass = command_encoder.BeginRenderPass(&pass_descriptor);

    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bind_group);
    pass.Draw(3);
    pass.End();
}

void ViewportRenderer::resize(
        const std::uint32_t new_width,
        const std::uint32_t new_height
)
{
    if (new_width == 0 || new_height == 0)
        return;

    if (new_width == width && new_height == height)
        return;

    width = new_width;
    height = new_height;

    recreate_texture();
}

wgpu::TextureView ViewportRenderer::get_texture_view() const noexcept
{
    return texture_view;
}

void ViewportRenderer::recreate_texture()
{
    const wgpu::TextureDescriptor texture_descriptor{
            .usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
            .dimension = wgpu::TextureDimension::e2D,
            .size = {width, height, 1},
            .format = texture_format
    };

    texture = device.CreateTexture(&texture_descriptor);
    texture_view = texture.CreateView();
}

void ViewportRenderer::create_parameter_buffer()
{
    constexpr wgpu::BufferDescriptor uniform_buffer_descriptor{
            .label = "Simulation parameters buffer",
            .usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,
            .size = sizeof(SimulationParameters)
    };

    uniform_buffer = device.CreateBuffer(&uniform_buffer_descriptor);
}

void ViewportRenderer::create_pipeline()
{
    wgpu::BindGroupLayoutEntry uniform_layout_entry{};
    uniform_layout_entry.binding = 0;
    uniform_layout_entry.visibility = wgpu::ShaderStage::Fragment;
    uniform_layout_entry.buffer.type = wgpu::BufferBindingType::Uniform;
    uniform_layout_entry.buffer.minBindingSize = sizeof(SimulationParameters);

    wgpu::BindGroupLayoutDescriptor bind_group_layout_descriptor{};
    bind_group_layout_descriptor.entryCount = 1;
    bind_group_layout_descriptor.entries = &uniform_layout_entry;

    const wgpu::BindGroupLayout bind_group_layout =
            device.CreateBindGroupLayout(&bind_group_layout_descriptor);

    wgpu::PipelineLayoutDescriptor pipeline_layout_descriptor{};
    pipeline_layout_descriptor.bindGroupLayoutCount = 1;
    pipeline_layout_descriptor.bindGroupLayouts = &bind_group_layout;

    const wgpu::PipelineLayout pipeline_layout =
            device.CreatePipelineLayout(&pipeline_layout_descriptor);

    wgpu::ShaderSourceWGSL wgsl{};
    wgsl.code = Shaders::vortex_wgsl;

    wgpu::ShaderModuleDescriptor module_descriptor{};
    module_descriptor.nextInChain = &wgsl;

    const wgpu::ShaderModule module = device.CreateShaderModule(&module_descriptor);

    wgpu::ColorTargetState colour_target{};
    colour_target.format = texture_format;

    wgpu::FragmentState fragment{};
    fragment.module = module;
    fragment.entryPoint = "fs_main";
    fragment.targetCount = 1;
    fragment.targets = &colour_target;

    wgpu::RenderPipelineDescriptor pipeline_descriptor{};
    pipeline_descriptor.layout = pipeline_layout;

    pipeline_descriptor.vertex.module = module;
    pipeline_descriptor.vertex.entryPoint = "vs_main";

    pipeline_descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;

    pipeline_descriptor.fragment = &fragment;

    pipeline = device.CreateRenderPipeline(&pipeline_descriptor);

    wgpu::BindGroupEntry bind_group_entry{};
    bind_group_entry.binding = 0;
    bind_group_entry.buffer = uniform_buffer;
    bind_group_entry.offset = 0;
    bind_group_entry.size = sizeof(SimulationParameters);

    wgpu::BindGroupDescriptor bind_group_descriptor{};
    bind_group_descriptor.layout = bind_group_layout;
    bind_group_descriptor.entryCount = 1;
    bind_group_descriptor.entries = &bind_group_entry;

    bind_group = device.CreateBindGroup(&bind_group_descriptor);
}

} // namespace WebCFD
