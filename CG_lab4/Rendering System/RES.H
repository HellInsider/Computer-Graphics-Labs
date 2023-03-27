#ifndef __RES_H_
#define __RES_H_

#include <ostream>
#include <istream>
#include <fstream>
#include <map>
#include <string>

#include "../DEF.H"

typedef vec color3;

/* Resource manager class */
template<class type>
  class resource_manager
  {
  public:
    /* Resource stock */
    std::map<std::string, type *> Stock;

    resource_manager & Add( type *Res )
    {
      Stock[Res->Name] = Res;
      return *this;
    } 

    ~resource_manager( void )
    {
      for (auto r : Stock)
        delete r.second;
    } 

    void Clear( void )
    {
      for (auto r : Stock)
        delete r.second;
      Stock.clear();
    }

    type * Find( const std::string &Name )
    {
      if (Stock.count(Name) == 0)
        return NULL;
      return Stock[Name];
    } 
  }; 

/* Shader class */
class shader
{
private:
  VOID Log(const std::string& Stage, const std::string& Text)
  {
    std::ofstream("SV{SHAD}30.LOG", std::ios_base::app) << Stage << ":" << Text << "\n";
  } 

  std::string LoadTextFile(const std::string& FileName)
  {
    return std::string((std::istreambuf_iterator<char>(std::ifstream(FileName))),
      std::istreambuf_iterator<char>());
  } 

  HRESULT CompileShaderFromFile(const CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, const D3D_SHADER_MACRO* defines = nullptr);

  /* Shader name */
  const std::string PostfixVert = "_vsh.hlsl";
  const std::string PostfixPix = "_psh.hlsl";

  ID3D11VertexShader* VertexShader = nullptr;
  ID3D11PixelShader* PixelShader = nullptr;
  ID3D11InputLayout* InputLayout = nullptr;

public:
  std::string Name;
  shader(VOID)
  {
  } 

  shader(const std::string& FileNamePrefix, const D3D_SHADER_MACRO* defines = nullptr);

  ~shader(VOID)
  {
    if (InputLayout) InputLayout->Release();
    if (VertexShader) VertexShader->Release();
    if (PixelShader) PixelShader->Release();
  } 

  VOID Apply(VOID);
};


class texture
{
public:
  /* Texture name */
  std::string Name;

  enum tex_type
  {
    TEX_TYPE_PLAIN,
    TEX_TYPE_CUBE
  } Type;

  /* Image size */
  INT W, H;

  ID3D11ShaderResourceView* TextureSRV = nullptr;
  ID3D11SamplerState* Sampler= nullptr;
  ID3D11Texture2D* Texture = nullptr;
  ID3D11Texture2D* TextureCubeFaces[6] = { nullptr };

  texture( VOID )
  {
  } 

  texture( const std::string &FileName, BOOL srgb = FALSE, tex_type NewType = TEX_TYPE_PLAIN );

  texture(const std::string& NewName, INT NewW, INT NewH, INT BytesPerPixel, BYTE* Pixels);

  ~texture( VOID )
  {
    if (Sampler) Sampler->Release();
    if (TextureSRV) TextureSRV->Release();
    if (Texture) Texture->Release();
    for (auto& t : TextureCubeFaces)
      if (t) t->Release();
  } 

  VOID InitGpuRes( INT NewW, INT NewH, INT BytesPerPixel, BYTE** Pixels, BOOL srgb = FALSE, tex_type NewType = TEX_TYPE_PLAIN);
  VOID Apply( UINT slot );
  static VOID ApplyNull( UINT slot );
}; 

class material
{
public:
  std::string Name;

  struct material_constants
  {
    /* Illumination coefficients (anbient, diffuse, specular) */
    color3 Albedo;
    FLT Roughness;
    FLT Metalness;
    FLT Trans;
    BOOL IsTex0, IsTex1;

  } MaterialData;
  /* Shader pointer */
  shader *Shd;
  /* Texture pointers */
  texture *Tex[2];

  ID3D11Buffer* ConstBuffer = nullptr;

  material( VOID )
  {
  } 

  material( const std::string &NewName,
            const color3 &NewAlbedo = color3(0.9), const FLT &NewRoughness = 0.99,
            FLT NewMetalness = 0.9, FLT NewTrans = 0,
            shader *NewShd = NULL, texture *NewTex0 = NULL, texture *NewTex1 = NULL );


  VOID Apply( VOID );

  ~material(VOID)
  {
    if (ConstBuffer) ConstBuffer->Release();
  }
}; 

class light
{
public:
  enum light_type
  {
    LIGHT_POINT,
    LIGHT_DIRECTION
  };

  light(light_type NewType, color NewCol, vec NewPos, vec NewDir, vec NewAtten) :
    Type(NewType), Color(NewCol), Pos(NewPos), Dir(NewDir), Attenuation(NewAtten)
  {
  }

  color Color;
  vec Pos; // not for direction light
  vec Dir; // not for point light
  vec Attenuation; // x - C0, y - C1, z - C2
  light_type Type;
};

struct raw_light
{
  color Color;
  vec PosDir;  // pos or dir depend on type
  int32_t Type;
  vec Atten;
  int32_t __paddings;
};

#endif /* __RES_H_ */
