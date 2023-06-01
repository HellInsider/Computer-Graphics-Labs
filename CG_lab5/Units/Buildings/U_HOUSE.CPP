#include "u_house.h"
#include "../../Anim/anim.h"

house::house( anim *Ani, const vec &position, const std::string &FileName ) : LastUpdTime(Ani->GetTime())
{
  BOOL f = sph.Load(FileName, matr::MatrScaling(vec(1.0)).MatrTranslate(position));
  f = trees.Load("BIN/Models/trees lo-poly.g3dm", matr::MatrScaling(vec(0.001)).MatrTranslate(position));
 // if (f == TRUE)
 //   ;
  Pos[0] = vec(0, -3.0, 0);
  Pos[1] = vec(10, 0, 15);
  Pos[2] = vec(0, 0, 5);
  Pos[3] = vec(5, 0, 5);
  Pos[4] = vec(10, 0, 0);

  material* mtl = Ani->Materials.Find("tree");
  if (mtl == NULL)
  {
    mtl = Ani->AddMaterial("tree", vec(0.0, 1.0, 0.3), 0.99, 0.0, 0.0, Ani->Shaders.Find("def"));
  }
  for (auto &pr : trees.Prims)
    pr->Mtl = mtl;
}

VOID house::Response( anim *Ani )
{
}

VOID house::Render( anim *Ani )
{
///  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  sph.Draw(matr::MatrTranslation(Pos[0]));
  trees.Draw(matr::MatrTranslation(Pos[1]));
}
