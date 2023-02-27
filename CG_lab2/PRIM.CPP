#include "prim.h"
#include "anim.h"

/**********************/
/* Prim class methods */
/*____________________*/

prim::prim( const topology::base &T ) :
  Type(T.Type), Transform(matr()), NumOfElements(T.I.size()), Mtl(NULL), Id(0)
{
  /* Collect min-max information */
  Min = Max = T.V[0].P;
  for (auto vrt : T.V)
    Min = vrt.P.Min(Min), Max = vrt.P.Max(Max);
  StartCenter = Center = (Min + Max) / 2;
  StartMax = Max;
  StartMin = Min;

  D3D11_BUFFER_DESC bd;
  ZeroMemory(&bd, sizeof(bd));
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(VERTEX) * T.V.size();
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;
  D3D11_SUBRESOURCE_DATA InitData;
  ZeroMemory(&InitData, sizeof(InitData));
  InitData.pSysMem = &T.V[0];

  render* rnd = anim::GetAnim();
  HRESULT hr = rnd->GetDriver()->CreateBuffer(&bd, &InitData, &VertexBuffer);
  if (FAILED(hr))
    MessageBox(NULL, "Failed to create vertex buf", "ERROR", MB_OK);

  // Set vertex buffer
  UINT stride = sizeof(VERTEX);
  UINT offset = 0;
  rnd->GetDriverIterface()->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);

  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(UINT) * T.I.size();
  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd.CPUAccessFlags = 0;
  InitData.pSysMem = &T.I[0];
  hr = rnd->GetDriver()->CreateBuffer(&bd, &InitData, &IndexBuffer);
  if (FAILED(hr))
    MessageBox(NULL, "Failed to create index buf", "ERROR", MB_OK);

  NumOfElements = T.I.size();

  // Set index buffer
  rnd->GetDriverIterface()->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

  // Set primitive topology
  rnd->GetDriverIterface()->IASetPrimitiveTopology(Type == TRIMESH ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

  bd.ByteWidth = sizeof(transform);
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.CPUAccessFlags = 0;
  hr = rnd->GetDriver()->CreateBuffer(&bd, nullptr, &TransformConstBuf);
  if (FAILED(hr))
    MessageBox(NULL, "Failed to create transform constant buf", "ERROR", MB_OK);

}

prim::~prim(VOID)
{
  if (VertexBuffer) VertexBuffer->Release();
  if (IndexBuffer) IndexBuffer->Release();
  if (TransformConstBuf) TransformConstBuf->Release();

  if (VertexLayout) VertexLayout->Release();    //IDD
}

VOID prim::Draw(const matr& World, BOOL ExplicitShader, BOOL MeasureTime)
{
    render* rnd = anim::GetAnim();
    transform tr;
    matr MatrWorld = (Transform * World);
    matr MatrWVP = MatrWorld * rnd->Cam.GetMatrVP();
    matr MatrWorldInvTrans = (!MatrWorld).MatrTranspose();

    tr.MatrWorld.fromPtr((float*)MatrWorld);
    tr.MatrWVP.fromPtr((float*)MatrWVP);
    tr.MatrWorldInvTrans.fromPtr((float*)MatrWorldInvTrans);

    rnd->GetDriverIterface()->UpdateSubresource(TransformConstBuf, 0, nullptr, &tr, 0, 0);
    rnd->GetDriverIterface()->VSSetConstantBuffers(CONST_BUF_SLOT_TRANSFORM, 1, &TransformConstBuf);

    if (!ExplicitShader)
    {
        material* mtl = Mtl;
        if (mtl == NULL)
            mtl = rnd->Materials.Find("default");
        assert(mtl != NULL);
        mtl->Apply();
    }

    /* Activete primitive vertex array */
    UINT stride = sizeof(VERTEX);
    UINT offset = 0;
    rnd->GetDriverIterface()->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
    rnd->GetDriverIterface()->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    rnd->GetDriverIterface()->IASetPrimitiveTopology(Type == TRIMESH ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    rnd->GetDriverIterface()->DrawIndexed(NumOfElements, 0, 0);
};


/****************************/
/* Primitives class methods */
/*__________________________*/

primitives::primitives( VOID ) : Transform(matr())
{
} 

primitives::~primitives( VOID )
{
  for (int i = 0; i < Prims.size(); i++)
    delete Prims[i];
  Prims.clear();
} 

VOID primitives::Draw( const matr &World )
{
  INT i;

  // Draw solid bodyes
  i = 0;
  for (auto p : Prims)
  {
    p->Id = i++;
    p->Id2 = Prims.size();
    material *mtl = p->Mtl;
    if (mtl != NULL)
      if (mtl->MaterialData.Trans != 1)
        continue;
    p->Draw(Transform * World);
  }
/*  glGetIntegerv(GL_CULL_FACE_MODE, &save);
  is = glIsEnabled(GL_CULL_FACE);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);*/
  for (auto p : Prims)
  {
    p->Id = i++;
    p->Id2 = Prims.size();
    material *mtl = p->Mtl;
    if (mtl == NULL || mtl->MaterialData.Trans == 1)
        continue;
    p->Draw(Transform * World);
  }

//  glCullFace(GL_BACK);
  for (auto p : Prims)
  {
    p->Id = i++;
    p->Id2 = Prims.size();
    material *mtl = p->Mtl;
    if (mtl == NULL || mtl->MaterialData.Trans == 1)
        continue;
    p->Draw(Transform * World);
  }
} 

BOOL primitives::Load( const std::string &FileName, const matr &Transform )
{
  FILE *F;
  DWORD Sign;
  INT NoofP = 0, NoofM = 0, NoofT = 0, i;

  /* Open file */
  if ((F = fopen(FileName.c_str(), "rb")) == NULL)
    return FALSE;
  /* Check signature */
  fread(&Sign, 4, 1, F);
  if (Sign != *(DWORD *)"G3DM")
  {
    fclose(F);
    return FALSE;
  }

  /* Read quantities */
  fread(&NoofP, 4, 1, F);
  fread(&NoofM, 4, 1, F);
  fread(&NoofT, 4, 1, F);

  Prims.resize(NoofP);

  /* Primitive material indices */
  std::vector<INT> prims_mtl;
  prims_mtl.resize(NoofP);

  /* Load primitives */
  for (i = 0; i < NoofP; i++)
  {
    INT nv = 0, ni = 0, j;
    std::vector<prim::VERTEX> V;
    std::vector<INT> I;

    fread(&nv, 4, 1, F);
    fread(&ni, 4, 1, F);
    fread(&prims_mtl[i], 4, 1, F);
    V.resize(nv);
    I.resize(ni);

    fread(&V[0], 1, sizeof(prim::VERTEX) * nv, F);
    fread(&I[0], 1, sizeof(INT) * ni, F);

    for (j = 0; j < nv; j++)
    {
      V[j].P = V[j].P.PointTransform(Transform);
      V[j].N = V[j].N.NormalTransform(Transform);
    }
    for (int k = 0; k < ni; k += 3)
      std::swap(I[k], I[k + 2]);
    Prims[i] = new prim(topology::trimesh(V, I));
  }

  /* Load materials */
  struct STORE_MATERIAL
  {
    CHAR Name[300]; /* Material name */

    /* Illumination coefficients */ 
    color3 Ka, Kd, Ks;                 /* Ambient, diffuse, specular coefficients */
    FLT Ph;                            /* Phong power coefficient */
    FLT Trans;                         /* Transparency factor */
    INT Tex[8];                        /* Texture references */

    /* Shader string */
    CHAR ShaderStr[300];
    INT Shader;                        /* Texture references */
  };

  std::vector<material *> mtls;
  std::vector<STORE_MATERIAL> store_mtls;
  mtls.resize(NoofM);
  store_mtls.resize(NoofM);
  if (NoofM > 0)
  {
    fread(&store_mtls[0], sizeof(STORE_MATERIAL), NoofM, F);
    for (INT i = 0; i < NoofM; i++)
      mtls[i] = anim::GetAnim()->AddMaterial(store_mtls[i].Name,
        store_mtls[i].Ka, store_mtls[i].Kd, store_mtls[i].Ks,
        store_mtls[i].Ph, store_mtls[i].Trans, NULL, NULL, NULL);
  }
  /* Load textures */
  std::vector<texture *> texs;
  texs.resize(NoofT);
  for (i = 0; i < NoofT; i++)
  {
    CHAR Name[300];
    INT W, H, C;
    BYTE *mem;

    fread(Name, 1, 300, F);
    fread(&W, 4, 1, F);
    fread(&H, 4, 1, F);
    fread(&C, 4, 1, F);
    mem = new BYTE[W * H * C];
    fread(mem, C, W * H, F);
    texs[i] = anim::GetAnim()->AddTexture(Name, W, H, C, mem);
    delete[] mem;
  }
  fclose(F);

  /* Correct material texture references */
  for (INT i = 0; i < NoofM; i++)
  {
    if (store_mtls[i].Tex[0] != -1)
      mtls[i]->Tex[0] = texs[store_mtls[i].Tex[0]];
    if (store_mtls[i].Tex[1] != -1)
      mtls[i]->Tex[1] = texs[store_mtls[i].Tex[1]];
  }

  /* Correct primitives material references */
  for (INT i = 0; i < NoofP; i++)
    if (prims_mtl[i] != -1)
      Prims[i]->Mtl = mtls[prims_mtl[i]];   

  //texs.clear(); //IDD
  //mtls.clear();// both didn't work

  return TRUE;   
}

/**********************/
/* Base class methods */
/*____________________*/

topology::base::base( VOID )
{
} 

topology::base::base( prim::type NewType,
      const std::vector<prim::VERTEX> &NewV,
      const std::vector<INT> &NewI ) : Type(NewType), V(NewV), I(NewI)
{
} 

topology::trimesh::trimesh( VOID )
{
} 

topology::trimesh::trimesh( const std::vector<prim::VERTEX> &NewV,
         const std::vector<INT> &NewI ) : topology::base(prim::TRIMESH, NewV, NewI)
{
} 

topology::trimesh& topology::trimesh::EvalNormals( VOID )
{
  /* Set to zero all normals */
  for (auto &v : V)
    v.N = vec(0);

  /* Evaluate all facets normals */
  for (INT i = 0; i < I.size(); i += 3)
  {
    prim::VERTEX *p0 = &V[I[i]], *p1 = &V[I[i + 1]], *p2 = &V[I[i + 2]];
    vec N = (p1->P - p0->P) % (p2->P - p0->P);
    N.Normalize();

    /* Disperse normal to all three points */
    p0->N += N;
    p1->N += N;
    p2->N += N;
  }

  /* Normalize all normals */
  for (auto &v : V)
    v.N.Normalize();
  return *this;

} 

topology::grid::grid( INT NewW, INT NewH ) : W(NewW), H(NewH), base(prim::STRIP)
{
  /* Setup vertices */
  V.resize(W * H, prim::VERTEX(vec(0)));
  /* Setup vertices */
  auto p = V.begin();
  for (INT i = 0; i < H; i++)
    for (INT j = 0; j < W; j++, p++)
      p->T = vec2(j / (W - 1.0), i / (H - 1.0));

  /* Setup indices */
  I.resize((W * 2 + 1) * (H - 1));
  for (INT k = 0, v = 0, i = 0; i < H - 1; i++)
  {
    for (INT j = 0; j < W; j++, v++)
    {
      I[k++] = v + W;
      I[k++] = v;
    }
    I[k++] = -1;
  }
} 

topology::grid & topology::grid::EvalNormals( VOID )
{
  /* Set to zero all normals */
  for (auto &v : V)
    v.N = vec(0);

  /* Evaluate all facets normals */
  for (INT k = 0, i = 0; i < H - 1; i++)
    for (INT j = 0; j < W - 1; j++, k++)
    {
      prim::VERTEX
        *p00 = &V[k],
        *p10 = &V[k + W],
        *p01 = &V[k + 1],
        *p11 = &V[k + W + 1];
      vec
        N1 = -((p00->P - p10->P) % (p11->P - p10->P)),
        N2 = -((p11->P - p01->P) % (p00->P - p01->P));
      N1.Normalize();
      N2.Normalize();

      /* Disperse normal to all three points */
      p10->N += N1;
      p00->N += N1;
      p11->N += N1;

      p00->N += N2;
      p01->N += N2;
      p11->N += N2;
    }

  /* Normalize all normals */
  for (auto &v : V)
    v.N.Normalize();
  return *this;
} 

topology::sphere::sphere( const vec &Center, FLT R, INT NewW, INT NewH ) : grid(NewW, NewH)
{
  /* Setup vertices */
  auto p = V.begin();
  INT i, j;
  FLT theta, phi;
  for (theta = 0, i = 0; i < H; i++, theta += mth::PI / (H - 1))
    for (phi = 0, j = 0; j < W; j++, phi += 2 * mth::PI / (W - 1), p++)
    {
      p->N = vec(sin(theta) * sin(phi), cos(theta), sin(theta) * cos(phi));
      p->P = Center + p->N * R;
    }
}

topology::box::box( const vec& p1, const vec& p2 )
{
  Type = prim::TRIMESH;
  V.resize(24, prim::VERTEX(vec(0)));
  FLT p1x = p1.GetX(), p1y = p1.GetY(), p1z = p1.GetZ();
  FLT p2x = p2.GetX(), p2y = p2.GetY(), p2z = p2.GetZ();

  prim::VERTEX vertices[] =
  {
      { vec(p1x, p2y, p1z), vec2(1.0f, 0.0f), vec(0.0f, 1.0f, 0.0f), color(0.0) },
      { vec(p2x, p2y, p1z), vec2(0.0f, 0.0f), vec(0.0f, 1.0f, 0.0f), color(0.0) },
      { vec(p2x, p2y, p2z), vec2(0.0f, 1.0f), vec(0.0f, 1.0f, 0.0f), color(0.0) },
      { vec(p1x, p2y, p2z), vec2(1.0f, 1.0f), vec(0.0f, 1.0f, 0.0f), color(0.0) },

      { vec(p1x, p1y, p1z), vec2(0.0f, 0.0f), vec(0.0f, -1.0f, 0.0f), color(0.0) },
      { vec(p2x, p1y, p1z), vec2(1.0f, 0.0f), vec(0.0f, -1.0f, 0.0f), color(0.0) },
      { vec(p2x, p1y, p2z), vec2(1.0f, 1.0f), vec(0.0f, -1.0f, 0.0f), color(0.0) },
      { vec(p1x, p1y, p2z), vec2(0.0f, 1.0f), vec(0.0f, -1.0f, 0.0f), color(0.0) },

      { vec(p1x, p1y, p2z), vec2(0.0f, 1.0f), vec(-1.0f, 0.0f, 0.0f), color(0.0) },
      { vec(p1x, p1y, p1z), vec2(1.0f, 1.0f), vec(-1.0f, 0.0f, 0.0f), color(0.0) },
      { vec(p1x, p2y, p1z), vec2(1.0f, 0.0f), vec(-1.0f, 0.0f, 0.0f), color(0.0) },
      { vec(p1x, p2y, p2z), vec2(0.0f, 0.0f), vec(-1.0f, 0.0f, 0.0f), color(0.0) },

      { vec(p2x, p1y, p2z), vec2(1.0f, 1.0f), vec(1.0f, 0.0f, 0.0f), color(0.0) },
      { vec(p2x, p1y, p1z), vec2(0.0f, 1.0f), vec(1.0f, 0.0f, 0.0f), color(0.0) },
      { vec(p2x, p2y, p1z), vec2(0.0f, 0.0f), vec(1.0f, 0.0f, 0.0f), color(0.0) },
      { vec(p2x, p2y, p2z), vec2(1.0f, 0.0f), vec(1.0f, 0.0f, 0.0f), color(0.0) },

      { vec(p1x, p1y, p1z), vec2(0.0f, 1.0f), vec(0.0f, 0.0f, -1.0f), color(0.0) },
      { vec(p2x, p1y, p1z), vec2(1.0f, 1.0f), vec(0.0f, 0.0f, -1.0f), color(0.0) },
      { vec(p2x, p2y, p1z), vec2(1.0f, 0.0f), vec(0.0f, 0.0f, -1.0f), color(0.0) },
      { vec(p1x, p2y, p1z), vec2(0.0f, 0.0f), vec(0.0f, 0.0f, -1.0f), color(0.0) },

      { vec(p1x, p1y, p2z), vec2(1.0f, 1.0f), vec(0.0f, 0.0f, 1.0f), color(0.0f) },
      { vec(p2x, p1y, p2z), vec2(0.0f, 1.0f), vec(0.0f, 0.0f, 1.0f), color(0.0f) },
      { vec(p2x, p2y, p2z), vec2(0.0f, 0.0f), vec(0.0f, 0.0f, 1.0f), color(0.0f) },
      { vec(p1x, p2y, p2z), vec2(1.0f, 0.0f), vec(0.0f, 0.0f, 1.0f), color(0.0f) }
  };
  V.assign(vertices, vertices + 24);

  INT indices[] =
  {
      3,0,1,
      2,3,1,

      6,5,4,
      7,6,4,

      11,8,9,
      10,11,9,

      14,13,12,
      15,14,12,

      19,16,17,
      18,19,17,

      22,21,20,
      23,22,20
  };
  I.resize(36);
  I.assign(indices, indices + 36);

}

