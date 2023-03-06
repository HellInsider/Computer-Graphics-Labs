#ifndef __U_SPHERESGRID_H_
#define __U_SPHERESGRID_H_

#include "unit.h"
#include "prim.h"

class spheres_grid : public unit
{
public:
  /* Unit position */
  INT gridSize = 5;
  FLT radius = 0.5;
  FLT offset = 1;

  primitives spheres;

  spheres_grid(anim* Ani, const vec& position, INT gridSize);

  ~spheres_grid();

  VOID Response(anim* Ani);

  VOID Render(anim* Ani);

};

#endif