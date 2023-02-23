/* FILE NAME   : U_TARAKAN.CPP
 * PURPOSE     : tarakan class implementation module
 * PROGRAMMER  : AK5a.
 * LAST UPDATE : 25.07.2017
 */

#include "u_tarakan.h"
#include "anim.h"

INT ragl::tarakan::Taracounter = 0;

/* Class constructor.
 * ARGUMENT:
 *   - animation interface pointer:
 *       ragl::anim *Ani;
 */
ragl::tarakan::tarakan( ragl::anim *Ani, const ragl::vec2 &position ) : unit(), 
                                      Pos(position), 
                                      Dir(!vec2(0, -1)),
                                      OldDir(Dir), NewDir(Dir),
                                      Speed(0), Id(Taracounter++), TimeLastUpd(Ani->GetTime()), OldPos(position)
{
  DBL tarasize = 0.01 * sqrt(sqrt(sqrt(Id)));
  INT sign = (tarasize > 0.3)? 1 : (Id % 2 - 0.5) * 2;
  BOOL f = sph.Load("tarakan.g3dm", matr::MatrScaling(vec(0.25 + sign * tarasize)));
  if (f == TRUE)
    ;
    sph.Prims[24]->Mtl = Ani->AddMaterial("tarakan", vec(0.0), vec(0.0, 0.0, 0.1), vec(0.001), 80, 0, Ani->AddShader("shaders\\tarakan"));

} /* End of 'my_unit' function */

/* Interframe animation response function.
 * ARGUMENT:
 *   - animation interface pointer:
 *       anim *Ani;
 * RETURNS: None.
 */
VOID ragl::tarakan::Response( ragl::anim *Ani )
{
  static DBL TimeShift;
  static INT Last;
  static DBL UpdateTime = 0.3;
  srand(Ani->GetTime() * Pos.GetX() * 1000);
  if (Speed < 0.1)
    Speed += (((rand() % 258) / 129.0) - 1) / 10;
  else
    Speed -= (((rand() % 258) / 129.0) - 1) / 10;
 // if (Dir == !vec2(0, -1))
  if (Ani->GetTime() - TimeLastUpd > UpdateTime)
  {
    TimeLastUpd = Ani->GetTime();
    OldPos = Pos;
    OldDir = NewDir;
    NewDir = !(OldDir + (!vec2(rand() % 100 - 50, rand() % 100 - 50)));
  }
  
  {
    double alpha = max((Ani->GetTime() - TimeLastUpd) / UpdateTime, 0);
    Dir = !(NewDir * alpha + OldDir * (1 - alpha));
  }
  vec2 DirTrue = Pos - OldPos;
 
  Pos += Dir * Ani->GetDeltaTime() * 2;
  if (Pos.VecLen() > 50)
    Pos = vec2(1, 1);
  sph.Transform = matr::MatrRotationY(-vec::RadianToDegree(vec2::AngleVecPlane(vec2(-1, 0), Dir))).MatrTranslate(vec(Pos.GetX(), 0.3, Pos.GetZ()));
  //if (Ani->GetTime() - TimeShift > 10)
  //{
  //  if (Last == Taracounter)
  //    TimeShift = Ani->GetTime();
  //  Last = Id;
  //  if (Speed < 1)
  //  {
  //    Speed += ((rand() % 258) / 129.0) - 1;
  //  }
  //  else
  //    Speed -= ((rand() % 258) / 129.0) - 1; //Speed -= (((rand() / 129) % 2) - 1) / 5;
  //  Dir = !(Dir + vec(Speed / 5, 0, Speed / 5));
  //  Pos += vec(Dir.GetX() * Speed / 10000000.0, 0, Dir.GetZ() * Speed / 10000000.0) * 0.00003;
  //  if (Pos.VecLen() > 99)
  //    Pos = vec(1, 0, 1);
  //  sph.Transform = sph.Transform.MatrRotateY(vec::RadianToDegree(vec::AngleVec(vec(1, 0, 0), Dir))).MatrTranslate(Pos);
  //}
} /* End of 'Response' function */

/* Render unit function.
 * ARGUMENT:
 *   - animation interface pointer:
 *       anim *Ani;
 * RETURNS: None.
 */
VOID ragl::tarakan::Render( ragl::anim *Ani )
{
  ///glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  sph.Draw(matr());
} /* End of 'Render' function */

/* END OF 'U_TARAKAN.CPP' FILE */