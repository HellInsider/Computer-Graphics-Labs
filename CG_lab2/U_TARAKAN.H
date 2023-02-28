#ifndef __U_TARAKAN_H_
#define __U_TARAKAN_H_

#include "unit.h"
#include "prim.h"

class tarakan : public unit
{
public:
  /* Unit position */
  DBL Speed, TimeLastUpd;
  vec2 Pos, Dir, OldPos;
  vec2 OldDir;
  vec2 NewDir;
  primitives sph;
  INT Id;
  static INT Taracounter;

  tarakan( anim *Ani, const vec2 &position = vec2(0) );

  VOID Response( anim *Ani );

  VOID Render( anim *Ani );

};

#endif // __U_TARAKAN_H_

