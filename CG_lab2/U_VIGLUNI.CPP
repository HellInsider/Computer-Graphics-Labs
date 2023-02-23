
#include "u_vigluni.h"
#include "anim.h"

ragl::vigluni::vigluni( ragl::anim *Ani, const ragl::vec &position ) : unit(), Pos(position)
{
  std::vector<BYTE> img((std::istreambuf_iterator<char>(std::ifstream("bin/textures/pyatack.g24"))),
                        std::istreambuf_iterator<char>());
  INT
    w = img[0] + (img[1] << 8),
    h = img[2] + (img[3] << 8);
  ragl::material * mtl = Ani->Materials.Find("default");
  mtl->Tex[0] = Ani->AddTexture("bin/textures/pyatack.g24", w, h, 3, &img[4]);
  
  BOOL f = sph.Load("elephant.g3dm", matr::MatrScaling(vec(0.003)));
  if (f == TRUE)
    ;
} 

VOID ragl::vigluni::Response( ragl::anim *Ani )
{
  ///Ani->Cam.SetView(ragl::vec(5, 3 * sin(Ani->GetTime()) + 4, 2 * cos(Ani->GetTime() * 4) + 3), ragl::vec(0));
  ///Ani->Cam.SetView(ragl::vec(3 * (FLT)Ani->GetCoord('Z') / 500), ragl::vec(0));
} 

VOID ragl::vigluni::Render( ragl::anim *Ani )
{
 ///// glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  ///glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  ///ragl::prim pri(ragl::topology::trimesh({ragl::vec(0), ragl::vec(1, 0, 0), ragl::vec(0, 1, 0),
  ///                                        ragl::vec(0), ragl::vec(0, 0, 1), ragl::vec(0, 1, 0)}));
  ///pri.Draw(ragl::matr::MatrRotation(Ani->GetTime() * 78, ragl::vec(0, 1, 0)));

  ///ragl::prim sph(ragl::topology::sphere(Pos, 1 + sin(Ani->GetTime() * 10)));
  sph.Draw(matr::MatrTranslation(Pos));

///  for (INT i = -3; i <= 3; i++)
///    for (INT j = -3; j <= 3; j++)
///      sph.Draw(ragl::matr::MatrScaling(ragl::vec(sin(Ani->GetTime() * 0.8) * 0.023 + 0.08)) *
///               ragl::matr::MatrTranslation(ragl::vec(i, fabs(sin(Ani->GetTime() * 3 + i + 13 * j)), j)));
} 
