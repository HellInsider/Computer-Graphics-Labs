struct PS_INPUT
{
  float4 Pos : SV_POSITION;
  float4 OutColor : COLOR;
  float3 OutWorldPoc : POSITION;
  float3 OutNormal: NORMAL;
  float2 OutTexCoord : TEXCOORD0;
};

Texture2D Texture0 : register(t0);
SamplerState Sampler0 : register(s0);

Texture2D Texture1 : register(t1);
SamplerState Sampler1: register(s1);

TextureCube IrradianceTexture : register(t6);

cbuffer CommonConstBuf : register(b0)
{
  matrix MatrView;
  matrix MatrProj;
  float3 CamLoc;
  float Time;
  int IsEnviromentTexture;
  int IsIrradianceTexture;
  int _pad0;
  int _pad1;
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

float4 AttenuatedLight(int LightIndex, float Dist)
{
  float3 a = RawLights[LightIndex].Atten;
  return RawLights[LightIndex].Color * (1.0 / (a.x + a.y * Dist + a.z * Dist * Dist));
}

float3 Shade2(float3 P, float3 N, float2 T)
{
  float PI = 3.1415926;
  float Threshold = 0.00005;
  // Resut color
  float3 color = float3(0, 0, 0);
  // materials
  float3 albedo = Albedo;
  if (IsTex0 == 1)
    albedo = (Texture0.Sample(Sampler0, T)) * 1.2 + 0.05 * albedo;

  float roughness = Roughness;
  float metallic = Metalness;

  float3 V = normalize(P - CamLoc);
  V = -V;

  float3 R = normalize(reflect(V, N));

  float nv = max(dot(V, N), 0.0);

  float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);

  // for each light
  for (int i = 0; i < NumLighs; i++)
  {
    // type == 0 - point, type == 1 - direction
    float3 L = normalize(RawLights[i].Type * RawLights[i].PosDir + (1 - RawLights[i].Type) * (RawLights[i].PosDir - P));
    float lDist = length(RawLights[i].PosDir - P) * (1 - RawLights[i].Type);
    float3 atten = AttenuatedLight(i, lDist).rgb;
    atten = step(0.05, atten.r + atten.g + atten.b) * atten;

    float3 Halfway = normalize(L + V);
    float nl = dot(N, L);
    float isNL = step(0.0, nl);
    nl = nl * isNL;
    float hv = max(dot(Halfway, V), 0.0);

    float3 FresnelSchlick = F0 + ((float3(1.0, 1.0, 1.0) - F0) * pow(1.0 - hv, 5.0));
    if (LightingMode == 3)
      return FresnelSchlick * isNL;

    float DistributionGGX = sqr(roughness) / (PI * sqr(sqr(max(dot(N, Halfway), 0.0)) * (sqr(roughness) - 1.0) + 1.0));
    if (LightingMode == 1)
      return DistributionGGX.xxx;

    float K = sqr(roughness + 1) / 8.0;
    float GeomObstructionGGX = nv / (nv * (1 - K) + K);
    float GeomSelfshadingGGX = nl / (nl * (1 - K) + K);
    float GeomFunc = GeomObstructionGGX * GeomSelfshadingGGX;
    if (LightingMode == 2)
      return GeomFunc.xxx;

    float3 Specular = FresnelSchlick * (DistributionGGX * GeomFunc / (4.0 * (nv + Threshold)));

    float3 KDiffuse = float3(1.0, 1.0, 1.0) - FresnelSchlick;
    float3 Diffuse = albedo * (1.0 - metallic) * KDiffuse * (1.0 / PI);

    color += (Diffuse + Specular) * nl * atten;// / 3.0;
    //color += F0 / 3.0;
  }

  // for IBL
  if (IsIrradianceTexture)
  {
    float3 F = F0 + (max((1.0 - roughness).xxx, F0) - F0) * pow(1.0 - nv, 5.0);
    float3 kS = F;
    float3 kD = 1 - kS;
    kD *= 1 - metallic;
    float3 irr = IrradianceTexture.Sample(Sampler0, N).rgb;
    float3 Diffuse = albedo * irr;

    color += kD * Diffuse;
  }
  return color;
}


float4 main(PS_INPUT input) : SV_Target
{
  return float4(Shade2(input.OutWorldPoc, normalize(input.OutNormal), input.OutTexCoord), Trans);
}
