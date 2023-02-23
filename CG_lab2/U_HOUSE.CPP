/* FILE NAME   : U_HOUSE.CPP
 * PURPOSE     : house class implementation module
 * PROGRAMMER  : AK5a.
 * LAST UPDATE : 25.07.2017
 */

#include "u_house.h"
#include "anim.h"

/* Class constructor.
 * ARGUMENT:
 *   - animation interface pointer:
 *       ragl::anim *Ani;
 *   - unit start position 
 *       const ragl::vec &position = ragl::vec(0);
 */
ragl::house::house( ragl::anim *Ani, const ragl::vec &position, const std::string &FileName ) : LastUpdTime(Ani->GetTime())
{
//  BOOL f = sph.Load(FileName, matr::MatrScaling(vec(1.0)).MatrTranslate(position));
  BOOL f = sph.Load(FileName, matr::MatrScaling(vec(1.0)).MatrTranslate(position));
  f = trees.Load("trees lo-poly.g3dm", matr::MatrScaling(vec(0.001)).MatrTranslate(position));
  if (f == TRUE)
    ;
//  Pos[0] = vec(0, -3.0, 0);
  Pos[0] = vec(0, -3.0, 0);
  Pos[1] = vec(10, 0, 15);
  Pos[2] = vec(0, 0, 5);
  Pos[3] = vec(5, 0, 5);
  Pos[4] = vec(10, 0, 0);

  ragl::material* mtl = Ani->Materials.Find("tree");
  if (mtl == NULL)
  {
    mtl = Ani->AddMaterial("tree", vec(0.0), vec(0.0, 1.0, 0.3), vec(1.0, 1.0, 0.2), 80, 1.0, Ani->Shaders.Find("def"));
  }
  for (auto &pr : trees.Prims)
    pr->Mtl = mtl;

}

/* Interframe animation response function.
 * ARGUMENT:
 *   - animation interface pointer:
 *       anim *Ani;
 * RETURNS: None.
 */
VOID ragl::house::Response( ragl::anim *Ani )
{
  static int updTime = 1;
  vec Dir = !vec(-0.0, -0.2, -1.0);
  if (Ani->GetTime() - LastUpdTime > updTime * 0.1)
  {
    LastUpdTime = Ani->GetTime();
    vec NewPos = vec(10.0 + (rand() % 100 - 50) / 50.0, 3.0 + (rand() % 100 - 50) / 100.0, 14.0 + (rand() % 100 - 50) / 50.0);
    bubbles.push_back(new prim(topology::sphere::sphere(NewPos, 0.1 + (rand() % 100 - 50) / 5000.0)));
    ragl::material* mtl = Ani->Materials.Find("bubble");
    if (mtl == NULL)
    {
      mtl = Ani->AddMaterial("bubble", vec(0.05), vec(0.8), vec(0.0, 0.8, 0.2), 80, 1.0, Ani->AddShader("shaders\\bubble"));
    }
    bubbles.back()->Mtl = mtl;

  }
  for (auto& bub : bubbles)
    bub->Transform = bub->Transform.MatrTranslate(Dir * Ani->GetDeltaTime() * 5);
}

/* Render unit function.
 * ARGUMENT:
 *   - animation interface pointer:
 *       anim *Ani;
 * RETURNS: None.
 */
VOID ragl::house::Render( ragl::anim *Ani )
{
///  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  sph.Draw(matr::MatrTranslation(Pos[0]));
  trees.Draw(matr::MatrTranslation(Pos[1]));
  Ani->SetTransparency(TRUE);
  for (auto& bub : bubbles)
    bub->Draw(matr());
  Ani->SetTransparency(FALSE);

}



/* END OF 'U_HOUSE.CPP' FILE */