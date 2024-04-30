/*
**
**                      -+- rx -+-
**
**                 Mundane Graphics API.
**
**  *--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*
**  Copyright(C) Dayan Rodriguez, 2022, All Rights Reserved
**  *--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*
**
**        NOT PRODUCTION READY/NOT PRODUCTION QUALITY
**
**
**               HONI SOIT QUI MAL Y PENSE
**
**
**                 github.com/MicroRJ/rx
**
*/

//
// Shared Structures:
//
cbuffer _ : register(b0) {
	float4x4  transform;
	float2    xyscreen;
	float2    xysource;
	float2    xycursor;

	double    total_seconds;
	double    delta_seconds;
};

SamplerState      sampler0: register(s0);
Texture2D<float4> texture0: register(t0);

struct VS_INPUT {
	float4  xyzw:POSITION;
	float4 color:COLOR0;
	float2    uv:TEXCOORD0;
};
struct PS_INPUT {
	float4  xyzw:SV_POSITION;
	float4 color:COLOR0;
	float2    uv:TEXCOORD0;
};

PS_INPUT MainVS(VS_INPUT i) {
	PS_INPUT o;
	o.    xyzw=mul(transform,i.xyzw);
	o.   color=i.color;
	o.      uv=i.uv;
	return o;
}

float4 MainPS(PS_INPUT i): SV_Target {
	return i.color * texture0.Sample(sampler0, i.uv);
}

struct PS_OUTPUT {
	float4 color:      SV_TARGET0;
	float4 color_mask: SV_TARGET1;
};

PS_OUTPUT MainPS_Text(PS_INPUT i) {
	const float samplingBias = 1.;
	float3 triSample =
	float3(
	texture0.Sample(sampler0, i.uv + float2(-samplingBias, 0) / xysource).r,
	texture0.Sample(sampler0, i.uv).r,
	texture0.Sample(sampler0, i.uv + float2(+samplingBias, 0) / xysource).r );

	PS_OUTPUT o;
	o.color      = i.color;
	o.color_mask = float4(triSample*i.color.aaa,1.);
	return o;
}

float4 MainPS_TextSDF(PS_INPUT i): SV_TARGET0 {
	float onEdge = -.5 + texture0.Sample(sampler0,i.uv).r;
	float range = length(float2( ddx(onEdge), ddy(onEdge) ));
	float alpha = smoothstep(-range, range, onEdge);
	return float4(lerp(i.color.rgb,i.color.rgb,.05), i.color.a*alpha);
}

/* these are the extended versions for SDF shape rendering */
struct VS_INPUT_SDF {
	float2 xy: POSITION0;
	float4 shape_xyxy: POSITION1;
	float4 shape_rgba: COLOR0;
  	float4 shape_flag: TEXCOORD0;
};

struct PS_INPUT_SDF {
	float4 xyzw: SV_POSITION;
	float4 shape_xyxy: POSITION1;
	float4 shape_rgba: COLOR0;
	float4 shape_flag: TEXCOORD0;
};

PS_INPUT_SDF MainVS_SDF(VS_INPUT_SDF i) {
	PS_INPUT_SDF o;
	o.xyzw     =mul(transform,float4(i.xy,.0,1.));
	o.shape_rgba=i.shape_rgba;
	o.shape_xyxy=i.shape_xyxy;
	o.shape_flag=i.shape_flag;
	return o;
}

float box_sdf(float2 distance_from_center, float2 dimensions, float r) {
	float2 d = abs(distance_from_center) - (dimensions - r);
	return length(max(d,.0)) + min(max(d.x,d.y),.0) - r;
}

float circle_sdf(float2 p, float r) {
	return length(p) - r;
}

float4 MainPS_CircleSDF(PS_INPUT_SDF i) : SV_Target {
	float2 xy = float2(i.xyzw.x,xyscreen.y - i.xyzw.y);
	return float4(i.shape_rgba.rgb,
	i.shape_rgba.a * smoothstep(0.,i.shape_flag.w,(1-circle_sdf(xy-i.shape_xyxy.xy,i.shape_xyxy.z))));
}

float4 MainPS_BoxSDF(PS_INPUT_SDF i) : SV_Target {
	float2 xy = float2(i.xyzw.x,xyscreen.y - i.xyzw.y);
	return float4(i.shape_rgba.rgb,
	i.shape_rgba.a * smoothstep(0.,i.shape_flag.w,(1-box_sdf(xy-i.shape_xyxy.xy,i.shape_xyxy.zw,i.shape_flag.x))));
}





#if 0
float4 MainPS_Text(PS_INPUT i) : SV_Target {
	const float smoothing = 1./8.;
	const float samplingBias = 1.;
	const float contrasting  = 1.;
	const float delineation  = .45;

	float3 trilinearSample =
	float3(
	texture0.Sample(sampler0, i.uv + float2(-samplingBias, 0) / xysource).r,
	texture0.Sample(sampler0, i.uv).r,
	texture0.Sample(sampler0, i.uv + float2(+samplingBias, 0) / xysource).r );

	float strength = contrasting -
	saturate( length(ddx(trilinearSample.g)) +
	length(ddy(trilinearSample.g)) );

	float3 strengthened = strength * smoothstep(.5-smoothing,.5+smoothing,trilinearSample);

	float3 delineated = lerp(strengthened, strengthened * trilinearSample, delineation);

	return i.color * delineated.rgbg;
}

const float smoothing    = 1./16.;
const float contrasting  = 1.5;
/* #important should be smaller for smaller fonts, otherwise they'll look chewed up */
const float delineation  = .95;

  /* #todo this probably overkill, the point of this is to sort of bevel or enunciate the
   edges of the font in subtle yet elegant border-like effect, it looks almost shadowy,
   this would work in favor of fonts with fine details, sharp curves, thin edges, etc ..
   promote contrast and help blending */
   trilinearSample =
   float3(
   texture0.Sample(sampler0, i.uv + float2(-samplingBias, 0) / xysource).r,
   texture0.Sample(sampler0, i.uv).r,
   texture0.Sample(sampler0, i.uv + float2(+samplingBias, 0) / xysource).r );

   float contrastFactor = contrasting;
   // saturate( length(ddx(trilinearSample.g)) +
   //           length(ddy(trilinearSample.g)) );

   float4 color = i.color;

   float smoothFactor = smoothstep(.5-smoothing,.5+smoothing,trilinearSample.g);
   color.rgb = lerp(color.rgb, color.rgb * trilinearSample, delineation);
   color.a *= (contrastFactor * smoothFactor);
   return color;
   #endif