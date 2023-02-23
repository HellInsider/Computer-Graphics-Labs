#include "RENDER.H"
#include "ANIM.H"

const D3D_SHADER_MACRO ragl::render::measure_luminance_system::SHADER_CONSTS[][2] =
{
  {"LUM_STAGE_EVAL_LOGLUM", "1", nullptr, nullptr},
  {"LUM_STAGE_DOWNSAMPLE", "1", nullptr, nullptr},
  {"LUM_STAGE_FINAL", "1", nullptr, nullptr}
};

ragl::render::measure_luminance_system::measure_luminance_system(render* Rnd) :
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
  for (int i = 0; i <= LUM_RESULT; i++)
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
    if (i == LUM_RESULT - 1)
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

ragl::render::measure_luminance_system::~measure_luminance_system(VOID)
{
  for (int i = 0; i <= LUM_RESULT; i++)
  {
    if (LuminanceSRV[i])
      LuminanceSRV[i]->Release();
    if (LuminanceRT[i])
      LuminanceRT[i]->Release();
    if (LuminanceTex[i])
      LuminanceTex[i]->Release();
  }
  if (ScreenSpaceConstBuf)
    ScreenSpaceConstBuf->Release();
}

VOID ragl::render::measure_luminance_system::Draw(render* Rnd, ID3D11ShaderResourceView *src)
{
  screenspace_cb_data d;
  d.sx = Rnd->SX;
  d.sy = Rnd->SY;
  d.sxInv = 1.0 / d.sx;
  d.syInv = 1.0 / d.sy;


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
        ShdFinal.Apply();
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
}

VOID Resize(INT w, INT h);

ragl::render::render(HWND hwnd) : hWnd(hwnd), hDC(GetDC(hWnd)), Cam()
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
  IDXGIFactory2* dxgiFactory2 = nullptr;
  hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
  if (dxgiFactory2)
  {
    // DirectX 11.1 or later
    hr = Driver->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&Driver1));
    if (SUCCEEDED(hr))
    {
      (void)DriverIterface->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&DriverIterface1));
    }

    DXGI_SWAP_CHAIN_DESC1 sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.Width = width;
    sd.Height = height;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 1;

    hr = dxgiFactory2->CreateSwapChainForHwnd(Driver, hWnd, &sd, nullptr, nullptr, &SwapChain1);
    if (SUCCEEDED(hr))
    {
      hr = SwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&SwapChain));
    }

    dxgiFactory2->Release();
  }
  else
  {
    // DirectX 11.0 systems
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
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

  // Create a render target view
  ID3D11Texture2D* pBackBuffer = nullptr;
  hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
  if (FAILED(hr))
    MessageBox(NULL, "Failed to init backbuf", "ERROR", MB_OK);

  hr = Driver->CreateRenderTargetView(pBackBuffer, nullptr, &RenderTargetView);
  if (FAILED(hr))
    MessageBox(NULL, "Failed to CreateRenderTargetView", "ERROR", MB_OK);

  pBackBuffer->Release();

  D3D11_TEXTURE2D_DESC textureDesc;
  ZeroMemory(&textureDesc, sizeof(textureDesc));
  textureDesc.Width = width;
  textureDesc.Height = height;
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
  descDepth.Width = width;
  descDepth.Height = height;
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
  vp.Width = (FLOAT)width;
  vp.Height = (FLOAT)height;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  DriverIterface->RSSetViewports(1, &vp);

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

  D3D11_QUERY_DESC qd;
  qd.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
  qd.MiscFlags = 0;
  hr = Driver->CreateQuery(&qd, &TimeQuery);
  if (FAILED(hr))
  {
    MessageBox(NULL, "Failed to create TimeQuery", "ERROR", MB_OK);
  }
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

VOID ragl::render::Resize( INT w, INT h )
{
  D3D11_VIEWPORT vp;
  vp.Width = (FLOAT)w;
  vp.Height = (FLOAT)h;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  DriverIterface->RSSetViewports(1, &vp);

  Cam.Resize(w, h);
} 

VOID ragl::render::FrameStart( VOID )
{
  float clearColor[] = { 0.01, 0.05, 0.3, 1.0 };
  DriverIterface->ClearRenderTargetView(ColorPassRTView, clearColor);
  DriverIterface->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

  common_cb_data d;
  d.MatrView.fromPtr((float *)Cam.GetMatrView());
  d.MatrProj.fromPtr((float *)Cam.GetMatrProj());
  d.CamLoc = Cam.GetLoc();
  d.Time = anim::GetAnim()->GetTime();

  DriverIterface->UpdateSubresource(CommonConstBuf, 0, nullptr, &d, 0, 0);
  DriverIterface->VSSetConstantBuffers(CONST_BUF_SLOT_COMMON, 1, &CommonConstBuf);
  DriverIterface->PSSetConstantBuffers(CONST_BUF_SLOT_COMMON, 1, &CommonConstBuf);

  SetTransparency(FALSE);

  DriverIterface->OMSetRenderTargets(1, &ColorPassRTView, DepthStencilView);
} 

VOID ragl::render::FrameCopy( VOID )
{
  //SwapBuffers(hDC);
} 

VOID ragl::render::FrameEnd( VOID )
{
  SysMeasureLum->Draw(this, ColorPassShadResView);

  DriverIterface->OMSetRenderTargets(1, &RenderTargetView, nullptr);
  ShdTonemap->Apply();
  DriverIterface->PSSetShaderResources(TEXTURE_CONSTANTS_SLOT, 1, &SysMeasureLum->LuminanceSRV[measure_luminance_system::LUM_RESULT]);
  DriverIterface->PSSetShaderResources(TEXTURE_SCREENSPACE_SLOT, 1, &ColorPassShadResView);
  ScreenQuad->Draw(matr(), TRUE);

  // unbound
/*  DriverIterface->OMSetRenderTargets(1, nullptr, nullptr);
  DriverIterface->PSSetShaderResources(0, k_NUM_OF_SLOTS, nullptr);*/

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
} 

ragl::render::~render( VOID )
{
  /* Delete OpenGL data */
  if (DriverIterface) DriverIterface->ClearState();

  delete ShdTonemap;
  delete SysMeasureLum;
  delete ScreenQuad;

  if (DepthStencilTex) DepthStencilTex->Release();
  if (DepthStencilView) DepthStencilView->Release();
  if (ColorPassRTView) ColorPassRTView->Release();
  if (ColorPassShadResView) ColorPassShadResView->Release();
  if (ColorPassTex) ColorPassTex->Release();
  if (RenderTargetView) RenderTargetView->Release();
  if (CommonConstBuf) CommonConstBuf->Release();
  if (BlendStateOpaqe) BlendStateOpaqe->Release();
  if (BlendStateTransparent) BlendStateTransparent->Release();
  if (SwapChain1) SwapChain1->Release();
  if (SwapChain) SwapChain->Release();
  if (DriverIterface1) DriverIterface1->Release();
  if (DriverIterface) DriverIterface->Release();
  if (Driver1) Driver1->Release();
  if (Driver) Driver->Release();
  /* Delete GDI data */
   ReleaseDC(hWnd, hDC);
} 

 ragl::shader * ragl::render::AddShader( const std::string &FileNamePrefix )
{
  shader *shd;
  if ((shd = Shaders.Find(FileNamePrefix)) != NULL)
    return shd;
  Shaders.Add(shd = new shader(FileNamePrefix));
  return shd;
} 

 ragl::texture * ragl::render::AddTexture( const std::string &Name, INT W, INT H, INT BytesPerPixel, BYTE *Pixels )
{
  texture *tex;
  if ((tex = Textures.Find(Name)) != NULL)
    return tex;
  Textures.Add(tex = new texture(Name, W, H, BytesPerPixel, Pixels));
  return tex;
} 

 ragl::material * ragl::render::AddMaterial( const std::string &Name,
                        const color3 &Ka, const color3 &Kd, const color3 &Ks,
                        FLT Ph, FLT Trans,
                        shader *Shd, texture *Tex0, texture *Tex1 )
{
  material *mtl;
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