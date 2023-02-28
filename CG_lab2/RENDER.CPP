#include "RENDER.H"
#include "ANIM.H"

const D3D_SHADER_MACRO render::measure_luminance_system::SHADER_CONSTS[][2] =
{
  {"LUM_STAGE_EVAL_LOGLUM", "1", nullptr, nullptr},
  {"LUM_STAGE_DOWNSAMPLE", "1", nullptr, nullptr},
  {"LUM_STAGE_FINAL", "1", nullptr, nullptr}
};

render::measure_luminance_system::measure_luminance_system(render* Rnd) :
  ShdEvalLogLum("shaders\\measure_lum", SHADER_CONSTS[0]),
  ShdDownsample("shaders\\measure_lum", SHADER_CONSTS[1]),
  ShdFinal("shaders\\measure_lum", SHADER_CONSTS[2]),
  quad(topology::screenspace_quad())
{
  D3D11_TEXTURE2D_DESC descRTtex;
  ZeroMemory(&descRTtex, sizeof(descRTtex));
  descRTtex.Width = Rnd->SX;
  descRTtex.Height = Rnd->SY;
  descRTtex.MipLevels = 1;
  descRTtex.ArraySize = 1;
  descRTtex.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
  descRTtex.SampleDesc.Count = 1;
  descRTtex.SampleDesc.Quality = 0;
  descRTtex.Usage = D3D11_USAGE_DEFAULT;
  descRTtex.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  descRTtex.CPUAccessFlags = 0;
  descRTtex.MiscFlags = 0;
  HRESULT hr;
  D3D11_RENDER_TARGET_VIEW_DESC descRT;
  descRT.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  descRT.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  descRT.Texture2D.MipSlice = 0;
  D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
  descSRV.Format = descRT.Format;
  descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  descSRV.Texture2D.MostDetailedMip = 0;
  descSRV.Texture2D.MipLevels = 1;
  for (int i = 0; i <= LUM_OLD_FRAME; i++)
  {
    hr = Rnd->Driver->CreateTexture2D(&descRTtex, nullptr, &LuminanceTex[i]);
    if (FAILED(hr))
      MessageBox(NULL, "Failed to Create LuminanceTex", "ERROR", MB_OK);
    hr = Rnd->Driver->CreateRenderTargetView(LuminanceTex[i], &descRT, &LuminanceRT[i]);
    if (FAILED(hr))
      MessageBox(NULL, "Failed to Create LuminanceRT", "ERROR", MB_OK);
    hr = Rnd->Driver->CreateShaderResourceView(LuminanceTex[i], &descSRV, &LuminanceSRV[i]);
    if (FAILED(hr))
      MessageBox(NULL, "Failed to Create LuminanceSRV", "ERROR", MB_OK);
    if (i >= LUM_RESULT - 1)
    {
      descRTtex.Width = descRTtex.Height = 1;
      descRTtex.Format = descRT.Format = descSRV.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    }
    else
    {
      descRTtex.Width /= 4;
      descRTtex.Height /= 4;
    }
  }

  D3D11_BUFFER_DESC bd;
  ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(screenspace_cb_data);
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.CPUAccessFlags = 0;
  hr = Rnd->Driver->CreateBuffer(&bd, nullptr, &ScreenSpaceConstBuf);
  if (FAILED(hr))
    MessageBox(NULL, "Failed to create screenspace cb", "ERROR", MB_OK);

}

render::measure_luminance_system::~measure_luminance_system(VOID)
{
  for (int i = 0; i <= LUM_OLD_FRAME; i++)
    if (LuminanceSRV[i])
      LuminanceSRV[i]->Release();
  for (int i = 0; i <= LUM_OLD_FRAME; i++)
    if (LuminanceRT[i])
      LuminanceRT[i]->Release();
  for (int i = 0; i <= LUM_OLD_FRAME; i++)
    if (LuminanceTex[i])
      LuminanceTex[i]->Release();

  if (ScreenSpaceConstBuf)
    ScreenSpaceConstBuf->Release();
}

VOID render::measure_luminance_system::Draw(render* Rnd, ID3D11ShaderResourceView* src)
{
  screenspace_cb_data d;
  d.sx = Rnd->SX;
  d.sy = Rnd->SY;
  d.sxInv = 1.0 / d.sx;
  d.syInv = 1.0 / d.sy;


  Rnd->Annotation->BeginEvent(L"Downsample");
  for (int i = 0; i <= LUM_RESULT; i++)
  {
    Rnd->DriverIterface->OMSetRenderTargets(1, &LuminanceRT[i], nullptr);
    if (i > 0)
    {
      if (i != LUM_FIRST_DOWNSAMPLE)
      {
        d.sx /= 4;
        d.sy /= 4;
      }
      if (i == LUM_RESULT)
      {
        ShdFinal.Apply();
        Rnd->DriverIterface->PSSetShaderResources(TEXTURE_CONSTANTS_SLOT_2, 1, &LuminanceSRV[LUM_OLD_FRAME]);
      }
      else
        ShdDownsample.Apply();
      Rnd->DriverIterface->PSSetShaderResources(TEXTURE_SCREENSPACE_SLOT, 1, &LuminanceSRV[i - 1]);
    }
    else
    {
      ShdEvalLogLum.Apply();
      Rnd->DriverIterface->PSSetShaderResources(TEXTURE_SCREENSPACE_SLOT, 1, &src);
    }
    d.sxInv = 1.0 / d.sx;
    d.syInv = 1.0 / d.sy;
    Rnd->DriverIterface->UpdateSubresource(ScreenSpaceConstBuf, 0, nullptr, &d, 0, 0);
    Rnd->DriverIterface->VSSetConstantBuffers(CONST_BUF_SLOT_SCREENSPACE, 1, &ScreenSpaceConstBuf);
    Rnd->DriverIterface->PSSetConstantBuffers(CONST_BUF_SLOT_SCREENSPACE, 1, &ScreenSpaceConstBuf);

    quad.Draw(matr(), TRUE);
  }
  Rnd->DriverIterface->CopyResource(LuminanceTex[LUM_OLD_FRAME], LuminanceTex[LUM_RESULT]);

  Rnd->Annotation->EndEvent();
}


render::render(HWND hwnd) : hWnd(hwnd), hDC(GetDC(hWnd)), Cam()
{

  HRESULT hr = S_OK;

  RECT rc;
  GetClientRect(hWnd, &rc);
  UINT width = rc.right - rc.left;
  UINT height = rc.bottom - rc.top;

  SX = width;
  SY = height;

  UINT createDeviceFlags = 0;
#ifdef _DEBUG
  createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_DRIVER_TYPE driverTypes[] =
  {
      D3D_DRIVER_TYPE_HARDWARE,
      D3D_DRIVER_TYPE_WARP,
      D3D_DRIVER_TYPE_REFERENCE,
  };
  UINT numDriverTypes = ARRAYSIZE(driverTypes);

  D3D_FEATURE_LEVEL featureLevels[] =
  {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
  };
  UINT numFeatureLevels = ARRAYSIZE(featureLevels);

  for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
  {
    DriverType = driverTypes[driverTypeIndex];
    hr = D3D11CreateDevice(nullptr, DriverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
      D3D11_SDK_VERSION, &Driver, &FeatureLevel, &DriverIterface);

    if (hr == E_INVALIDARG)
    {
      // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
      hr = D3D11CreateDevice(nullptr, DriverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
        D3D11_SDK_VERSION, &Driver, &FeatureLevel, &DriverIterface);
    }

    if (SUCCEEDED(hr))
      break;
  }
  if (FAILED(hr))
    MessageBox(NULL, "Failed to create device", "ERROR", MB_OK);

  Driver->QueryInterface(IID_ID3D11Debug, (void**)&debug);//my str

  // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
  IDXGIFactory1* dxgiFactory = nullptr;
  {
    IDXGIDevice* dxgiDevice = nullptr;
    hr = Driver->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
    if (SUCCEEDED(hr))
    {
      IDXGIAdapter* adapter = nullptr;
      hr = dxgiDevice->GetAdapter(&adapter);
      if (SUCCEEDED(hr))
      {
        hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
        adapter->Release();
      }
      dxgiDevice->Release();
    }
  }
  if (FAILED(hr))
    MessageBox(NULL, "Failed to Obtain DXGI factory", "ERROR", MB_OK);

  // Create swap chain
  {
    // DirectX 11.0 systems
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    hr = dxgiFactory->CreateSwapChain(Driver, &sd, &SwapChain);
  }

  // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
  dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

  dxgiFactory->Release();

  if (FAILED(hr))
    MessageBox(NULL, "Failed to init dx", "ERROR", MB_OK);

  InitSizeDependentResources();

  D3D11_BUFFER_DESC bd;
  ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(common_cb_data);
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.CPUAccessFlags = 0;
  hr = Driver->CreateBuffer(&bd, nullptr, &CommonConstBuf);
  if (FAILED(hr))
  {
    MessageBox(NULL, "Failed to create common cb", "ERROR", MB_OK);
    return;
  }
  bd.ByteWidth = sizeof(light_cb_data);
  hr = Driver->CreateBuffer(&bd, nullptr, &LightConstBuf);
  if (FAILED(hr))
  {
    MessageBox(NULL, "Failed to create light cb", "ERROR", MB_OK);
    return;
  }

  D3D11_BLEND_DESC BlendState;
  ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC));
  BlendState.RenderTarget[0].BlendEnable = FALSE;
  BlendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
  hr = Driver->CreateBlendState(&BlendState, &BlendStateOpaqe);
  if (FAILED(hr))
  {
    MessageBox(NULL, "Failed to create BlendStateOpaqe", "ERROR", MB_OK);
  }

  BlendState.RenderTarget[0].BlendEnable = TRUE;
  BlendState.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  BlendState.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  BlendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  BlendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  BlendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
  BlendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  BlendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
  hr = Driver->CreateBlendState(&BlendState, &BlendStateTransparent);
  if (FAILED(hr))
  {
    MessageBox(NULL, "Failed to create BlendStateTransparent", "ERROR", MB_OK);
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
  Driver->CreateSamplerState(&samplerDesc, &DefaultLinearSampler);
  if (FAILED(hr))
  {
    MessageBox(NULL, "Failed to create DefaultLinearSampler", "ERROR", MB_OK);
    return;
  }


  D3D11_QUERY_DESC qd;
  qd.Query = D3D11_QUERY_TIMESTAMP;
  qd.MiscFlags = 0;
  hr = Driver->CreateQuery(&qd, &TimeQuery);
  if (FAILED(hr))
  {
    MessageBox(NULL, "Failed to create TimeQuery", "ERROR", MB_OK);
  }


  AddMaterial("default", color3(0.1, 0.3, 0.2), color3(0.9), color3(0.0), 30, 0,
    AddShader("shaders\\def"), NULL, NULL);

  ShdTonemap = new shader("shaders\\tonemap");
  ScreenQuad = new prim(topology::screenspace_quad());

  SysMeasureLum = new measure_luminance_system(this);
}

VOID render::InitSizeDependentResources(VOID)
{
  // Create a render target view
  ID3D11Texture2D* pBackBuffer = nullptr;
  HRESULT hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
  if (FAILED(hr))
    MessageBox(NULL, "Failed to init backbuf", "ERROR", MB_OK);

  hr = Driver->CreateRenderTargetView(pBackBuffer, nullptr, &RenderTargetView);
  if (FAILED(hr))
    MessageBox(NULL, "Failed to CreateRenderTargetView", "ERROR", MB_OK);

  pBackBuffer->Release();

  D3D11_TEXTURE2D_DESC textureDesc;
  ZeroMemory(&textureDesc, sizeof(textureDesc));
  textureDesc.Width = SX;
  textureDesc.Height = SY;
  textureDesc.MipLevels = 1;
  textureDesc.ArraySize = 1;
  textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
  textureDesc.SampleDesc.Count = 1;
  textureDesc.Usage = D3D11_USAGE_DEFAULT;
  textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
  textureDesc.CPUAccessFlags = 0;
  textureDesc.MiscFlags = 0;

  hr = Driver->CreateTexture2D(&textureDesc, nullptr, &ColorPassTex);
  if (FAILED(hr))
  {
    MessageBox(NULL, "Failed to create ColorPassTex", "ERROR", MB_OK);
    return;
  }


  D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
  shaderResourceViewDesc.Format = textureDesc.Format;
  shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
  shaderResourceViewDesc.Texture2D.MipLevels = 1;

  Driver->CreateShaderResourceView(ColorPassTex, &shaderResourceViewDesc, &ColorPassShadResView);
  if (FAILED(hr))
  {
    ColorPassTex->Release();
    MessageBox(NULL, "Failed to create ColorPassShadResView", "ERROR", MB_OK);
    return;
  }
  hr = Driver->CreateRenderTargetView(ColorPassTex, nullptr, &ColorPassRTView);
  if (FAILED(hr))
    MessageBox(NULL, "Failed to ColorPassRTView", "ERROR", MB_OK);


  // Create depth stencil texture
  D3D11_TEXTURE2D_DESC descDepth;
  ZeroMemory(&descDepth, sizeof(descDepth));
  descDepth.Width = SX;
  descDepth.Height = SY;
  descDepth.MipLevels = 1;
  descDepth.ArraySize = 1;
  descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  descDepth.SampleDesc.Count = 1;
  descDepth.SampleDesc.Quality = 0;
  descDepth.Usage = D3D11_USAGE_DEFAULT;
  descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  descDepth.CPUAccessFlags = 0;
  descDepth.MiscFlags = 0;
  hr = Driver->CreateTexture2D(&descDepth, nullptr, &DepthStencilTex);
  if (FAILED(hr))
    MessageBox(NULL, "Failed to Create DepthStencilTex", "ERROR", MB_OK);

  // Create the depth stencil view
  D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
  ZeroMemory(&descDSV, sizeof(descDSV));
  descDSV.Format = descDepth.Format;
  descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  descDSV.Texture2D.MipSlice = 0;
  hr = Driver->CreateDepthStencilView(DepthStencilTex, &descDSV, &DepthStencilView);
  if (FAILED(hr))
    MessageBox(NULL, "Failed to Create DepthStencilView", "ERROR", MB_OK);

  DriverIterface->OMSetRenderTargets(1, &ColorPassRTView, DepthStencilView);

  // Setup the viewport
  D3D11_VIEWPORT vp;
  vp.Width = (FLOAT)SX;
  vp.Height = (FLOAT)SY;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  DriverIterface->RSSetViewports(1, &vp);
}

VOID render::Resize(INT w, INT h)
{
  SX = w;
  SY = h;

  DriverIterface->OMSetRenderTargets(0, nullptr, nullptr);
  delete SysMeasureLum;
  if (DepthStencilTex) DepthStencilTex->Release();
  if (DepthStencilView) DepthStencilView->Release();
  if (ColorPassRTView) ColorPassRTView->Release();
  if (ColorPassShadResView) ColorPassShadResView->Release();
  if (ColorPassTex) ColorPassTex->Release();
  if (RenderTargetView) RenderTargetView->Release();

  HRESULT hr;
  hr = SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

  if (FAILED(hr))
    MessageBox(NULL, "Failed to resize backbuf", "ERROR", MB_OK);

  InitSizeDependentResources();

  SysMeasureLum = new measure_luminance_system(this);

  Cam.Resize(w, h);
}

VOID render::FrameStart(VOID)
{
  float clearColor[] = { 0.0, 0.0, 0.0, 1.0 };
  DriverIterface->ClearRenderTargetView(ColorPassRTView, clearColor);
  DriverIterface->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

  common_cb_data d;
  d.MatrView.fromPtr((float*)Cam.GetMatrView());
  d.MatrProj.fromPtr((float*)Cam.GetMatrProj());
  d.CamLoc = Cam.GetLoc();
  d.DeltaTime = anim::GetAnim()->GetDeltaTime();

  DriverIterface->UpdateSubresource(CommonConstBuf, 0, nullptr, &d, 0, 0);
  DriverIterface->VSSetConstantBuffers(CONST_BUF_SLOT_COMMON, 1, &CommonConstBuf);
  DriverIterface->PSSetConstantBuffers(CONST_BUF_SLOT_COMMON, 1, &CommonConstBuf);

  light_cb_data ld;
  ld.NumLighs = (unsigned)(Lights.size());
  for (int i = 0; i < MAX_LIGHTS; i++)
  {
    if (i < Lights.size())
    {
      ld.RawLights[i].Type = Lights[i]->Type;
      ld.RawLights[i].Color = Lights[i]->Color;
      ld.RawLights[i].PosDir = Lights[i]->Type == light::LIGHT_POINT ? Lights[i]->Pos : Lights[i]->Dir;
      ld.RawLights[i].Atten = Lights[i]->Attenuation;
    }
    else
    {
      ld.RawLights[i].Color = color(0);
    }
  }
  DriverIterface->UpdateSubresource(LightConstBuf, 0, nullptr, &ld, 0, 0);
  DriverIterface->PSSetConstantBuffers(CONST_BUF_SLOT_LIGHT, 1, &LightConstBuf);

  SetTransparency(FALSE);

  DriverIterface->OMSetRenderTargets(1, &ColorPassRTView, DepthStencilView);

  DriverIterface->QueryInterface(__uuidof(Annotation), reinterpret_cast<void**>(&Annotation));

  Annotation->BeginEvent(L"ColorPass");
}

VOID render::FrameCopy(VOID)
{
}

VOID render::FrameEnd(VOID)
{
  Annotation->EndEvent();
  SysMeasureLum->Draw(this, ColorPassShadResView);

  DriverIterface->OMSetRenderTargets(1, &RenderTargetView, nullptr);
  ShdTonemap->Apply();
  DriverIterface->PSSetShaderResources(TEXTURE_CONSTANTS_SLOT, 1, &SysMeasureLum->LuminanceSRV[measure_luminance_system::LUM_RESULT]);
  DriverIterface->PSSetShaderResources(TEXTURE_SCREENSPACE_SLOT, 1, &ColorPassShadResView);
  ScreenQuad->Draw(matr(), TRUE);
  ID3D11ShaderResourceView *ref[1] = { nullptr };
  DriverIterface->PSSetShaderResources(TEXTURE_SCREENSPACE_SLOT, 1, ref);
  DriverIterface->PSSetShaderResources(TEXTURE_CONSTANTS_SLOT, 1, ref);

  SwapChain->Present(0, 0);

  std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
  if (timed)
  {
    //   std::chrono::duration<double, std::milli> time_span = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(t1 - t);
     //  duration += time_span.count();
  }
  timed = true;
  if (FrameNum == 0)
    t = t1;
  FrameNum++;
  if (FrameNum == 10000)
  {
    std::chrono::duration<double, std::milli> time_span = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(t1 - t);
    duration += time_span.count();

    char buf[30];
    sprintf(buf, "TIME %lf\n", duration / 10000.0);
    OutputDebugString(buf);
    FrameNum = 0;
    duration = 0;
   
  }
  //if(ref[0]) ref[0]->Release(); //IDD: didn't work
  Annotation->Release();
 }

render::~render(VOID)
{
  if (DriverIterface) DriverIterface->ClearState();

  Textures.Clear();
  Materials.Clear();
  Shaders.Clear();
  for (auto &l : Lights)
    delete l;

  delete ShdTonemap;
  delete SysMeasureLum;
  delete ScreenQuad;

  if (TimeQuery) TimeQuery->Release(); //IDD Didn't work

  if (DepthStencilView) DepthStencilView->Release();
  if (DepthStencilTex) DepthStencilTex->Release();
  if (ColorPassRTView) ColorPassRTView->Release();
  if (ColorPassShadResView) ColorPassShadResView->Release();
  if (ColorPassTex) ColorPassTex->Release();
  if (RenderTargetView) RenderTargetView->Release();
  if (CommonConstBuf) CommonConstBuf->Release();
  if (LightConstBuf) LightConstBuf->Release();
  if (BlendStateOpaqe) BlendStateOpaqe->Release();
  if (BlendStateTransparent) BlendStateTransparent->Release();
  if (DefaultLinearSampler) DefaultLinearSampler->Release();
  if (SwapChain) SwapChain->Release();
  if (DriverIterface) DriverIterface->Release();
  if (Driver)
  {
    UINT refs = Driver->Release();
    if (refs > 1) debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
  }
  if (debug) debug->Release();

  /* Delete GDI data */
  ReleaseDC(hWnd, hDC);

}

shader* render::AddShader(const std::string& FileNamePrefix)
{
  shader* shd;
  if ((shd = Shaders.Find(FileNamePrefix)) != NULL)
    return shd;
  Shaders.Add(shd = new shader(FileNamePrefix));
  return shd;
}

texture* render::AddTexture(const std::string& Name, INT W, INT H, INT BytesPerPixel, BYTE* Pixels)
{
  texture* tex;
  if ((tex = Textures.Find(Name)) != NULL)
    return tex;
  Textures.Add(tex = new texture(Name, W, H, BytesPerPixel, Pixels));
  return tex;
}

material* render::AddMaterial(const std::string& Name,
  const color3& Ka, const color3& Kd, const color3& Ks,
  FLT Ph, FLT Trans,
  shader* Shd, texture* Tex0, texture* Tex1)
{
  material* mtl;
  if ((mtl = Materials.Find(Name)) != NULL)
  {
    mtl->MaterialData.Ka = Ka;
    mtl->MaterialData.Kd = Kd;
    mtl->MaterialData.Ks = Ks;
    mtl->MaterialData.Ph = Ph;
    mtl->MaterialData.Trans = Trans;
    mtl->Shd = Shd;
    mtl->Tex[0] = Tex0;
    mtl->Tex[1] = Tex1;
    return mtl;
  }
  Materials.Add(mtl = new material(Name, Ka, Kd, Ks, Ph, Trans, Shd, Tex0, Tex1));
  return mtl;
}

light* render::AddLight(const light::light_type type, const color col, const vec posDir, const vec atten)
{
  if (Lights.size() == MAX_LIGHTS)
    return nullptr;
  light* l = new light(type, col, type == light::LIGHT_POINT ? posDir : vec(0), type == light::LIGHT_DIRECTION ? posDir : vec(0), atten);
  Lights.push_back(l);
  return l;
}


