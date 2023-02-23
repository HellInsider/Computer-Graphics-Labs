#include "anim.h"


ragl::material::material(const std::string& NewName,
  const color3& NewKa/* = color3(0.1, 0.3, 0.2)*/, const color3& NewKd/* = color3(0.9)*/, const color3& NewKs/* = color3(0.2)*/,
  FLT NewPh/* = 30*/, FLT NewTrans/* = 0*/,
  shader* NewShd/* = NULL*/, texture* NewTex0/* = NULL*/, texture* NewTex1/* = NULL*/) :
  Name(NewName), MaterialData({ NewKa, NewPh, NewKd, NewTrans, NewKs }),
  Shd(NewShd)
{
  Tex[0] = NewTex0;
  Tex[1] = NewTex1;

  MaterialData.IsTex0 = Tex[0] != NULL;
  MaterialData.IsTex1 = Tex[1] != NULL;

  ragl::render* rnd = anim::GetAnim();

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

VOID ragl::material::Apply( VOID )
{
  ragl::render *rnd = anim::GetAnim();

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
    MaterialData.IsTex0 = FALSE;

  if (Tex[1] != NULL)
  {
    Tex[1]->Apply(TEXTURE_MATERIAL_SLOT_1);
    MaterialData.IsTex1 = TRUE;
  }
  else
    MaterialData.IsTex1 = FALSE;

  rnd->GetDriverIterface()->UpdateSubresource(ConstBuffer, 0, nullptr, &MaterialData, 0, 0);
  rnd->GetDriverIterface()->PSSetConstantBuffers(CONST_BUF_SLOT_MATERIAL, 1, &ConstBuffer);

 }

HRESULT ragl::shader::CompileShaderFromFile(const CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, const D3D_SHADER_MACRO* defines)
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


ragl::shader::shader(const std::string& FileNamePrefix, const D3D_SHADER_MACRO* defines) : Name(FileNamePrefix)
{
  ragl::render* rnd = anim::GetAnim();
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

VOID ragl::shader::Apply(VOID)
{
  /* Enable shader */
  ragl::render* rnd = anim::GetAnim();

  rnd->GetDriverIterface()->VSSetShader(VertexShader, nullptr, 0);
  rnd->GetDriverIterface()->PSSetShader(PixelShader, nullptr, 0);
} 


ragl::texture::texture(const std::string& NewName, INT NewW, INT NewH, INT BytesPerPixel, BYTE* Pixels) :
  Name(NewName), W(NewW), H(NewH)
{
  ragl::render* rnd = anim::GetAnim();
  HRESULT hr;
  // format selection
  DXGI_FORMAT format;
  if (BytesPerPixel * 8 == 32 * 3)
    format = DXGI_FORMAT_R32G32B32_UINT;
  else if (BytesPerPixel * 8 == 32 * 4)
    format = DXGI_FORMAT_R32G32B32A32_UINT;
  else if (BytesPerPixel * 8 == 16 * 4)
    format = DXGI_FORMAT_R16G16B16A16_UINT;
  else if (BytesPerPixel * 8 == 8 * 4)
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
  subres.pSysMem = Pixels;
  subres.SysMemPitch = W * BytesPerPixel;
  subres.SysMemSlicePitch = 0; // Not needed since this is a 2d texture

  hr = rnd->GetDriver()->CreateTexture2D(&textureDesc, &subres, &Texture);
  if (FAILED(hr))
  {
    MessageBox(NULL, "Failed to create texture", "ERROR", MB_OK);
    return;
  }


  D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
  shaderResourceViewDesc.Format = textureDesc.Format;
  shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
  shaderResourceViewDesc.Texture2D.MipLevels = 1;

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

VOID ragl::texture::Apply(UINT slot)
{
  ragl::render* rnd = anim::GetAnim();
  rnd->GetDriverIterface()->PSSetShaderResources(slot, 1, &TextureSRV);
  rnd->GetDriverIterface()->PSSetSamplers(slot, 1, &Sampler);
} 

