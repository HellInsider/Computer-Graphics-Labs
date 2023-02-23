
////* Main window funktion */
////INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, 
////                    CHAR *CmdLine, INT ShowCmd )
////{
////  ragl::win w, ww, www, wwww, wwwww;
////  w.Run();
////} /* End of 'WinMain' function */
//
//#include "anim.h"
//
//#include <glut.h>
//
//#include <string>
//#include <vector>
//#include <iostream>
//#include <map>
//
//#include <fstream>
//#include <sstream>
//
///* The main program function.
// * ARGUMENTS:
// *   - handle of application instance:
// *       HINSTANCE hInstance;
// *   - dummy handle of previous application instance (not used):
// *       HINSTANCE hPrevInstance;
// *   - command line string:
// *       CHAR *CmdLine;
// *   - show window command parameter (see SW_***):
// *       INT CmdShow;
// * RETURNS:
// *   (INT) Error level for operation system (0 for success).
// */
//INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInctance, CHAR *CmdLine, INT ShowCmd )
//{
//  ragl::anim *a = ragl::anim::GetAnim();
//  ragl::vec p;
//  ragl::matr m = ragl::matr::MatrRotation(90, ragl::vec(0, 1, 0));
//
//  m = !m;
//
//  std::vector<INT> pe;
//
//  INT n = 13;
//  while (n-- > 0)
//    pe.push_back(rand() % 18);
//  std::vector<INT>::iterator i = pe.begin();
//  std::vector<INT>::iterator last = pe.end();
//  *i = 23;
//  for (std::vector<INT>::iterator i = pe.begin(); i != pe.end(); i++)
//    *i = 23;
//  for (auto i = pe.begin(); i != pe.end(); i++)
//    *i = 23;
//  for (auto i : pe)
//    std::cout << i << " ";
//  for (auto &i : pe)
//    i = 8;
//
//  std::map<std::string, INT> Age;
//  Age["Вася"] = 18;
//  Age["Арина"] = 14;
//  Age["Вася"]++;
//  
//  INT x1 = Age["Петя"];
//
//  struct foo
//  {
//    std::string Name;
//    FLT X;
//  };
//  std::map<std::string, foo> Skl;
//  foo pp = Skl["yoyo"];
//  INT nn;
//  if (nn = Skl.count("yoyo"))
//    std::cout << "aaa";
//  if (nn = Skl.count("фф"))
//    std::cout << "aaa";
//
//  /*
//  std::ifstream f("a.txt");
//  if (!f)
//    INT O  = 0;                 //не открылся
//  INT x;
//  while (!f.eof())             // пока не конец файла
//    f >> x;
//  f.close();
//  f.open("a.txt");
//  std::string s, line;
//  while (!f.eof())
//  {
//    std::getline(f, line);
//    s += line;
//  }
//  f.close();
//  
//  std::stringstream ss;
//  std::ifstream("a.txt").rdbuf();
//
//  //std::ifstream f1("a.txt");
//  std::string s1((std::istreambuf_iterator<char>(std::ifstream("a.txt"))), std::istreambuf_iterator<char>());
//  std::cout << s1;
//
//  auto sss = std::istreambuf_iterator<char>(std::ifstream("a.txt"));
//  std::string s2(sss, std::istreambuf_iterator<char>());
//  */
//  class my_unit : public ragl::unit
//  {
//    ragl::topology::sphere sph;
//    ragl::prim prSph;
//  public:
//    my_unit( VOID ) : sph(ragl::vec(0,0,0), 3), prSph(sph)
//    {
//    }
//    /* Interframe animation response function.
//     * ARGUMENT:
//     *   - animation interface pointer:
//     *       anim *Ani;
//     * RETURNS: None.
//     */
//    VOID Response( ragl::anim *Ani )
//    {
//      if (Ani->Keys[VK_ESCAPE])
//        Ani->OnDestroy();
//      Ani->Cam.SetView(ragl::vec(30), ragl::vec(0));
//    } /* End of 'Response' function */
//
//    /* Render unit function.
//     * ARGUMENT:
//     *   - animation interface pointer:
//     *       anim *Ani;
//     * RETURNS: None.
//     */
//    VOID Render( ragl::anim *Ani )
//    {
//      glLoadMatrixf((FLT *)&ragl::matr::MatrRotation(Ani->GetTime() * 78, ragl::vec(0, 1, 0) * Ani->Cam.GetMatrVP()));
//      glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
//      glEnable(GL_LIGHTING);
//      glEnable(GL_LIGHT0);
//      glEnable(GL_COLOR_MATERIAL);
//      glColor3d(0.9, 0.9, 0.8);
//      glutWireTeapot(5 * 0.23);
//      prSph.Draw(ragl::matr());
//    } /* End of 'Render' function */
//  }; /* End of 'my_unit' class */
//
//  *a << new my_unit() << new my_unit();
//
//  a->Run();
//  return 23;
//} /* End of 'WinMain' function */
//
///* END OF 'MAIN.CPP' FILE */

#include "anim.h"

#include <string>
#include <vector>
#include <iostream>
#include <map>

#include <fstream>
#include <sstream>


INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInctance, CHAR *CmdLine, INT ShowCmd )
{
  ragl::anim *a = ragl::anim::GetAnim();
  a->Run();
  return 35;
}
