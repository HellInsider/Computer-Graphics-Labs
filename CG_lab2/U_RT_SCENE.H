
#ifndef __U_RT_SCENE_H_
#define __U_RT_SCENE_H_

#include "unit.h"
#include "prim.h"

/* Working namespace of AK5a */
namespace ragl
{
  class rt_scene : public ragl::unit
  {
  public:
    /* Unit position */
    ragl::prim *spheres[7];
    ragl::prim *cube;
    ragl::prim *planes[2];

    rt_scene(ragl::anim* Ani);

    VOID Response(ragl::anim* Ani);

    VOID Render(ragl::anim* Ani);

  }; 

} 


#endif 
