#include "anim.h"

#include <string>
#include <vector>
#include <iostream>
#include <map>

#include <fstream>
#include <sstream>


INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInctance, CHAR *CmdLine, INT ShowCmd )
{
  anim *a = anim::GetAnim();
  a->Run();
  return 35;
}
