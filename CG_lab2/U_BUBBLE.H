
#ifndef __U_BUBBLE_H_
#define __U_BUBBLE_H_

#include "unit.h"
#include "prim.h"

 /* Working namespace of AK5a */
namespace ragl
{
  class bubble : public ragl::unit
  {
  public:
    /* Unit position */
    vec2 Pos;
    ragl::prim sph;

    bubble(ragl::anim* Ani, const ragl::vec& position = ragl::vec(0));

    VOID Response(ragl::anim* Ani);


    VOID Render(ragl::anim* Ani);

  }; 

}


#endif // __U_BUBBLE_H_


