#ifndef __U_HOUSE_H_
#define __U_HOUSE_H_

#include "../unit.h"
#include "../../Rendering System/prim.h"
class house : public unit
{
public:
  /* Unit position */
  vec Pos[5];
  DBL LastUpdTime;
  primitives sph, trees;
  std::string FileName;

  house( anim *Ani, const vec &position, const std::string &FileName );

  VOID Response( anim *Ani );

  VOID Render( anim *Ani );

};

#endif // __U_HOUSE_H_
