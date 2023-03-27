#ifndef __U_SKY_H_
#define __U_SKY_H_

#include "../unit.h"
#include "../../Rendering System/prim.h"

class sky : public unit
{
public:
  prim sph;

  sky( anim *Ani );

  VOID Response( anim *Ani );

  VOID Render( anim *Ani );


};

#endif // __U_SKY_H_

