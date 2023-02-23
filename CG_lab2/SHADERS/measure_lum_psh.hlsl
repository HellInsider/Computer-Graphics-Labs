struct PS_INPUT
{
  float4 Pos : SV_POSITION;
  float2 OutTexCoord : TEXCOORD0;
};

Texture2D Texture0 : register(t2);

cbuffer ScreenSpaceConstBuf : register(b3)
{
  float sxInv; // 1.0 / sx
  float syInv;
  uint sx;
  uint sy;
};

float LogLuminance(float4 col)
{
  float lum = 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b;
  return log(lum + 1.0);
}

float4 main(PS_INPUT input) : SV_Target
{
  float4 rez;
  float lum = 0;

#if defined(LUM_STAGE_EVAL_LOGLUM)
  float4 src = Texture0.Load(int3(input.Pos.xy, 0));
  lum = LogLuminance(src);
  rez.x = lum;
#elif defined(LUM_STAGE_DOWNSAMPLE)
  int3 tcoord;
  tcoord.x = (input.Pos.x - 0.5) * 4;
  tcoord.y = (input.Pos.y - 0.5) * 4;
  tcoord.z = 0;

  for (int dx = 0; dx < 4; dx++)
    for (int dy = 0; dy < 4; dy++)
    {
      int3 tc = tcoord;
      tc.x += dx;
      tc.y += dy;
      lum += Texture0.Load(tc).x;
    }

  lum /= 16.0;
  rez.x = lum;
#elif defined(LUM_STAGE_FINAL)
  int3 tcoord;
  tcoord.x = 0;
  tcoord.y = 0;
  tcoord.z = 0;
  //return 2.2;
  for (int dx = 0; dx < sx; dx++)
    for (int dy = 0; dy < sy; dy++)
    {
      int3 tc = tcoord;
      tc.x += dx;
      tc.y += dy;
      lum += Texture0.Load(tc).x;
    }

  lum /= float(sx * sy);
  lum = exp(lum) - 1.0;
  float key = 1.03 - (2.0 / (2.0 + log10(lum + 1.0)));
  float expo = key / lum * 3.0;
  rez.x = expo;
#endif
  return rez;
}
