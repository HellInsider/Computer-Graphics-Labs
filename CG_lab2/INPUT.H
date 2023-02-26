#ifndef __INPUT_H_
#define __INPUT_H_

#include "DEF.H"
#include "MTHDEF.H"
#include <mmsystem.h>
#pragma comment(lib, "winmm") 

class keyboard
{
public:
  /* Keyboard data */
  BYTE Keys[256], KeysOld[256];
  BOOL KeysClick[256];

  keyboard( VOID );

  VOID Response( VOID );
};

#define RAGL_MOUSE_LEFT_BUTTON   0
#define RAGL_MOUSE_RIGHT_BUTTON  1
#define RAGL_MOUSE_MEDIUM_BUTTON 2

class mouse
{
protected:
  /* Mouse position data */
  INT Mx, My, Mz, Mdx, Mdy, Mdz;
  HWND hWnd;
public:
  /* Mouse buttons data */
  BYTE Buttons[3], ButtonsOld[3];
  BOOL ButtonsClick[3];

  mouse( HWND hWnd, INT MouseWheel );

  VOID Response( INT MouseWheel );

  INT GetCoord( CHAR a );

  INT GetDeltaCoord( CHAR a );
};

class joystick
{
protected:
  /* Joystick data */
  BYTE JBut[32],               /* Joystick button state */     
       JButOld[32],            /* Joystick button old state */     
       JButClick[32];          /* Joystick button click info */   
  INT JPov;                    /* Joystick point-of-view control [0,1..8] */
  DBL Jx, Jy, Jz, Jr;          /* Joystick axes */
public:

  joystick( VOID );

  VOID Response( VOID );
};

class input : public keyboard, public mouse, public joystick
{
protected:

public:

  input( HWND hWnd, INT MouseWheel );

  VOID Response( INT MouseWheel );
};

#endif // __INPUT_H_
