/* FILE NAME   : U_SKY.H
 * PURPOSE     : sky class declaration module
 * PROGRAMMER  : AK5a.
 * LAST UPDATE : 26.07.2017
 */

#ifndef __U_SKY_H_
#define __U_SKY_H_

#include "unit.h"
#include "prim.h"

/* Working namespace of AK5a */
namespace ragl
{
  class sky : public ragl::unit
  {
  public:
    ragl::prim sph;

    /* Class constructor.
     * ARGUMENT:
     *   - animation interface pointer:
     *       ragl::anim *Ani;
     */
    sky( ragl::anim *Ani );

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


  }; /* End of 'sky' class */
} /* End of 'ragl' namespace */

#endif // __U_SKY_H_

/* END OF 'U_SKY.H' FILE */
