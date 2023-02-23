#ifndef __U_CONTROL_H_
#define __U_CONTROL_H_

#include "unit.h"
#include "u_vigluni.h"

/* Working namespace of AK5a */
namespace ragl
{
  class control : public ragl::unit
  {
  public:
  
    control( ragl::anim *Ani, UINT preset = 0 );

    VOID Response( ragl::anim *Ani );

  };
} 

#endif // __U_CONTROL_H_


