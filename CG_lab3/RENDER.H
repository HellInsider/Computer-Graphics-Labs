#ifndef __RENDER_H_
#define __RENDER_H_

#include <chrono>
#include "DEF.H"
#include "CAMERA.H"
#include "RES.H"
#include "PRIM.H"


enum const_buf_slots
{
  CONST_BUF_SLOT_COMMON,
  CONST_BUF_SLOT_TRANSFORM,
  CONST_BUF_SLOT_MATERIAL,
  CONST_BUF_SLOT_SCREENSPACE,
  CONST_BUF_SLOT_LIGHT
};

enum texture_slots
{
  TEXTURE_MATERIAL_SLOT_0,
  TEXTURE_MATERIAL_SLOT_1,
  TEXTURE_SCREENSPACE_SLOT,
  TEXTURE_CONSTANTS_SLOT,
  TEXTURE_CONSTANTS_SLOT_2,
  k_NUM_OF_SLOTS
};

struct common_cb_data
{
  raw_matr MatrView;
  raw_matr MatrProj;
  vec CamLoc;
  float DeltaTime;
};

const UINT MAX_LIGHTS = 16;
struct light_cb_data
{
  raw_light RawLights[MAX_LIGHTS];
  uint32_t NumLighs;
  uint32_t __padding[3];
};

class render
{
private:
  HWND hWnd;        // Work window handle
  HDC hDC;          //Work window device context
  UINT SX, SY;
  ID3D11Device* Driver = nullptr;
  ID3D11DeviceContext* DriverIterface = nullptr;
  IDXGISwapChain* SwapChain = nullptr;
  ID3D11RenderTargetView* RenderTargetView = nullptr;
  ID3D11Texture2D* ColorPassTex = nullptr;
  ID3D11ShaderResourceView* ColorPassShadResView = nullptr;
  ID3D11RenderTargetView* ColorPassRTView = nullptr;
  ID3D11Texture2D* DepthStencilTex = nullptr;
  ID3D11DepthStencilView* DepthStencilView = nullptr;
  ID3D11Buffer* CommonConstBuf = nullptr;
  ID3D11Buffer* LightConstBuf = nullptr;
  D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_NULL;
  D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_11_0;
  ID3D11BlendState* BlendStateOpaqe = nullptr, * BlendStateTransparent = nullptr;
  ID3D11SamplerState* DefaultLinearSampler;
  ID3D11Query* TimeQuery = nullptr;
  ID3DUserDefinedAnnotation* Annotation = nullptr;
  ID3D11Debug* debug;
  BOOL timed = false;
  std::chrono::high_resolution_clock::time_point t;
  UINT FrameNum = 0;
  long double duration = 0;

  struct measure_luminance_system
  {
    ID3D11Texture2D* LuminanceTex[6];      // 6th is for old frame lum
    ID3D11RenderTargetView* LuminanceRT[6];
    ID3D11ShaderResourceView* LuminanceSRV[6];
    ID3D11Buffer* ScreenSpaceConstBuf = nullptr;
    shader ShdEvalLogLum, ShdDownsample, ShdFinal;
    prim quad;

    enum luninance_adress
    {
      LUM_SCREEN_SIZE = 0,
      LUM_FIRST_DOWNSAMPLE,
      LUM_SECOND_DOWNSAMPLE,
      LUM_THIRD_DOWNSAMPLE,
      LUM_RESULT,
      LUM_OLD_FRAME
    };

    struct screenspace_cb_data
    {
      FLT sxInv, syInv;
      UINT sx, sy;
    };

    static const D3D_SHADER_MACRO SHADER_CONSTS[][2];

    measure_luminance_system( render *Rnd );
    ~measure_luminance_system( VOID );
    VOID Draw( render* Rnd, ID3D11ShaderResourceView *src );
    VOID Resize( INT w, INT h );
  };

  measure_luminance_system* SysMeasureLum;

  shader *ShdTonemap;
  prim *ScreenQuad;

  VOID InitSizeDependentResources( VOID );

public:
  camera Cam;       //Current camera

  resource_manager<shader> Shaders;
  resource_manager<texture> Textures;
  resource_manager<material> Materials;
  std::vector<light *> Lights;

  render( HWND hwnd );

  VOID Resize( INT w, INT h );

  VOID FrameStart( VOID );

  VOID FrameCopy( VOID );

  VOID FrameEnd( VOID );

  ~render( VOID );

  VOID SetTransparency(BOOL enable)
  {
    float blendFactorO[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float blendFactorT[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
    UINT sampleMask = 0xffffffff;

    if (enable)
      DriverIterface->OMSetBlendState(BlendStateTransparent, blendFactorT, sampleMask);
    else
      DriverIterface->OMSetBlendState(BlendStateOpaqe, blendFactorO, sampleMask);
  }

  ID3D11Device* GetDriver( VOID ) { return Driver; }
  ID3D11DeviceContext* GetDriverIterface(VOID) { return DriverIterface; }
  ID3D11Query* GetTimeQuery(VOID) { return TimeQuery; }
  ID3D11SamplerState* const* GetDefaultLinearSampler( VOID ) { return &DefaultLinearSampler; }

  shader * AddShader( const std::string &FileNamePrefix );

  texture * AddTexture( const std::string &Name, INT W, INT H, INT BytesPerPixel, BYTE *Pixels );

  material * AddMaterial( const std::string &Name,
                          const color3 &Ka = color3(0.1), const color3 &Kd = color3(0.9), const color3 &Ks = color3(0.0),
                          FLT Ph = 30, FLT Trans = 0,
                          shader *Shd = NULL, texture *Tex0 = NULL, texture *Tex1 = NULL );

  light * AddLight( const light::light_type type, const color col, const vec posDir, const vec atten );
};

#endif 
