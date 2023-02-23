/* FILE NAME   : U_GROUND.H
 * PURPOSE     : ground class declaration module
 * PROGRAMMER  : AK5a.
 * LAST UPDATE : 25.07.2017
 */

#ifndef __U_GROUND_H_
#define __U_GROUND_H_

#include "unit.h"
#include "prim.h"

/* Working namespace of AK5a */
namespace ragl
{
  class ground : public ragl::unit
  {
  public:
    /* Unit position */
    vec Pos;
    ragl::prim sph;

    /* Class constructor.
     * ARGUMENT:
     *   - animation interface pointer:
     *       ragl::anim *Ani;
     */
    ground( ragl::anim *Ani );

    /* Interframe animation response function.
     * ARGUMENT:
     *   - animation interface pointer:
     *       anim *Ani;
     * RETURNS: None.
     */
    VOID Response( ragl::anim *Ani );

    /* Render unit function.
     * ARGUMENT:
     *   - animation interface pointer:
     *       anim *Ani;
     * RETURNS: None.
     */
    VOID Render( ragl::anim *Ani );


  }; /* End of 'ground' class */
} /* End of 'ragl' namespace */

#endif // __U_GROUND_H_

/* END OF 'U_GROUND.H' FILE */