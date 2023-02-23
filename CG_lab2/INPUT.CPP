#include "input.h"

ragl::input::input( HWND hWnd, INT MouseWheel ) : keyboard(), mouse(hWnd, MouseWheel), joystick()
{

} 

VOID ragl::input::Response( INT MouseWheel )
{
  keyboard::Response();
  mouse::Response(MouseWheel);
  joystick::Response();
} 

ragl::keyboard::keyboard( VOID )
{
  memset(Keys, 0, 256 * sizeof(BYTE));
  memset(KeysOld, 0, 256 * sizeof(BYTE));
  memset(KeysClick, 0, 256 * sizeof(BOOL));
} 

VOID ragl::keyboard::Response( VOID )
{
   GetKeyboardState(Keys);   
   for (INT i = 0; i < 256; i++)   
   {     
     Keys[i] >>= 7;
     KeysClick[i] = Keys[i] && !KeysOld[i];
   }  
   memcpy(KeysOld, Keys, 256);
} 

ragl::mouse::mouse( HWND hWnd, INT MouseWheel ) : hWnd(hWnd), 
                                                  Mx(0), My(0), Mz(0), Mdx(0), Mdy(0), Mdz(MouseWheel)
{
  memset(Buttons, 0, 3 * sizeof(BYTE));
  memset(ButtonsOld, 0, 3 * sizeof(BYTE));
  memset(ButtonsClick, 0, 3 * sizeof(BOOL));
} 

VOID ragl::mouse::Response( INT MouseWheel )
{
  /* Mouse buttons handle */
  BYTE tmpKeys[256];
  GetKeyboardState(tmpKeys); 
  Buttons[0] = tmpKeys[VK_LBUTTON] >> 7;
  Buttons[1] = tmpKeys[VK_RBUTTON] >> 7;
  Buttons[2] = tmpKeys[VK_MBUTTON] >> 7;

  for (INT i = 0; i < 3; i++)
    ButtonsClick[i] = Buttons[i] && !ButtonsOld[i];

  memcpy(ButtonsOld, Buttons, 3);

  /* Mouse axis handle */
  POINT pt; 
 
  GetCursorPos(&pt);   
  ScreenToClient(hWnd, &pt);   
  Mdx = pt.x - Mx;  
  Mdy = pt.y - My;   
  Mx = pt.x;   
  My = pt.y; 
  Mdz = MouseWheel;   
  Mz += MouseWheel; 
} 

INT ragl::mouse::GetCoord( CHAR a )
{
  switch (a)
  {
  case 'Y':
    return My;
  case 'Z':
    return Mz;
  default:
    return Mx;
  }
} 

INT ragl::mouse::GetDeltaCoord( CHAR a )
{
  switch (a)
  {
  case 'Y':
    return Mdy;
  case 'Z':
    return Mdz;
  default:
    return Mdx;
  }
} 

/**********************/
/* _____Joystick_____ */
/*____________________*/

/* Joystick axis */
#define GET_JOYSTICK_AXIS(A) \
          (2.0 * (ji.dw##A##pos - jc.w##A##min) / (jc.w##A##max - jc.w##A##min) - 1)

ragl::joystick::joystick( VOID )
{

}

VOID ragl::joystick::Response( VOID )
{
  /*** Joystick handle ***/
  if (joyGetNumDevs() > 0)
  {
    JOYCAPS jc;
 
    /* Get joystick info */
    if (joyGetDevCaps(JOYSTICKID1, &jc, sizeof(JOYCAPS)) == JOYERR_NOERROR)
    {
      JOYINFOEX ji; 

      ji.dwSize = sizeof(JOYINFOEX);
      ji.dwFlags = JOY_RETURNALL;
      if (joyGetPosEx(JOYSTICKID1, &ji) == JOYERR_NOERROR)
      {
        /* Axes */
        Jx = GET_JOYSTICK_AXIS(X);
        Jy = GET_JOYSTICK_AXIS(Y);
        Jz = GET_JOYSTICK_AXIS(Z);
        Jr = GET_JOYSTICK_AXIS(R);
 
        /* Buttons */         
        for (int i = 0; i < 32; i++)         
        {           
          JBut[i] = (ji.dwButtons >> i) & 1;           
          JButClick[i] = JBut[i] && !JButOld[i];           
          memcpy(JButOld, JBut, 32);         
        } 
 
        /* Point of view */         
        JPov = ji.dwPOV == 0xFFFF ? 0 : ji.dwPOV / 4500 + 1;       
      }     
    }   
  }
} 
