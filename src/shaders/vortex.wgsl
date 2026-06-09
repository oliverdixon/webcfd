struct Params {
    viewport: vec4<f32>,
    controls: vec4<f32>,
    colour_a: vec4<f32>,
    colour_b: vec4<f32>,
    colour_c: vec4<f32>,
    colour_d: vec4<f32>,
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

fn rot(a: f32) -> mat2x2<f32> {
    let s = sin(a);
    let c = cos(a);

    return mat2x2<f32>(
        vec2<f32>( c, s),
        vec2<f32>(-s, c),
    );
}

fn palette(t: f32) -> vec3<f32> {
    return params.colour_a.xyz +
           params.colour_b.xyz *
           cos(6.28318530718 * (params.colour_c.xyz * t + params.colour_d.xyz));
}

fn hash21(p: vec2<f32>) -> f32 {
    let q = fract(vec2<f32>(
        dot(p, vec2<f32>(127.1, 311.7)),
        dot(p, vec2<f32>(269.5, 183.3))
    ));

    return fract(sin(q.x + q.y) * 43758.5453123);
}

fn star(p: vec2<f32>, flare: f32) -> f32 {
    let d = max(length(p), 0.0001);

    var m = 0.018 / d;

    let rays_a = max(0.0, 1.0 - abs(p.x * p.y * 950.0));
    let rp = rot(0.78539816339) * p;
    let rays_b = max(0.0, 1.0 - abs(rp.x * rp.y * 950.0));

    m += flare * rays_a * 0.85;
    m += flare * rays_b * 0.35;

    return m;
}

fn grid_stars(p: vec2<f32>, t: f32) -> vec3<f32> {
    var colour = vec3<f32>(0.0);

    for (var layer: i32 = 0; layer < 3; layer = layer + 1) {
        let fl = f32(layer);
        let scale = 5.0 + fl * 6.0;

        var q = p * scale;
        q.y += t * (0.28 + fl * 0.11);

        let cell = floor(q);
        let local = fract(q) - vec2<f32>(0.5);

        let h = hash21(cell + fl * 19.17);

        if (h > 0.82) {
            let pos = local + vec2<f32>(
                sin(h * 41.0 + t * 0.7),
                cos(h * 53.0 + t * 0.6)
            ) * 0.18;

            let twinkle = 0.65 + 0.35 * sin(t * 5.0 + h * 80.0);
            let s = star(pos, 0.025 + 0.02 * fl) * twinkle;

            colour += palette(h + fl * 0.17 + t * 0.03) * s * 0.18;
        }
    }

    return colour;
}

@fragment
fn fs_main(in: VsOut) -> @location(0) vec4<f32> {
    var uv = in.uv * 2.0 - vec2<f32>(1.0);
    uv.x *= params.viewport.y;

    let t = params.viewport.x * params.controls.x;

    let intensity = params.controls.y;
    let warp = params.controls.z;
    let scale = max(params.controls.w, 0.1);

    var p = uv * scale;

    let slow_spin = 0.25 * sin(t * 0.27);
    p = rot(slow_spin) * p;

    var colour = vec3<f32>(0.0);

    /*
       Deep background starfield.
    */
    colour += grid_stars(p, t);

    /*
       Iterated inversion fractal.
    */
    var q = p;

    for (var i: i32 = 0; i < 9; i = i + 1) {
        let fi = f32(i);

        q = abs(q);

        let inv = clamp(dot(q, q), 0.16, 1.05);
        q = q / inv;

        q -= vec2<f32>(
            0.73 + 0.11 * sin(t * 0.31 + fi * 1.21 * warp),
            0.58 + 0.10 * cos(t * 0.25 + fi * 1.73 * warp)
        );

        q = rot(0.42 + 0.10 * sin(t * 0.37 + fi * warp)) * q;

        let d = abs(length(q) - 0.68);

        let core = exp(-15.0 * d);
        let haze = exp(-3.0 * d);

        let bands = 0.5 + 0.5 * sin(22.0 * d - t * 3.6 + fi * 0.9);
        let c = palette(0.10 * fi + 0.11 * t + bands * 0.18);

        colour += c * core * 0.105;
        colour += c * haze * 0.018;
    }

    /*
       Polar tunnel shockwaves.
    */
    let r = max(length(p), 0.001);
    let a = atan2(p.y, p.x);

    let tunnel_a = sin(20.0 / r + 10.0 * a + t * 4.6);
    let tunnel_b = sin(8.0 / r - 6.0 * a - t * 2.7);
    let tunnel = tunnel_a * tunnel_b;

    let tunnel_mask = smoothstep(1.15, 0.05, r);
    colour += palette(tunnel * 0.09 + t * 0.045) * tunnel_mask * 0.18 * warp;

    /*
       Animated event-horizon ring.
    */
    let ring_radius = 0.42 + 0.035 * sin(t * 0.9);
    let ring_distance = abs(r - ring_radius);
    let ring = exp(-70.0 * ring_distance);

    let ring_colour = palette(a * 0.08 + t * 0.12);
    colour += ring_colour * ring * 0.38;

    /*
       Central lens/starburst.
    */
    let flare_p = rot(t * 0.45) * p;
    let flare = star(flare_p, 0.58);
    colour += palette(t * 0.04 + 0.18) * flare * 0.9;

    /*
       Subtle scanline shimmer.
    */
    let scanline = 0.96 + 0.04 * sin((in.uv.y * params.viewport.w + t * 80.0) * 0.7);
    colour *= scanline;

    /*
       Vignette, tone map, gamma.
    */
    let vignette = smoothstep(1.45, 0.18, length(uv));
    colour *= vignette * intensity;

    colour = colour / (colour + vec3<f32>(1.0));
    colour = pow(colour, vec3<f32>(0.454545));

    return vec4<f32>(colour, 1.0);
}
