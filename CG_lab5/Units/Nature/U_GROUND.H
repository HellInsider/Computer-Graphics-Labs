#ifndef __U_GROUND_H_
#define __U_GROUND_H_

#include "../unit.h"
#include "../../Rendering System/prim.h"

class ground : public unit
{
public:
  /* Unit position */
  vec Pos;
  prim sph;

  ground( anim *Ani );

  VOID Response( anim *Ani );

  VOID Render( anim *Ani );


};

#endif // __U_GROUND_H_
