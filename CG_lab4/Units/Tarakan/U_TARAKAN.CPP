#include "u_tarakan.h"
#include "../../Anim/anim.h"

INT tarakan::Taracounter = 0;

tarakan::tarakan( anim *Ani, const vec2 &position ) : unit(), 
                                      Pos(position), 
                                      Dir(!vec2(0, -1)),
                                      OldDir(Dir), NewDir(Dir),
                                      Speed(0), Id(Taracounter++), TimeLastUpd(Ani->GetTime()), OldPos(position)
{
  DBL tarasize = 0.01 * sqrt(sqrt(sqrt(Id)));
  INT sign = (tarasize > 0.3)? 1 : (Id % 2 - 0.5) * 2;
  BOOL f = sph.Load("BIN/Models/tarakan.g3dm", matr::MatrScaling(vec(0.25 + sign * tarasize)));
  //if (f == TRUE)
  //  ;
    sph.Prims[24]->Mtl = Ani->AddMaterial("tarakan", vec(0.0, 0.0, 0.1), 0.99, 1.0, 0, Ani->AddShader("shaders\\tarakan"));

}

VOID tarakan::Response( anim *Ani )
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
}

VOID tarakan::Render( anim *Ani )
{
  ///glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  sph.Draw(matr());
}

