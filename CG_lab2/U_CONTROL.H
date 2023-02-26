#ifndef __U_CONTROL_H_
#define __U_CONTROL_H_

#include "unit.h"

class control : public unit
{
public:

  control( anim *Ani, UINT preset = 0 );

  VOID Response( anim *Ani );

};

#endif // __U_CONTROL_H_


