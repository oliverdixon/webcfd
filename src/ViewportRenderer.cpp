//
// Created by owd on 04/06/2026.
//

#include "ViewportRenderer.hpp"

#include "webgpu/webgpu_glfw.h"

namespace WebCFD
{

namespace
{

constexpr char shader_code[] = R"(
    struct FragmentUniforms {
        colour : vec4f,
    };

    @group(0) @binding(0)
    var<uniform> uniforms : FragmentUniforms;

    @vertex fn vertexMain(
        @builtin(vertex_index) i : u32
    ) -> @builtin(position) vec4f {
        const pos = array(
            vec2f( 0,  1),
            vec2f(-1, -1),
            vec2f( 1, -1)
        );

        return vec4f(pos[i], 0, 1);
    }

    @fragment fn fragmentMain() -> @location(0) vec4f {
        return uniforms.colour;
    }
)";

}

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

    device.GetQueue().WriteBuffer(uniform_buffer, 0, &parameters, sizeof(SimulationParameters));

    wgpu::RenderPassColorAttachment attachment{
            .view = texture_view,
            .loadOp = wgpu::LoadOp::Clear,
            .storeOp = wgpu::StoreOp::Store,
            .clearValue = {0.02, 0.02, 0.04, 1.0}
    };

    const wgpu::RenderPassDescriptor pass_descriptor{.colorAttachmentCount = 1, .colorAttachments = &attachment};
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
    // TODO monster function needs refactoring.

    // Step 1.  Create the bind group layout for the uniform buffer.

    constexpr wgpu::BindGroupLayoutEntry uniform_layout_entry{
            .binding = 0,
            .visibility = wgpu::ShaderStage::Fragment,
            .buffer = {.type = wgpu::BufferBindingType::Uniform, .minBindingSize = sizeof(SimulationParameters)}
    };

    const wgpu::BindGroupLayoutDescriptor bind_group_layout_descriptor{
            .entryCount = 1,
            .entries = &uniform_layout_entry
    };

    wgpu::BindGroupLayout bind_group_layout = device.CreateBindGroupLayout(&bind_group_layout_descriptor);

    // Step 2.  Use the bind group layout to describe the render pipeline, and create the pipeline.

    const wgpu::PipelineLayoutDescriptor pipeline_layout_descriptor{
            .bindGroupLayoutCount = 1,
            .bindGroupLayouts = &bind_group_layout
    };

    wgpu::PipelineLayout pipeline_layout = device.CreatePipelineLayout(&pipeline_layout_descriptor);

    wgpu::ShaderSourceWGSL wgsl{{.code = shader_code}};
    const wgpu::ShaderModuleDescriptor module_descriptor{.nextInChain = &wgsl};
    const wgpu::ShaderModule module = device.CreateShaderModule(&module_descriptor);

    wgpu::ColorTargetState colour_target{.format = texture_format};
    wgpu::FragmentState fragment{.module = module, .targetCount = 1, .targets = &colour_target};

    const wgpu::RenderPipelineDescriptor pipeline_descriptor{
            .layout = pipeline_layout,
            .vertex = {.module = module},
            .fragment = &fragment
    };

    pipeline = device.CreateRenderPipeline(&pipeline_descriptor);

    // Step 3.  Create the bind group.

    wgpu::BindGroupEntry
            bind_group_entry{.binding = 0, .buffer = uniform_buffer, .offset = 0, .size = sizeof(SimulationParameters)};

    wgpu::BindGroupDescriptor bind_group_descriptor{
            .layout = bind_group_layout,
            .entryCount = 1,
            .entries = &bind_group_entry
    };

    bind_group = device.CreateBindGroup(&bind_group_descriptor);
}

} // namespace WebCFD
