struct PS_INPUT
{
  float4 Pos : SV_POSITION;
  float2 OutTexCoord : TEXCOORD0;
};

Texture2D Texture0 : register(t2);
Texture2D Exposure : register(t3);

cbuffer ScreenSpaceConstBuf : register(b3)
{
  float sxInv; // 1.0 / sx
  float syInv;
  uint sx;
  uint sy;
};


float3 Uncharted2Tonemap(float3 x)
{
  const float A = 0.1; // Shoulder Strength
  const float B = 0.50; // Linear Strength
  const float C = 0.1; // Linear Angle
  const float D = 0.20; // Toe Strength
  const float E = 0.02; // Toe Numerator
  const float F = 0.30; // Toe Denominator
  // Note: E/F = Toe Angle
  return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float4 main(PS_INPUT input) : SV_Target
{
  const float WH = 11.2; // Linear White Point Value
  float4 rez;

  float4 src = Texture0.Load(int3(input.Pos.xy, 0));
  float exp = Exposure.Load(int3(0, 0, 0)) * 7.0;
  float3 cur = Uncharted2Tonemap(src * exp);
  float3 whiteScale = float3(1, 1, 1) / Uncharted2Tonemap(WH);
  rez = float4(pow(cur * whiteScale, 1.0 / 2.2), 1.0);

  return rez;
}
