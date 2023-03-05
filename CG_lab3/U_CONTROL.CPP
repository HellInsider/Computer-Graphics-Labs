
#include "u_control.h"
#include "u_tarakan.h"
#include "u_ground.h"
#include "u_house.h"
#include "u_spheresgrid.h"
#include "anim.h"

control::control( anim *Ani, UINT preset ) : unit()
{
  srand(Ani->GetGlobalTime());
  if (preset == 0)
  {
    Ani->Cam.SetView(vec(30, 1.8, 15), vec(3, 0.8, 3));
    *Ani << new ground(Ani);
    *Ani << new house(Ani, vec(0), "Building_A08.g3dm");
    *Ani << new spheresGrid(Ani, vec(20, 1, 15), 4);
    l1 = Ani->AddLight(light::LIGHT_POINT, color(1, 0, 0, 1), vec(10, 7, 0), vec(1, 0.0, 1.0));
    l2 = Ani->AddLight(light::LIGHT_POINT, color(0, 1, 0, 1), vec(10, 7, 10), vec(1, 0.07, 1.0));
    l3 = Ani->AddLight(light::LIGHT_POINT, color(0, 0, 1, 1), vec(7, 4, 10), vec(1, 0.07, 1.0));
  }
} 

inline int NextBright(int bright)
{
  if (bright == 1)
    return 10;
  if (bright == 10)
    return 100;
  return 1;
}

VOID control::Response( anim *Ani )
{
  static INT jumpCounter = 0;
  static FLT TimeShift = 0;
  vec L = Ani->Cam.GetLoc(), D = !(Ani->Cam.GetAt() - L), R = Ani->Cam.GetRight();
  /* Keyboard handle */
  if (Ani->KeysClick[VK_ESCAPE])
    Ani->OnDestroy();
  else if (Ani->KeysClick['F'])
    Ani->FlipFullScreen();
  else if (Ani->KeysClick['P'])
    Ani->SetPause();
  else if (Ani->KeysClick['T'])
    *Ani << new tarakan(Ani, vec2(rand() / 137 % 20, rand() / 137 % 20));
  else if (Ani->KeysClick['1'])
    l1->Color.R = NextBright(l1->Color.R);
  else if (Ani->KeysClick['2'])
    l2->Color.G = NextBright(l2->Color.G);
  else if (Ani->KeysClick['3'])
    l3->Color.B = NextBright(l3->Color.B);

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
  }
  /* Move camera */
    Ani->Cam.SetViewDir(L, D);
} 