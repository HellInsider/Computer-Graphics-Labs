
#include "u_control.h"
#include "u_tarakan.h"
#include "u_ground.h"
#include "u_house.h"
#include "u_sky.h"
#include "U_BUBBLE.H"
#include "U_RT_SCENE.H"
#include "anim.h"

ragl::control::control( ragl::anim *Ani, UINT preset ) : unit()
{
  srand(Ani->GetGlobalTime());
  if (preset == 0)
  {
    Ani->Cam.SetView(ragl::vec(30, 1.8, 15), ragl::vec(3, 0.8, 3));
    *Ani << new ground(Ani);
    //  *Ani << new sky(Ani);
 //   *Ani << new house(Ani, vec(0), "Building_A08.g3dm");
    *Ani << new house(Ani, vec(0), "Building_A08.g3dm");
    *Ani << new bubble(Ani, vec(3, 0.5, 3));
  }
  else
  {
    Ani->Cam.SetView(ragl::vec(5, 3, 8), ragl::vec(3.0, 2.5, 4.3));
    *Ani << new rt_scene(Ani);
 //   for (int i = 0; i < 10000; i++)
    *Ani << new bubble(Ani, vec(3.0, 2.5, 4.3));
    *Ani << new bubble(Ani, vec(0.0, 2.5, 5.0));
    *Ani << new bubble(Ani, vec(4.0, 4.5, 1.3));
    *Ani << new bubble(Ani, vec(5.0, 2.5, 2.3));
  }
} 

VOID ragl::control::Response( ragl::anim *Ani )
{
  ///prim p(topology::plane(vec(-50, 0, -50), vec(100, 0, 0), vec(0, 0, 100), 4, 4));
  ///p.Draw(matr());
  static INT jumpCounter = 0;
  static  FLT TimeShift = 0;
  vec L = Ani->Cam.GetLoc(), D = !(Ani->Cam.GetAt() - L), R = Ani->Cam.GetRight();
  /* Keyboard handle */
  if (Ani->KeysClick[VK_ESCAPE])
    Ani->OnDestroy();
  else if (Ani->KeysClick['F'])
    Ani->FlipFullScreen();
  else if (Ani->KeysClick['P'])
    Ani->SetPause();
  else if (Ani->KeysClick['V'])
  {
    vigluni *v = new vigluni(Ani, vec(rand() / 500 % 5, 0, 2));
    *Ani << v;
  }
  else if (Ani->KeysClick['T'])
    *Ani << new tarakan(Ani, vec2(rand() / 137 % 20, rand() / 137 % 20));

  if (Ani->KeysClick[VK_SPACE])
    if (!jumpCounter)
      jumpCounter = 8;
  if (Ani->Keys['W'])
    L += vec(D.GetX() / 10, 0, D.GetZ() / 10);
  else if (Ani->Keys['S'])
    L -= vec(D.GetX() / 19, 0, D.GetZ() / 19);
  if (Ani->Keys['A'])
    D = D * matr::MatrRotationY(40 * Ani->GetGlobalDeltaTime());
  else if (Ani->Keys['D'])
    D = D * matr::MatrRotationY(-40 * Ani->GetGlobalDeltaTime());
  if (Ani->Keys['Y'])
    D = D * matr::MatrRotation(-30 * Ani->GetGlobalDeltaTime(), R);
  else if (Ani->Keys['H'])
    D = D * matr::MatrRotation(30 * Ani->GetGlobalDeltaTime(), R);

  /* Jumping */
  if (jumpCounter)
  {
    if ((Ani->GetTime() - TimeShift) > 0.1 )
    {
      TimeShift = Ani->GetTime();
      jumpCounter--;
    }
    //L = vec(L.GetX(), 1.8 - pow(((FLT)(jumpCounter - 4) / 4), 2), L.GetZ());
  }
  /* Move camera */
  ///3 * (FLT)Ani->GetDeltaCoord('Z') / 500
    Ani->Cam.SetViewDir(L, D);
} 