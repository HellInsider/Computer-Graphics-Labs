#include "RENDER.H"
#include "../Anim/ANIM.H"
#include "../Stb_image/stb_image.h"
#include "../Stb_image/stb_image_write.h"

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

  D3D11_RASTERIZER_DESC RasterizerState;
  ZeroMemory(&RasterizerState, sizeof(D3D11_RASTERIZER_DESC));
  RasterizerState.FillMode = D3D11_FILL_SOLID;
  RasterizerState.CullMode = D3D11_CULL_BACK;
  RasterizerState.DepthClipEnable = TRUE;
  hr = Driver->CreateRasterizerState(&RasterizerState, &RasterizerStateCull);
  if (FAILED(hr))
  {
    MessageBox(NULL, "Failed to create RasterizerStateCull", "ERROR", MB_OK);
  }
  RasterizerState.CullMode = D3D11_CULL_NONE;
  hr = Driver->CreateRasterizerState(&RasterizerState, &RasterizerStateNoCull);
  if (FAILED(hr))
  {
    MessageBox(NULL, "Failed to create RasterizerStateNoCull", "ERROR", MB_OK);
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


  AddMaterial("default", 0.9, 0.99, 0.9, 0,
    AddShader("shaders\\def"), NULL, NULL);

  ShdTonemap = new shader("shaders\\tonemap");
  ScreenQuad = new prim(topology::screenspace_quad());

  SysMeasureLum = new measure_luminance_system(this);
}

VOID render::InitSizeDependentResources(VOID)
{
  // Create a render target view
  ID3D11Texture2D* BackBufferTex = nullptr;
  HRESULT hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&BackBufferTex));
  if (FAILED(hr))
    MessageBox(NULL, "Failed to init backbuf", "ERROR", MB_OK);

  hr = Driver->CreateRenderTargetView(BackBufferTex, nullptr, &RenderTargetView);
  if (FAILED(hr))
    MessageBox(NULL, "Failed to CreateRenderTargetView", "ERROR", MB_OK);
  
  if (BackBufferTex) BackBufferTex->Release();

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
  common_cb_data d;
  d.MatrView.fromPtr((float*)Cam.GetMatrView());
  d.MatrProj.fromPtr((float*)Cam.GetMatrProj());
  d.CamLoc = Cam.GetLoc();
  d.DeltaTime = anim::GetAnim()->GetDeltaTime();
  d.IsEnviromentTexture = sharedTextures.enviromentTex != nullptr;
  d.IsIrradianceTexture = sharedTextures.irradianceMap != nullptr;
  d.IsPrefilteredTexture = sharedTextures.prefilteredMap != nullptr;
  d.IsBRDFLUT = sharedTextures.brdfLUT != nullptr;

  DriverIterface->UpdateSubresource(CommonConstBuf, 0, nullptr, &d, 0, 0);
  DriverIterface->VSSetConstantBuffers(CONST_BUF_SLOT_COMMON, 1, &CommonConstBuf);
  DriverIterface->PSSetConstantBuffers(CONST_BUF_SLOT_COMMON, 1, &CommonConstBuf);

  light_cb_data ld;
  ld.NumLighs = (unsigned)(Lights.size());
  ld.LightingMode = (unsigned)lightMode;
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

  float clearColor[] = { 0.0, 0.0, 0.0, 1.0 };
  DriverIterface->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

  if (tonemapEnabled)
  {
    DriverIterface->ClearRenderTargetView(ColorPassRTView, clearColor);
    DriverIterface->OMSetRenderTargets(1, &ColorPassRTView, DepthStencilView);
  }
  else
  {
    DriverIterface->ClearRenderTargetView(RenderTargetView, clearColor);
    DriverIterface->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);
  }


  DriverIterface->QueryInterface(__uuidof(Annotation), reinterpret_cast<void**>(&Annotation));

  Annotation->BeginEvent(L"ColorPass");
}

VOID render::FrameCopy(VOID)
{
}

VOID render::FrameEnd(VOID)
{
  Annotation->EndEvent();
  if (tonemapEnabled)
  {
    SysMeasureLum->Draw(this, ColorPassShadResView);

    DriverIterface->OMSetRenderTargets(1, &RenderTargetView, nullptr);
    ShdTonemap->Apply();
    DriverIterface->PSSetShaderResources(TEXTURE_CONSTANTS_SLOT, 1, &SysMeasureLum->LuminanceSRV[measure_luminance_system::LUM_RESULT]);
    DriverIterface->PSSetShaderResources(TEXTURE_SCREENSPACE_SLOT, 1, &ColorPassShadResView);
    ScreenQuad->Draw(matr(), TRUE);
    ID3D11ShaderResourceView* ref[1] = { nullptr };
    DriverIterface->PSSetShaderResources(TEXTURE_SCREENSPACE_SLOT, 1, ref);
    DriverIterface->PSSetShaderResources(TEXTURE_CONSTANTS_SLOT, 1, ref);
  }
  else
  {
  }

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

  if (sharedTextures.enviromentTex) delete sharedTextures.enviromentTex;
  if (sharedTextures.irradianceMap) delete sharedTextures.irradianceMap;
  if (sharedTextures.brdfLUT) delete sharedTextures.brdfLUT;
  if (sharedTextures.prefilteredMap) delete sharedTextures.prefilteredMap;
  
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
  if (RasterizerStateCull) RasterizerStateCull->Release();
  if (RasterizerStateNoCull) RasterizerStateNoCull->Release();
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

static inline bool FileExists(std::string name)
{
  if (FILE* file = fopen(name.c_str(), "r"))
  {
    fclose(file);
    return true;
  }
  else
  {
    return false;
  }
}

VOID render::EvaluateIrradiance(std::string filename)
{
  size_t p = filename.find_last_of('\\');
  std::string path = filename.substr(0, p);
  std::string cubeFaces[] = { "posx", "negx", "posy", "negy", "posz", "negz" };
  INT W, H, n;
  float* data = stbi_loadf(filename.c_str(), &W, &H, &n, 0);
  if (!data)
  {
    MessageBox(NULL, "Failed to load equirectangular image", "ERROR", MB_OK);
    return;
  }

  float *irradianseData = new float[6 * irradianseW * irradianseH * n];

  for (int face = 0; face < 6; face++)
    for (int y = 0; y < irradianseH; y++)
      for (int x = 0; x < irradianseW; x++)
      {
        vec normal;
        switch (face)
        {
        case 0:
          normal = !vec(1.0, -(2.0 * float(y) / irradianseH - 1.0), 2.0 * float(x) / irradianseW - 1.0);
          break;
        case 1:
          normal = !vec(-1.0, -(2.0 * float(y) / irradianseH - 1.0), -(2.0 * float(x) / irradianseW - 1.0));
          break;
        case 2:
          normal = !vec(2.0 * float(x) / irradianseW - 1.0, 1.0, -(2.0 * float(y) / irradianseH - 1.0));
          break;
        case 3:
          normal = !vec(2.0 * float(x) / irradianseW - 1.0, -1.0, 2.0 * float(y) / irradianseH - 1.0);
          break;
        case 4:
          normal = !vec(2.0 * float(x) / irradianseW - 1.0, -(2.0 * float(y) / irradianseH - 1.0), -1.0);
          break;
        case 5:
          normal = !vec(-(2.0 * float(x) / irradianseW - 1.0), -(2.0 * float(y) / irradianseH - 1.0), 1.0);
          break;
        }

        vec dir = abs(normal.GetZ()) < 0.999 ? vec(0.0, 0.0, 1.0) : vec(1.0, 0.0, 0.0);
        vec tangent = !(dir % normal);
        vec binormal = normal % tangent;

        int cur = face * irradianseW * irradianseH + y * irradianseW + x;
        for (int comp = 0; comp < n; comp++)
          irradianseData[cur * n + comp] = 0;

        for (int i = 0; i < irradN1; i++)
        {
          for (int j = 0; j < irradN2; j++)
          {
            float phi = i * (2 * mth::PI / irradN1);
            float theta = j * (mth::PI / 2.0 / irradN2);
            vec tangentSample = vec(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec sampleVec = tangent * tangentSample.GetX()  + binormal * tangentSample.GetY() + normal * tangentSample.GetZ();

            vec2 uv = texture::DecartToEquirectangularProjection(sampleVec.GetX(), sampleVec.GetY(), sampleVec.GetZ());
            
            for (int comp = 0; comp < n; comp++)
              irradianseData[cur * n + comp] += texture::Sample(uv, data, W, H, n, comp) * cos(theta) * sin(theta);
          }
        }

        for (int comp = 0; comp < n; comp++)
          irradianseData[cur * n + comp] = irradianseData[cur * n + comp] * (mth::PI / (float)(irradN1 * irradN2));
      }

  for (int i = 0; i < 6; i++)
  {
    stbi_write_hdr((path + "\\irr\\" + cubeFaces[i] + ".hdr").c_str(), irradianseW, irradianseH, n, (float*)(irradianseData + i * irradianseW * irradianseH * n));
  }
  stbi_image_free(data);

  delete[] irradianseData;
}
namespace IBLUtils
{
  float RadicalInverse_VdC(unsigned int bits)
  {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(float(bits) * 2.3283064365386963e-10); // / 0x100000000
  }

  vec2 Hammersley(unsigned int i, unsigned int N)
  {
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
  }

  vec ImportanceSampleGGX(vec2 Xi, vec norm, float roughness)
  {
    float a = roughness * roughness;

    float phi = 2.0f * mth::PI * Xi.GetX();
    float cosTheta = sqrt((1.0f - Xi.GetZ()) / (1.0f + (a * a - 1.0f) * Xi.GetZ()));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

    // from spherical coordinates to cartesian coordinates
    vec H = vec(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

    // from tangent-space vector to world-space sample vector
    vec up = abs(norm.GetZ()) < 0.999 ? vec(0.0, 0.0, 1.0) : vec(1.0, 0.0, 0.0);
    vec tangent = (norm % up).Normalize();
    vec bitangent = tangent % norm;

    vec sampleVec = tangent * H.GetX() + bitangent * H.GetY() + norm * H.GetZ();
    return sampleVec.Normalize();
  }

  float DistributionGGX(vec N, vec H, float roughness)
  {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(N & H, 0.0f);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = max(mth::PI * denom * denom, 0.0001f);

    if (nom == 0 && denom == 0)
      return 1.0;

    return nom / denom;
  }

  float GeometrySchlickGGX(float NdotV, float roughness)
  {
    float a = roughness;
    float k = (a * a) / 2.0f;

    float nom = NdotV;
    float denom = NdotV * (1.0f - k) + k;

    return nom / denom;
  }

  float GeometrySmith(vec N, vec V, vec L, float roughness)
  {
    float NdotV = max((N & V), 0.0f);
    float NdotL = max((N & L), 0.0f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
  }
}

VOID render::EvaluateBRDFLUT()
{
  float* data = new float[4 * brdfLUTW * brdfLUTH];

  for (int i = 0; i < brdfLUTH; i++)
  {
    for (int j = 0; j < brdfLUTW; j++)
    {
      vec2 value;
      float NdotV = 1.0f * (j + 0.5f) / brdfLUTW;
      float roughness = 1.0f * (i + 0.5f) / brdfLUTH;

      vec prefilteredColor;
      float totalWeight = 0;

      vec V(sqrt(1.0f - NdotV * NdotV), 0.0f, NdotV);

      float A = 0.0;
      float B = 0.0;

      vec N(0.0, 0.0, 1.0);

      for (int k = 0; k < brdfLUTSampleCount; k++)
      {
        vec2 Xi = IBLUtils::Hammersley(k, brdfLUTSampleCount);
        vec H = IBLUtils::ImportanceSampleGGX(Xi, N, roughness);
        vec L = !(H * 2.0 * (V & H) - V);

        float NdotL = max(L.GetZ(), 0.0f);
        float NdotH = max(H.GetZ(), 0.0f);
        float VdotH = max((H & V), 0.0f);

        if (NdotL > 0.0f)
        {
          float G = IBLUtils::GeometrySmith(N, V, L, roughness);
          float G_Vis = (G * VdotH) / (NdotH * NdotV);
          float Fc = pow(1.0f - VdotH, 5.0f);

          A += (1.0f - Fc) * G_Vis;
          B += Fc * G_Vis;
        }
      }

      A /= float(brdfLUTSampleCount);
      B /= float(brdfLUTSampleCount);

      data[i * brdfLUTW * 4 + j * 4 + 0] = A;
      data[i * brdfLUTW * 4 + j * 4 + 1] = B;
      data[i * brdfLUTW * 4 + j * 4 + 2] = 0;
      data[i * brdfLUTW * 4 + j * 4 + 3] = 1;
    }
  }

  stbi_write_hdr("BIN\\Textures\\brdfLUT.hdr", brdfLUTW, brdfLUTH, 4, data);
  delete[] data;
}

VOID render::EvaluatePrefiltered(std::string filename)
{
  size_t p = filename.find_last_of('\\');
  std::string path = filename.substr(0, p);
  std::string cubeFaces[] = { "posx", "negx", "posy", "negy", "posz", "negz" };
  INT imageW, imageH, n;
  float* data = stbi_loadf(filename.c_str(), &imageW, &imageH, &n, 0);
  const int NumOfMips = 5;
  if (!data)
  {
    MessageBox(NULL, "Failed to load equirectangular image", "ERROR", MB_OK);
    return;
  }

  for (int mip = NumOfMips - 1; mip >= 0; mip--)
  {
    int prefilteredMipW = prefilteredW >> mip;
    int prefilteredMipH = prefilteredH >> mip;
    float roughness = 1.0f * mip / NumOfMips;
    float* prefilteredData = new float[6 * prefilteredMipH * prefilteredMipW * n];

    for (int face = 0; face < 6; face++)
      for (int y = 0; y < prefilteredMipH; y++)
        for (int x = 0; x < prefilteredMipW; x++)
        {
          vec view;
          switch (face)
          {
          case 0:
            view = !vec(1.0, -(2.0 * float(y) / prefilteredMipH - 1.0), 2.0 * float(x) / prefilteredMipW - 1.0);
            break;
          case 1:
            view = !vec(-1.0, -(2.0 * float(y) / prefilteredMipH - 1.0), -(2.0 * float(x) / prefilteredMipW - 1.0));
            break;
          case 2:
            view = !vec(2.0 * float(x) / prefilteredMipW - 1.0, 1.0, -(2.0 * float(y) / prefilteredMipH - 1.0));
            break;
          case 3:
            view = !vec(2.0 * float(x) / prefilteredMipW - 1.0, -1.0, 2.0 * float(y) / prefilteredMipH - 1.0);
            break;
          case 4:
            view = !vec(2.0 * float(x) / prefilteredMipW - 1.0, -(2.0 * float(y) / prefilteredMipH - 1.0), -1.0);
            break;
          case 5:
            view = !vec(-(2.0 * float(x) / prefilteredMipW - 1.0), -(2.0 * float(y) / prefilteredMipH - 1.0), 1.0);
            break;
          }
          vec norm = view;

          vec prefilteredColor;
          float totalWeight = 0;

          for (int k = 0; k < prefilteredSampleCount; k++)
          {
            vec2 Xi = IBLUtils::Hammersley(k, prefilteredSampleCount);
            vec H = IBLUtils::ImportanceSampleGGX(Xi, view, roughness);
            vec L = !(H * 2.0 * (view & H) - view);

            float ndotl = max((norm & L), 0.0f);
            float ndoth = max((norm & H), 0.0f);
            float hdotv = max((H & view), 0.0f);

            float D = IBLUtils::DistributionGGX(norm, H, roughness);
            float pdf = (D * ndoth / (4.0f * hdotv)) + 0.0001f;

            float resolution = 512.0f; // resolution of source cubemap (per face)
            float saTexel = 4.0f * mth::PI / (6.0f * resolution * resolution);
            float saSample = 1.0f / (float(prefilteredSampleCount) * pdf + 0.0001f);

            float mipLevel = roughness == 0.0f ? 0.0f : 0.5f * log2(saSample / saTexel);

            int down_mip = (int)floor(mipLevel);
            int up_mip = (int)ceil(mipLevel);
            float a = mipLevel - down_mip;

            if (ndotl > 0.0)
            {
              vec2 uv = texture::DecartToEquirectangularProjection(L.GetX(), L.GetY(), L.GetZ());

              for (int comp = 0; comp < n; comp++)
                prefilteredColor[comp] +=
                (texture::SampleLevel(uv, data, imageW, imageH, n, comp, down_mip) * (1 - a) +
                  texture::SampleLevel(uv, data, imageW, imageH, n, comp, up_mip) * (a)) * ndotl;
              totalWeight += ndotl;
            }
          }

          int cur = face * prefilteredMipW * prefilteredMipH + y * prefilteredMipW+ x;
          for (int comp = 0; comp < n; comp++)
            prefilteredData[cur * n + comp] = prefilteredColor[comp] / totalWeight;
        }
    for (int i = 0; i < 6; i++)
    {
      stbi_write_hdr((path + "\\pref\\" + std::to_string(mip) + "\\" + cubeFaces[i] + ".hdr").c_str(), prefilteredMipW, prefilteredMipH, n, (float*)(prefilteredData + i * prefilteredMipW * prefilteredMipH * n));
    }

    delete[] prefilteredData;
  }
  stbi_image_free(data);
}

VOID render::SetEnviroment(std::string filename)
{
  if (!FileExists("BIN\\Textures\\HDRI\\" + filename + "\\posx.hdr"))
  {
    texture::ProcessEquirectangularImage("BIN\\Textures\\HDRI\\" + filename + "\\" + filename + ".hdr");
  }
  
  sharedTextures.enviromentTex = new texture("BIN\\Textures\\HDRI\\" + filename + "\\*.hdr", FALSE, texture::tex_type::TEX_TYPE_CUBE);
  
  if (!FileExists("BIN\\Textures\\HDRI\\" + filename + "\\irr\\posx.hdr"))
  {
    CreateDirectoryA(("BIN\\Textures\\HDRI\\" + filename + "\\irr").c_str(), NULL);
    EvaluateIrradiance("BIN\\Textures\\HDRI\\" + filename + "\\" + filename + ".hdr");
  }

  sharedTextures.irradianceMap = new texture("BIN\\Textures\\HDRI\\" + filename + "\\irr\\*.hdr", FALSE, texture::tex_type::TEX_TYPE_CUBE);

  if (!FileExists("BIN\\Textures\\brdfLUT.hdr"))
  {
    EvaluateBRDFLUT();
  }

  sharedTextures.brdfLUT = new texture("BIN\\Textures\\brdfLUT.hdr", FALSE, texture::tex_type::TEX_TYPE_PLAIN);

  if (!FileExists("BIN\\Textures\\HDRI\\" + filename + "\\pref\\0\\posx.hdr"))
  {
    CreateDirectoryA(("BIN\\Textures\\HDRI\\" + filename + "\\pref").c_str(), NULL);
    for (int i = 0; i < 5; i++)
      CreateDirectoryA(("BIN\\Textures\\HDRI\\" + filename + "\\pref\\" + std::to_string(i)).c_str(), NULL);
    EvaluatePrefiltered("BIN\\Textures\\HDRI\\" + filename + "\\" + filename + ".hdr");
  }

  sharedTextures.prefilteredMap = new texture("BIN\\Textures\\HDRI\\" + filename + "\\pref\\*.hdr", FALSE, texture::tex_type::TEX_TYPE_CUBE_WITH_MIPS);
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
texture* render::AddTexture(const std::string& Name, BOOL srgb, BOOL isCube)
{
  texture* tex;
  if ((tex = Textures.Find(Name)) != NULL)
    return tex;
  Textures.Add(tex = new texture(Name, srgb, (isCube ? texture::tex_type::TEX_TYPE_CUBE : texture::tex_type::TEX_TYPE_PLAIN)));
  return tex;
}

material* render::AddMaterial(const std::string& Name,
  const color3& Albedo, const FLT& Roughness,
  FLT Metalness, FLT Trans,
  shader* Shd, texture* Tex0, texture* Tex1)
{
  material* mtl;
  if ((mtl = Materials.Find(Name)) != NULL)
  {
    mtl->MaterialData.Albedo = Albedo;
    mtl->MaterialData.Roughness = Roughness;
    mtl->MaterialData.Metalness = Metalness;
    mtl->MaterialData.Trans = Trans;
    mtl->Shd = Shd;
    mtl->Tex[0] = Tex0;
    mtl->Tex[1] = Tex1;
    return mtl;
  }
  Materials.Add(mtl = new material(Name, Albedo, Roughness, Metalness, Trans, Shd, Tex0, Tex1));
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


