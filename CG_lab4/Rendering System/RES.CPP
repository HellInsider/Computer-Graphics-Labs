#include "../Anim/anim.h"
#include "../Stb_image/stb_image.h"
#include "../Stb_image/stb_image_write.h"
#include "../Math/MTHDEF.H"
#include <math.h>


material::material(const std::string& NewName,
  const color3& NewAlbedo, const FLT& NewRoughness,
  FLT NewMetalness, FLT NewTrans,
  shader* NewShd, texture* NewTex0, texture* NewTex1) :
  Name(NewName), MaterialData({ NewAlbedo, NewRoughness, NewMetalness, NewTrans, 0, 0 }),
  Shd(NewShd)
{
  Tex[0] = NewTex0;
  Tex[1] = NewTex1;

  MaterialData.IsTex0 = Tex[0] != NULL;
  MaterialData.IsTex1 = Tex[1] != NULL;

  render* rnd = anim::GetAnim();

  D3D11_BUFFER_DESC bd;
  ZeroMemory(&bd, sizeof(bd));
  bd.Usage = D3D11_USAGE_DEFAULT;
  UINT sz = sizeof(material_constants);
  bd.ByteWidth = sz + (16 - sz % 16);
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.CPUAccessFlags = 0;
  HRESULT hr = rnd->GetDriver()->CreateBuffer(&bd, nullptr, &ConstBuffer);
  if (FAILED(hr))
  {
    MessageBox(NULL, "Failed to create material cb", "ERROR", MB_OK);
    return;
  }

  rnd->GetDriverIterface()->UpdateSubresource(ConstBuffer, 0, nullptr, &MaterialData, 0, 0);


} 

VOID material::Apply( VOID )
{
  render *rnd = anim::GetAnim();

  // Apply shader
  shader *shd = Shd;
  if (shd == NULL)
    shd = rnd->Shaders.Find("shaders\\def");
  assert(shd != NULL);
  shd->Apply();
  
  // Textures
  if (Tex[0] != NULL)
  {
    Tex[0]->Apply(TEXTURE_MATERIAL_SLOT_0);
    MaterialData.IsTex0 = TRUE;
  }
  else
  {
    texture::ApplyNull(TEXTURE_MATERIAL_SLOT_0);
    MaterialData.IsTex0 = FALSE;
  }

  if (Tex[1] != NULL)
  {
    Tex[1]->Apply(TEXTURE_MATERIAL_SLOT_1);
    MaterialData.IsTex1 = TRUE;
  }
  else
  {
    texture::ApplyNull(TEXTURE_MATERIAL_SLOT_1);
    MaterialData.IsTex1 = FALSE;
  }

  rnd->GetDriverIterface()->UpdateSubresource(ConstBuffer, 0, nullptr, &MaterialData, 0, 0);
  rnd->GetDriverIterface()->PSSetConstantBuffers(CONST_BUF_SLOT_MATERIAL, 1, &ConstBuffer);

 }

HRESULT shader::CompileShaderFromFile(const CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, const D3D_SHADER_MACRO* defines)
{
  HRESULT hr = S_OK;

  DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
  dwShaderFlags |= D3DCOMPILE_DEBUG;
  dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  ID3DBlob* pErrorBlob = nullptr;
  WCHAR fileName[300] = { 0 };
  MultiByteToWideChar(CP_UTF8, 0, szFileName, (int)strlen(szFileName), fileName, 300);
  
  hr = D3DCompileFromFile(fileName, defines, nullptr, szEntryPoint, szShaderModel,
    dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
  if (FAILED(hr))
  {
    if (pErrorBlob)
    {
      Log(std::string(szFileName), reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
      pErrorBlob->Release();
    }
    else
    {
      Log(std::string(szFileName), "Error with no error blob");
    }
    return hr;
  }
  if (pErrorBlob) pErrorBlob->Release();

  return S_OK;
}


shader::shader(const std::string& FileNamePrefix, const D3D_SHADER_MACRO* defines) : Name(FileNamePrefix)
{
  render* rnd = anim::GetAnim();
  // Compile the vertex shader
  ID3DBlob* pVSBlob = nullptr;
  std::string fullName = FileNamePrefix + PostfixVert;

  HRESULT hr = CompileShaderFromFile(fullName.c_str(), "main", "vs_5_0", &pVSBlob, defines);
  if (FAILED(hr))
  {
    MessageBox(NULL, "Failed to compile vertex shader", "ERROR", MB_OK);
    return;
  }

  // Create the vertex shader
  hr = rnd->GetDriver()->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &VertexShader);
  if (FAILED(hr))
  {
    pVSBlob->Release();
    MessageBox(NULL, "Failed to create vertex shader", "ERROR", MB_OK);
    return;
  }

  // Define the input layout
  D3D11_INPUT_ELEMENT_DESC layout[] =
  {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
      { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
  };
  UINT numElements = ARRAYSIZE(layout);

  // Create the input layout
  hr = rnd->GetDriver()->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
    pVSBlob->GetBufferSize(), &InputLayout);
  pVSBlob->Release();
  if (FAILED(hr))
  {
    MessageBox(NULL, "Failed to create input layout", "ERROR", MB_OK);
    return;
  }

  // Set the input layout
  rnd->GetDriverIterface()->IASetInputLayout(InputLayout);

  // Compile the pixel shader
  fullName = FileNamePrefix + PostfixPix;
  ID3DBlob* pPSBlob = nullptr;
  hr = CompileShaderFromFile(fullName.c_str(), "main", "ps_5_0", &pPSBlob, defines);
  if (FAILED(hr))
  {
    MessageBox(NULL, "Failed to compile pixel shader", "ERROR", MB_OK);
    return;
  }

  // Create the pixel shader
  hr = rnd->GetDriver()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &PixelShader);
  pPSBlob->Release();
  if (FAILED(hr))
    if (FAILED(hr))
    {
      MessageBox(NULL, "Failed to create pixel shader", "ERROR", MB_OK);
      return;
    }
   
} 

VOID shader::Apply(VOID)
{
  /* Enable shader */
  render* rnd = anim::GetAnim();

  rnd->GetDriverIterface()->VSSetShader(VertexShader, nullptr, 0);
  rnd->GetDriverIterface()->PSSetShader(PixelShader, nullptr, 0);
} 

texture::texture(const std::string& FileName, BOOL srgb, tex_type NewType) : Name(FileName), Type(NewType)
{
  INT n;

  if (NewType == TEX_TYPE_PLAIN)
  {
    BYTE* data = stbi_load(FileName.c_str(), &W, &H, &n, 4);
    if (data)
      InitGpuRes(W, H, 4, &data, srgb);
    else
      MessageBox(NULL, "Failed to load texture", "ERROR", MB_OK);
    stbi_image_free(data);
  }
  else if (NewType == TEX_TYPE_CUBE)
  {
    size_t p = FileName.find_last_of('\\');
    std::string format = FileName.substr(p + 2, std::string::npos);
    std::string path = FileName.substr(0, p);

    float* data[6];
    std::string cubeFaces[] = { "posx", "negx", "posy", "negy", "posz", "negz" };

    for (int i = 0; i < 6; i++)
    {
      std::string curFace = path + "\\" + cubeFaces[i] + format;
      data[i] = stbi_loadf(curFace.c_str(), &W, &H, &n, 4);
      if (!data[i])
        MessageBox(NULL, "Failed to load texture", "ERROR", MB_OK);
    }
    InitGpuRes(W, H, 4 * sizeof(float), (BYTE **)data, srgb, TEX_TYPE_CUBE);
    for (int i = 0; i < 6; i++)
      stbi_image_free(data[i]);
  }
}

inline float texture::Sample(vec2 coord, float* data, int W, int H, int numChannels, int channel)
{
  int x = (W - 1) * coord.GetX();
  int y = (H - 1) * coord.GetZ();
  int x1 = min(x + 1, W - 1);
  int y1 = min(y + 1, H - 1);
  float fraqX = (W - 1) * coord.GetX() - x;
  float fraqY = (H - 1) * coord.GetZ() - y;
  float d00, d01, d10, d11;
  float d0, d1;
  d00 = data[(y * W + x) * numChannels + channel];
  d01 = data[(y * W + x1) * numChannels + channel];
  d10 = data[(y1 * W + x) * numChannels + channel];
  d11 = data[(y1 * W + x1) * numChannels + channel];
  d0 = d00 * (1.0 - fraqY) + d10 * fraqY;
  d1 = d01 * (1.0 - fraqY) + d11 * fraqY;

  return d0 * (1.0 - fraqX) + d1 * fraqX;
}

VOID texture::ProcessEquirectangularImage(const std::string& FileName)
{
    INT n;
    INT W, H;
    FLOAT offset = 1.0f;
    int cubemapSideSize = 512;
    float* squares[6];
    size_t p = FileName.find_last_of('\\');
    std::string path = FileName.substr(0, p);
    //std::string imgName = FileName.substr(p+1, std::string::npos);
    std::string cubeFaces[] = { "posx", "negx", "posy", "negy", "posz", "negz" };

    
    float* data = stbi_loadf( FileName.c_str(), &W, &H, &n, 0);
    if (!data)
    {
        MessageBox(NULL, "Failed to load equirectangular image", "ERROR", MB_OK);
        return;
    }

    for(int i = 0; i < 6; i++)
    {
        squares[i] = new float[cubemapSideSize * cubemapSideSize * n];
    }
    

    vec2 c;
    for (int i = 0; i < cubemapSideSize; i++)
    {
        for (int j = 0; j < cubemapSideSize; j++)
        {
            c = DecartToEquirectangularProjection(offset, -(2 * float(i) / cubemapSideSize - 1), 2 * float(j) / cubemapSideSize - 1);   //U, V
            for (int chan = 0; chan < n; chan++)
            {
                squares[0][(i * cubemapSideSize + j) * n + chan] = Sample(c, data, W, H, n, chan);   //posx
            }

            c = DecartToEquirectangularProjection(-offset,  -(2 * float(i) / cubemapSideSize - 1), -(2 * float(j) / cubemapSideSize - 1));
            for (int chan = 0; chan < n; chan++)
            {
                squares[1][(i * cubemapSideSize + j) * n + chan] = Sample(c, data, W, H, n, chan);   //negx
            }

            c = DecartToEquirectangularProjection(2 * float(j) / cubemapSideSize - 1, offset, -(2 * float(i) / cubemapSideSize - 1));
            for (int chan = 0; chan < n; chan++)
            {
                squares[2][(i * cubemapSideSize + j) * n + chan] = Sample(c, data, W, H, n, chan);    //posy
            }

            c = DecartToEquirectangularProjection(2 * float(j) / cubemapSideSize - 1, -offset, 2 * float(i) / cubemapSideSize - 1);
            for (int chan = 0; chan < n; chan++)
            {
                squares[3][(i * cubemapSideSize + j) * n + chan] = Sample(c, data, W, H, n, chan);    //negy
            }

            c = DecartToEquirectangularProjection(-(2 * float(j) / cubemapSideSize - 1), -(2 * float(i) / cubemapSideSize - 1), offset);
            for (int chan = 0; chan < n; chan++)
            {
                squares[5][(i * cubemapSideSize + j) * n + chan] = Sample(c, data, W, H, n, chan);    //negz
            }

            c = DecartToEquirectangularProjection(2 * float(j) / cubemapSideSize - 1, -(2 * float(i) / cubemapSideSize - 1), -offset);
            for (int chan = 0; chan < n; chan++)
            {
                squares[4][(i * cubemapSideSize + j) * n + chan] = Sample(c, data, W, H, n, chan);    //posz
            }
        }
    }
    
    int k = 0;
    for (auto &square : squares)
    {
        stbi_write_hdr((path + "\\" + cubeFaces[k] + ".hdr").c_str(), cubemapSideSize, cubemapSideSize, n, square);
        k++;
        delete[] square;
    }
    //stbi_image_free(rawData);
    stbi_image_free(data);
}

vec2 texture::DecartToEquirectangularProjection(FLOAT x, FLOAT y, FLOAT z)    //returns U, V
{
    FLOAT sum = sqrt(x * x + y * y + z * z);
    x = x / sum;
    y = y / sum;
    z = z / sum;

    float U, V;
    U = 1.0f - atan2f(z, x) / (2 * mth::PI);
    V = 1.0f -( asin(y) / mth::PI + 0.5f);

    while (U > 1) 
    {
        U -= 1;
    }

    while (U < 0) 
    {
        U += 1;
    }

    while (V > 1)
    {
        V -= 1;
    }
    while (V < 0)
    {
        V += 1;
    }

    return vec2(U, V);
}

texture::texture(const std::string& NewName, INT NewW, INT NewH, INT BytesPerPixel, BYTE* Pixels) :
  Name(NewName), W(NewW), H(NewH), Type(TEX_TYPE_PLAIN)
{
  InitGpuRes(NewW, NewH, BytesPerPixel, &Pixels);
}

VOID texture::InitGpuRes( INT NewW, INT NewH, INT BytesPerPixel, BYTE** Pixels, BOOL srgb, tex_type NewType)
{
  render* rnd = anim::GetAnim();
  HRESULT hr;
  // format selection
  DXGI_FORMAT format;
  if (NewType == tex_type::TEX_TYPE_CUBE)
    format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  else if (BytesPerPixel * 8 == 32 * 3)
    format = DXGI_FORMAT_R32G32B32_UINT;
  else if (BytesPerPixel * 8 == 32 * 4)
    format = DXGI_FORMAT_R32G32B32A32_UINT;
  else if (BytesPerPixel * 8 == 16 * 4)
    format = DXGI_FORMAT_R16G16B16A16_UINT;
  else if (BytesPerPixel * 8 == 8 * 4)
    if (srgb)
      format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    else
      format = DXGI_FORMAT_R8G8B8A8_UNORM;
  else
  {
    MessageBox(NULL, "Unknown texture format", "ERROR", MB_OK);
    return;
  }

  D3D11_TEXTURE2D_DESC textureDesc;
  ZeroMemory(&textureDesc, sizeof(textureDesc));
  textureDesc.Width = W;
  textureDesc.Height = H;
  textureDesc.MipLevels = 1;
  textureDesc.ArraySize = 1;
  textureDesc.Format = format;
  textureDesc.SampleDesc.Count = 1;
  textureDesc.Usage = D3D11_USAGE_DEFAULT;
  textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  textureDesc.CPUAccessFlags = 0;
  textureDesc.MiscFlags = 0;


  D3D11_SUBRESOURCE_DATA subres;
  subres.SysMemPitch = W * BytesPerPixel;
  subres.SysMemSlicePitch = 0; // Not needed since this is a 2d texture

  if (NewType == TEX_TYPE_PLAIN)
  {
    subres.pSysMem = *Pixels;
    hr = rnd->GetDriver()->CreateTexture2D(&textureDesc, &subres, &Texture);
    if (FAILED(hr))
    {
      MessageBox(NULL, "Failed to create texture", "ERROR", MB_OK);
      return;
    }
  }
  else if (NewType == TEX_TYPE_CUBE)
  {
    textureDesc.Usage = D3D11_USAGE_STAGING;
    textureDesc.BindFlags = 0;
    textureDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    for (int i = 0; i < 6; i++)
    {
      subres.pSysMem = Pixels[i];
      hr = rnd->GetDriver()->CreateTexture2D(&textureDesc, &subres, &TextureCubeFaces[i]);
      if (FAILED(hr))
      {
        MessageBox(NULL, "Failed to create cube face", "ERROR", MB_OK);
        return;
      }
    }
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.ArraySize = 6;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    textureDesc.CPUAccessFlags &= ~(D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE);
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    hr = rnd->GetDriver()->CreateTexture2D(&textureDesc,  NULL, &Texture);
    if (FAILED(hr))
    {
      MessageBox(NULL, "Failed to create cube texture", "ERROR", MB_OK);
      return;
    }
    for (int i = 0; i < 6; i++)
    {

      D3D11_MAPPED_SUBRESOURCE mappedTex2D;
      hr = (rnd->GetDriverIterface()->Map(TextureCubeFaces[i], 0, D3D11_MAP_READ, 0, &mappedTex2D));
      if (FAILED(hr))
      {
        MessageBox(NULL, "Failed to map cube texture", "ERROR", MB_OK);
        return;
      }
      rnd->GetDriverIterface()->UpdateSubresource(Texture,
        D3D11CalcSubresource(0, i, textureDesc.MipLevels),
        0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch);

      rnd->GetDriverIterface()->Unmap(TextureCubeFaces[i], 0);
      TextureCubeFaces[i]->Release();
      TextureCubeFaces[i] = NULL;
    }
  }


  D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
  shaderResourceViewDesc.Format = textureDesc.Format;
  shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
  shaderResourceViewDesc.Texture2D.MipLevels = 1;
  if (NewType == TEX_TYPE_PLAIN)
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  else if (NewType == TEX_TYPE_CUBE)
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;

  rnd->GetDriver()->CreateShaderResourceView(Texture, &shaderResourceViewDesc, &TextureSRV);
  if (FAILED(hr))
  {
    Texture->Release();
    MessageBox(NULL, "Failed to create srv", "ERROR", MB_OK);
    return;
  }

  D3D11_SAMPLER_DESC samplerDesc;
  ZeroMemory(&samplerDesc, sizeof(samplerDesc));
  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
  rnd->GetDriver()->CreateSamplerState(&samplerDesc, &Sampler);
  if (FAILED(hr))
  {
    Texture->Release();
    TextureSRV->Release();
    MessageBox(NULL, "Failed to create ssampler", "ERROR", MB_OK);
    return;
  }
}


VOID texture::Apply(UINT slot)
{
  render* rnd = anim::GetAnim();
  rnd->GetDriverIterface()->PSSetShaderResources(slot, 1, &TextureSRV);
  rnd->GetDriverIterface()->PSSetSamplers(slot, 1, &Sampler);
} 

VOID texture::ApplyNull(UINT slot)
{
  render* rnd = anim::GetAnim();
  ID3D11ShaderResourceView* ref = nullptr;
  rnd->GetDriverIterface()->PSSetShaderResources(slot, 1, &ref);
  rnd->GetDriverIterface()->PSSetSamplers(slot, 1, rnd->GetDefaultLinearSampler());

}


