#ifndef __PRIM_H_
#define __PRIM_H_

#include <vector>
#include <string>

#include "def.h"


class material;
namespace topology
{
  class base;
} 

class prim
{
private:
public:
  /* Vertex representation type */
  struct VERTEX
  {
    vec  P;  /* Vertex position */
    vec2 T;  /* Vertex texture coordinates */
    vec  N;  /* Normal at vertex */
    color C;  /* Vertex color */

    VERTEX( VOID )
    {
    } 

    VERTEX( const vec &NewP,
            const vec2 &NewT = vec2(0, 0),
            const vec &NewN = vec(0, 1, 0),
            const color &NewC = color(1) ) :
      P(NewP), T(NewT), N(NewN), C(NewC)
    {
    } 
  };

  enum type
  {
    TRIMESH,  /* Set of triangles */
    STRIP     /* Strip of triangles */
  };

  struct transform
  {
    raw_matr MatrWVP;
    raw_matr MatrWorld;
    raw_matr MatrWorldInvTrans;
  };

  type Type;   /* Primitive type */
  ID3D11InputLayout* VertexLayout = nullptr;
  UINT VA;     /* Primitive vertex array */
  ID3D11Buffer* VertexBuffer = nullptr;
  ID3D11Buffer* IndexBuffer = nullptr;
  ID3D11Buffer* TransformConstBuf = nullptr;
  INT NumOfElements; /* Number of elements for OpenGL */
  matr Transform; /* Primitive transformation matrix */
  material *Mtl; /* Material pointer */
  vec
    Min,         /* Minimal primitive position */
    Max,         /* Maximal primitive position */
    Center,      /* Central primitive position */
    StartMin,    /* Minimal position in begin */
    StartMax,    /* Maximal position in begin */
    StartCenter; /* Central position in begin */
  INT Id, Id2; /* Primitive Ids */

  prim( const topology::base &T );

  prim(VOID)
  {
  }

  ~prim( VOID );

  VOID Draw( const matr &World, BOOL ExplicitShader = 0, BOOL MeasureTime = 0 );
}; 

class primitives
{
public:
  matr Transform;
  std::vector<prim *> Prims;

  primitives( VOID );

  ~primitives( VOID );

  VOID AutoCenter( FLT Size = 13 )
  {
    if (Prims.size() == 0)
      return;
    vec
      minv = Prims[0]->Min, maxv = Prims[0]->Max;

    for (auto p : Prims)
      minv = minv.Min(p->Min), maxv = maxv.Max(p->Max);
    vec sizev = maxv - minv, center = (minv + maxv) / 2;
    ///FLT scale = mth::Max(sizev[0], mth::Max(sizev[1], sizev[2]));
    ///Transform = matr::MatrTranslation(-center) * matr::MatrScaling(vec(Size / scale));
  } 

  VOID EvaluateCenters( VOID )
  {
    for (int i = 0; i < Prims.size(); i++)
    {
      Prims[i]->Min = Prims[i]->StartMin * Transform;
      Prims[i]->Max = Prims[i]->StartMax * Transform;
      Prims[i]->Center = (Prims[i]->Max + Prims[i]->Min) / 2;
    }
  } 

  static BOOL IsCollision( primitives &p1, primitives &p2 )
  {
    p1.EvaluateCenters();
    p2.EvaluateCenters();
    for (int i = 0; i < p1.Prims.size(); i++)
      for (int j = 0; j < p2.Prims.size(); j++)
        if ((p1.Prims[i]->Center - p2.Prims[j]->Center).VecLen() <= 
                (p1.Prims[i]->Max - p1.Prims[i]->Center).VecLen() + 
                (p2.Prims[j]->Max - p2.Prims[j]->Center).VecLen())
          return TRUE;
    return FALSE;
  }

  VOID Draw( const matr &World );

  BOOL Load( const std::string &FileName, const matr &Transform );
};

namespace topology
{
  /* Base topology class */
  class base
  {
    friend class prim;
  public:
    prim::type Type = prim::TRIMESH;

    /* Vertex array */
    std::vector<prim::VERTEX> V;
    /* Index array */
    std::vector<INT> I;

    base( VOID );

    base( prim::type NewType,
          const std::vector<prim::VERTEX> &NewV = {},
          const std::vector<INT> &NewI = {} );
  }; 

  class trimesh : public base
  {
  public:
    trimesh( VOID );

    trimesh( const std::vector<prim::VERTEX> &NewV,
             const std::vector<INT> &NewI = {} );

    trimesh& EvalNormals( VOID );
  };

  class grid : public base
  {
  protected:
    /* Grid size */
    INT W, H;

  public:
    grid( VOID )
    {
    }

    grid( INT NewW, INT NewH );

    INT GetW( VOID ) const
    {
      return W;
    } 

    INT GetH( VOID ) const
    {
      return H;
    } 

    prim::VERTEX * operator[]( INT Row )
    {
      assert(Row >= 0 && Row < H);
      return &V[Row * W];
    } 

    grid & EvalNormals( VOID );
  };

  class sphere : public grid
  {
  public:
    sphere( VOID )
    {
    } 

    sphere( const vec &Center, FLT R, INT NewW = 50, INT NewH = 23 );
  }; 

  class box : public base
  {
  public:
    box( VOID )
    {
    }

    box( const vec &p1, const vec &p2 );
  };

  class plane : public grid
  {
  public:
    plane( VOID )
    {
    }

    plane( const vec &Corner, const vec &Du, const vec &Dv, INT NewW = 50, INT NewH = 50 ) :
      grid(NewW, NewH)
    {
      vec N = Du % Dv;
      N.Normalize();

      /* Setup vertices */
      auto p = V.begin();
      for (INT i = 0; i < H; i++)
        for (INT j = 0; j < W; j++, p++)
        {
          p->N = N;
          p->P = Corner + Du * j / (W - 1.0) + Dv * i / (H - 1.0);
        }
    }
  }; 

  class screenspace_quad : public plane
  {
  public:
    screenspace_quad() : plane(vec(-1, -1, 0), vec(0, 2, 0), vec(2, 0, 0), 2, 2)
    {
    }
  };
}

#endif 
