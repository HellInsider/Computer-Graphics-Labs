struct VS_INPUT
{
  float3 InPosition : POSITION;
  float2 InTexCoord : TEXCOORD0;
  float3 InNormal : NORMAL;
  float4 InColor : COLOR;
};

struct PS_INPUT
{
  float4 Pos : SV_POSITION;
  float2 OutTexCoord : TEXCOORD0;
};

PS_INPUT main(VS_INPUT input)
{
  PS_INPUT output = (PS_INPUT)0;
  float4 inP = float4(input.InPosition.x, input.InPosition.y, input.InPosition.z, 1);
  output.Pos = inP;
  output.OutTexCoord = input.InTexCoord;

  return output;
}
