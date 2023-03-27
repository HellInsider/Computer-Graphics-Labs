struct PS_INPUT
{
  float4 Pos : SV_POSITION;
  float4 OutColor : COLOR;
  float3 OutWorldPoc : POSITION;
  float3 OutNormal: NORMAL;
  float2 OutTexCoord : TEXCOORD0;
};

TextureCube Texture0 : register(t0);
SamplerState Sampler0 : register(s0);

Texture2D Texture1 : register(t1);
SamplerState Sampler1: register(s1);


cbuffer CommonConstBuf : register(b0)
{
  matrix MatrView;
  matrix MatrProj;
  float3 CamLoc;
  float Time;
};

cbuffer ConstBuffer : register(b2)
{
  /* pbr material */
  float3 Albedo;
  float Roughness;
  float Metalness;
  float Trans;

  int IsTex0;
  int IsTex1;
};

struct raw_light
{
  float4 Color;
  float3 PosDir;  // pos or dir depend on type
  int Type;
  float3 Atten;
  int __padding;
};

const uint MAX_LIGHTS = 16;

cbuffer LightConstBuf : register(b4)
{
  uint NumLighs;
  uint LightingMode;
  uint2 __padding;
  raw_light RawLights[16];
};


float sqr(float x)
{
  return x * x;
}

float3 Shade2(float3 P, float3 N, float2 T)
{
  float PI = 3.1415926;
  float Threshold = 0.00005;
  // Resut color
  float3 color = float3(0, 0, 0);
  // materials
  float3 albedo = Albedo;
  float3 dir = normalize(P - CamLoc);
  if (IsTex0 == 1)
    albedo = (Texture0.Sample(Sampler0, dir));
  return albedo;
}


float4 main(PS_INPUT input) : SV_Target
{
  return float4(Shade2(input.OutWorldPoc, normalize(input.OutNormal), input.OutTexCoord), Trans);
}