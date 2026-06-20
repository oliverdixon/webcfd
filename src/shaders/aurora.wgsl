struct Params {
    viewport: vec4<f32>,
    controls: vec4<f32>,
    colour_a: vec3<f32>,
    colour_b: vec3<f32>,
    colour_c: vec3<f32>,
    colour_d: vec3<f32>,
};

@group(0) @binding(0)
var<uniform> params: Params;

struct VsOut {
    @builtin(position) position: vec4<f32>,
    @location(0) uv: vec2<f32>,
};

@vertex
fn vs_main(@builtin(vertex_index) vertex_index: u32) -> VsOut {
    let positions = array<vec2<f32>, 3>(
        vec2<f32>(-1.0, -1.0),
        vec2<f32>( 3.0, -1.0),
        vec2<f32>(-1.0,  3.0),
    );

    let p = positions[vertex_index];

    var out: VsOut;
    out.position = vec4<f32>(p, 0.0, 1.0);
    out.uv = p * 0.5 + vec2<f32>(0.5);
    return out;
}

fn rotate(a: f32) -> mat2x2<f32> {
    let angle_sine = sin(a);
    let angle_cosine = cos(a);

    return mat2x2<f32>(
        vec2<f32>(angle_cosine, angle_sine),
        vec2<f32>(-angle_sine, angle_cosine),
    );
}

fn palette(t: f32) -> vec3<f32> {
    return params.colour_a +
           params.colour_b *
           cos(6.28318530718 * (params.colour_c * t + params.colour_d));
}

fn soft_field(p: vec2<f32>, t: f32) -> f32 {
    var v = 0.0;

    v += 0.500 * sin(p.x * 1.30 + p.y * 0.55 + t * 0.33);
    v += 0.250 * sin(p.x * 2.10 - p.y * 0.80 - t * 0.27);
    v += 0.125 * sin(p.x * 3.70 + p.y * 1.40 + t * 0.19);
    v += 0.063 * sin(p.x * 7.10 - p.y * 2.20 - t * 0.13);

    return v;
}

fn spectral_colour(t: f32) -> vec3<f32> {
    let user_colour = palette(t);

    /*
     * Bias the palette toward aurora-like cyan/green/purple. The user's palette still matters, but this avoids grey
     * mush.
     */
    let aurora_bias = vec3<f32>(
        0.35 + 0.30 * sin(6.2831853 * (t + 0.10)),
        0.75 + 0.20 * sin(6.2831853 * (t + 0.35)),
        0.85 + 0.25 * sin(6.2831853 * (t + 0.62))
    );

    return user_colour * aurora_bias;
}

fn ribbon(
    p: vec2<f32>,
    t: f32,
    index: f32,
    y_offset: f32,
    thickness: f32,
    strength: f32,
    warp: f32
) -> vec3<f32> {
    let x = p.x;

    let path =
        y_offset +
        0.18 * sin(x * (1.10 + index * 0.11) + t * (0.24 + index * 0.04)) +
        0.09 * sin(x * (2.35 + index * 0.17) - t * (0.17 + index * 0.03)) +
        0.06 * soft_field(vec2<f32>(x * 0.9, index * 1.7), t) * warp;

    let dy = p.y - path;

    // Bright central ribbon.
    let core = exp(-pow(dy / thickness, 2.0));

    // Soft vertical curtain glow extending mostly upward.
    let upward = max(dy, 0.0);
    let curtain = exp(-upward * (1.15 + index * 0.08)) *
                  smoothstep(-0.12, 0.50, dy);

    // Smooth, broad folds. Not hard bars.
    let fold =
        0.74 +
        0.26 * sin(
            x * (3.6 + index * 0.8) +
            1.8 * sin(x * 1.1 - t * 0.25) +
            t * (0.35 + index * 0.05)
        );

    // Fine shimmer, deliberately weak.
    let shimmer =
        0.92 +
        0.08 * sin(
            x * (13.0 + index * 2.0) +
            p.y * 2.5 -
            t * (0.9 + index * 0.08)
        );

    let body = core * 1.15 + curtain * 0.32;
    let alpha = body * fold * shimmer;

    let fade_bottom = smoothstep(-1.05, path + 0.03, p.y);
    let fade_top = smoothstep(1.35, 0.15, p.y);

    let c = spectral_colour(
        0.10 * index +
        0.08 * sin(x * 0.7 + t * 0.11) +
        0.04 * p.y +
        t * 0.015
    );

    return c * alpha * fade_bottom * fade_top * strength;
}

fn star_field(p: vec2<f32>, t: f32) -> vec3<f32> {
    // Continuous pseudo-stars using trig, not grid hash. Subtle enough not to distract.
    let s1 = sin(p.x * 91.7 + sin(p.y * 37.1) * 12.0);
    let s2 = sin(p.y * 83.3 + sin(p.x * 29.4) * 10.0);
    let s = s1 * s2;

    let stars = smoothstep(0.995, 1.0, s);
    let twinkle = 0.75 + 0.25 * sin(t * 3.0 + p.x * 11.0);

    return vec3<f32>(0.55, 0.70, 1.0) * stars * twinkle * 0.18;
}

@fragment
fn fs_main(in: VsOut) -> @location(0) vec4<f32> {
    var uv = in.uv * 2.0 - vec2<f32>(1.0);
    uv.x *= params.viewport.y;

    let t = params.viewport.x * clamp(params.controls.x, 0.0, 4.0);
    let intensity = clamp(params.controls.y, 0.0, 4.0);
    let warp = clamp(params.controls.z, 0.0, 3.0);
    let scale = clamp(params.controls.w, 0.25, 4.0);

    // Slight camera tilt makes the curtain feel less flat.
    var p = uv * scale;
    p = rotate(0.035 * sin(t * 0.13)) * p;

    // Smooth night sky.
    let night_sky = smoothstep(-1.0, 1.0, uv.y);

    var colour = mix(
        vec3<f32>(0.002, 0.004, 0.015),
        vec3<f32>(0.025, 0.035, 0.090),
        night_sky
    );

    // Very soft large-scale atmospheric glow.
    let atmospheric =
        0.5 +
        0.5 * soft_field(vec2<f32>(p.x * 0.55, p.y * 0.45), t * 0.35);

    colour += vec3<f32>(0.02, 0.05, 0.08) *
              atmospheric *
              smoothstep(-0.6, 0.9, p.y) *
              0.20;

    colour += star_field(p, t);

    // Layered aurora curtains.
    colour += ribbon(p, t, 0.0,  0.10, 0.070, 0.95, warp);
    colour += ribbon(p, t, 1.0, -0.08, 0.055, 0.78, warp);
    colour += ribbon(p, t, 2.0,  0.30, 0.090, 0.55, warp);
    colour += ribbon(p, t, 3.0, -0.30, 0.075, 0.42, warp);

    // Gentle bloom-like wash, not enough to turn it into fog.
    let centre_glow = exp(-1.8 * length(vec2<f32>(uv.x * 0.75, uv.y + 0.05)));
    colour += spectral_colour(t * 0.02) * centre_glow * 0.045;

    // Vignette, tone map, gamma.
    let vignette = smoothstep(1.55, 0.18, length(uv));
    colour *= vignette * intensity;

    colour = colour / (colour + vec3<f32>(1.0));
    colour = pow(colour, vec3<f32>(0.454545));

    return vec4<f32>(colour, 1.0);
}
