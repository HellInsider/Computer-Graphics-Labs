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


cbuffer CommonConstBuf : register(b0)
{
  matrix MatrView;
  matrix MatrProj;
  float3 CamLoc;
  float Time;
};

cbuffer ConstBuffer : register(b2)
{
  /* Illumination coefficients (anbient, diffuse, specular) */
  float3 Ka;
  float Ph;
  float3 Kd;
  float Trans;
  float3 Ks;

  int IsTex0;
  int IsTex1;
};

float sqr(float x)
{
  return x * x;
}

float3 LookUp(float angle)
{
  float PI = 3.1415926;

  /*float3 table[] = {
    float3(0.0, 0.0, 0.0),
    float3(0.9, 0.0, 0.0),
    float3(0.8, 0.4, 0.0),
    float3(0.7, 0.8, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.5, 0.8),
    float3(0.0, 0.0, 1.0),
    float3(0.5, 0.0, 0.5),
    float3(0.0, 0.0, 0.0)
  };*/
  float3 tableSRC[] = {
  float3(0.0, 0.0, 0.0),
  float3(0.011, 0.0, 0.0),
  float3(0.2, 0.0, 0.0),
  float3(0.4, 0.0, 0.0),
  float3(0.8, 0.4, 0.0),
  float3(0.6, 0.8, 0.0),
  float3(0.1, 0.5, 0.3),
  float3(0.0, 0.4, 0.5),
  float3(0.0, 0.2, 0.8),
  float3(0.2, 0.0, 0.3),
  float3(0.0, 0.0, 0.0)
  };
  float3 table[] = {
  float3(0.0, 0.0, 0.0),
  float3(0.0, 0.0, 0.0),
  float3(0.0, 0.0, 0.0),
  //tableSRC[7] + tableSRC[6],
  float3(0.2, 0.5, 0.6),
  //tableSRC[3] + tableSRC[6],
  float3(0.5, 0.9, 0.1),
  //tableSRC[3] + tableSRC[9],
  float3(0.7, 0.2, 0.2),
  float3(0.0, 0.0, 0.0)
  };
  //angle -= PI / 16;
  //angle /= PI / 8;
  if (angle > 0.995)
    return pow(angle, 300).xxx;
  if (angle <= 0.1 || angle >= 1.0)
    return float3(0.0, 0.0, 0.0);
  angle -= 0.2;
  angle = angle / 0.9 * 7.0;
 // angle *= 10; //table size
  uint idx = uint(angle);
  float frac = angle - float(idx);
  //frac = 0.0;
  if (idx == 0)
    return table[idx] * frac;
  return table[idx] * frac + table[idx - 1] * (1.0 - frac);

}

float4 Shade2(float3 P, float3 N, float2 T)
{
  float PI = 3.1415926;
  float Threshold = 0.00005;
  // Resut color
  float3 color = float3(0, 0, 0);
  // materials
  float3 albedo = Kd;
  if (IsTex0 == 1)
    albedo = (Texture0.Sample(Sampler0, T)) * 1.2 + 0.05 * albedo;

  float3 rTemp = float3(1.0, 1.0, 1.0) - Ks;
  float roughness = min((rTemp.x + rTemp.y + rTemp.z) / 2.0, 0.99);
  float metallic = min((Ph + 20) / 100.0, 0.99);

  float3 V = normalize(P - CamLoc);
  V = -V;
//  float3 LightPos = float3(10, 50, 50);
  float3 LightPos = float3(5, 8, -3);
  float3 LightColor = float(1).xxx;// float3(0.8, 1, 0.9);
  float LightDist = length(LightPos - P);
//  float3 Ls[] = { normalize(LightPos - P), normalize(float3(50, 50, 10) - P), normalize(float3(-50, 50, -50) - P) };//normalize(float3(-40, 50, -50)) };
  float3 Ls[] = { normalize(LightPos - P), normalize(float3(1, 3, 5) - P), normalize(float3(-50, 50, -50) - P) };//normalize(float3(-40, 50, -50)) };
  float3 L = normalize(LightPos - P);
  float3 R = normalize(reflect(V, N));

  float coef;

  // Ambient
  color += Ka;

  float nv = max(dot(V, N), 0.0);
  coef = 1.0 - pow((1.0 - nv), 17);
  coef = pow(coef, 17);
  float coef2 = coef;
  float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
  float3 FresnelSchlick = F0 + ((float3(1.0, 1.0, 1.0) - F0) * pow(1.0 - nv, 5.0));
  // for each light
  for (int i = 0; i < 2; i++)
  {
    L = Ls[i];
    float nl = max(dot(N, L), 0.0);
    float rl = max(dot(L, R), 0.0);
    // todo attenuate
    float3 Halfway = normalize(L + V);

    //return F0;
    //return (Texture0.Sample(Sampler0, T));
    float DistributionGGX = sqr(roughness) / (PI * sqr(sqr(max(dot(N, Halfway), 0.0)) * (sqr(roughness) - 1.0) + 1.0));
    float K = sqr(roughness + 1) / 8.0;
    float GeomObstructionGGX = nv / (nv * (1 - K) + K);
    float GeomSelfshadingGGX = nl / (nl * (1 - K) + K);
    float GeomFunc = GeomObstructionGGX * GeomSelfshadingGGX;

    float3 Specular = FresnelSchlick * (DistributionGGX * GeomFunc / (4.0 * (nv + Threshold)));

    float3 KDiffuse = float3(1.0, 1.0, 1.0) - FresnelSchlick;
    float3 Diffuse = albedo * (1.0 - metallic) * KDiffuse * (1.0 / PI);

    color += (Diffuse + Specular) * nl / 3.0;

    // transp
    float tr = min(Trans * coef * 2.0, 1.0);
    //   return float4(tr.xxx, 1.0);
    float hn = max(dot(Halfway, N), 0.0);
    float3 inter = LookUp(hn);
    if (inter.x > 0.01 || inter.y > 0.01 || inter.z > 0.01)
    {
      float mean = (inter.x + inter.y + inter.z) / 3.0;

      if (coef2 == coef)
      {
        if (hn < 0.995)
        {
          coef -= sqr(mean) / nv;
          coef = max(coef, 0.0);
          color += inter * 2.0 * pow(coef, 2);// *sqrt(1.0 - nv);// *(max(dot(Halfway, N), 0.0));
        }
        else
        {
          color += inter * 2.0;
          coef -= mean;
          coef = max(coef, 0.0);
        }
      }
      else
      {
        if (hn < 0.995)
        {
          coef2 -= sqr(mean) / nv;
          coef2 = max(coef2, 0.0);
          color += inter * 2.0 * pow(coef2, 2);// *sqrt(1.0 - nv);// (max(dot(Halfway, N), 0.0));
        }
        else
        {
          color += inter * 2.0;
          coef2 -= mean;
          coef2 = max(coef2, 0.0);
        }

      }
    }
    
     // return float4( sqr(sqr(1-coef)).xxx, 1.0);
    //return float4(0, 0, 0, 1);
    //color = inter * (coef);
  }
    coef = min(coef, coef2);
  //return float4(sqr(nv).xxx / 5.0, 1.0);
  //return float4((1.0 * (1.0 - coef)).xxx, 1.0);
 //   return float4(coef.xxx, 1.0);
  //return float4((Trans * (1.0 - coef)).xxx, 1.0);
  return float4(color, Trans * sqr(sqr(1.0 - coef)));
}
/*
float4 Shade(float3 P, float3 N, float2 T)
{
  // Resut color
  float3 color = float3(0, 0, 0);

  float3 V = normalize(P - CamLoc);

  //return N;// + float(1.0).xxx;
  // Ambient
  color += Ka;

  // Diffuse
  float3 LightPos = float3(0, 50, 4);
  float3 LightColor = float(1).xxx;// float3(0.8, 1, 0.9);
  float3 L = normalize(LightPos - P);

  float3 Kds = Kd;
  if (IsTex0 == 1)
    Kds = Texture0.Sample(Sampler0, T) + 0.2 * Kd;
  float nl = dot(L, N);
  color += Kds * LightColor * max(nl, 0);

  // Specular
  float3 R = normalize(reflect(V, N));
  float rl = max(dot(L, R), 0.0);

  color += Ks * pow(max(rl, 0), Ph + 30);

  color += Ka * 0.5;


  float nv = max(dot(-V, N), 0);
//  return float4(nv.xxx, 1.0);
  float coef = 1.0 - (1.0 - nv) * (1.0 - nv)* (1.0 - nv)* (1.0 - nv)* (1.0 - nv);
  coef = coef * coef * coef * coef * coef;
  float tr = min(Trans * coef * 2.0, 1.0);
 //   return float4(tr.xxx, 1.0);

  float3 inter = LookUp(sqrt(rl * nv));
  if (inter.x > 0.01 || inter.y > 0.01 || inter.z > 0.01)
  {
    float mean = (inter.x + inter.y + inter.z) / 3.0;
    coef *= 1.0 - 2.0 * mean;
  }
  color += inter;
  return float4(color, Trans * (1.0 - coef));
}*/


float4 main(PS_INPUT input) : SV_Target
{
  float4 sh = Shade2(input.OutWorldPoc, normalize(input.OutNormal), input.OutTexCoord);
  return sh;
}