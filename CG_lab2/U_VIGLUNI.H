#ifndef __U_VIGLUNI_H_
#define __U_VIGLUNI_H_

#include "unit.h"
#include "prim.h"

/* Working namespace of AK5a */
namespace ragl
{
  class vigluni : public ragl::unit
  {
  public:
    /* Unit position */
    vec Pos;
    ragl::primitives sph;

    vigluni( ragl::anim *Ani, const ragl::vec &position = ragl::vec(0) );

    VOID Response( ragl::anim *Ani );

    VOID Render( ragl::anim *Ani );

  }; 

} 

#endif // __U_VIGLUNI_H_