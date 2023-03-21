
#include "u_control.h"
#include "u_tarakan.h"
#include "u_ground.h"
#include "u_house.h"
#include "u_spheresgrid.h"
#include "u_sky.h"
#include "anim.h"

control::control( anim *Ani, UINT preset ) : unit()
{
  srand(Ani->GetGlobalTime());
  if (preset == 0)
  {
    Ani->Cam.SetView(vec(5, 4.8, 10), vec(5, 4.8, 0));

    *Ani << new spheres_grid(Ani, vec(0, 1, 0), 10);
    *Ani << new spheres_grid(Ani, vec(10, 7, 10), 1);
    *Ani << new sky(Ani);
    l1 = Ani->AddLight(light::LIGHT_POINT, color(100, 100, 100, 1), vec(10, 7, 10), vec(1, 0.0, 1.0));
  }
} 


VOID control::Response( anim *Ani )
{
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
    Ani->SetLightingMode(LIGHT_MODE_DEFAULT);
  else if (Ani->KeysClick['2'])
    Ani->SetLightingMode(LIGHT_MODE_DISTRIBUTION);
  else if (Ani->KeysClick['3'])
    Ani->SetLightingMode(LIGHT_MODE_GEOMETRY);
  else if (Ani->KeysClick['4'])
    Ani->SetLightingMode(LIGHT_MODE_FRESNEL);

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

  /* Move camera */
    Ani->Cam.SetViewDir(L, D);
} 
