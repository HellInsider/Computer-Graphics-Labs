/* FILE NAME   : U_TARAKAN.H
 * PURPOSE     : tarakan class declaration module
 * PROGRAMMER  : AK5a.
 * LAST UPDATE : 25.07.2017
 */

#ifndef __U_TARAKAN_H_
#define __U_TARAKAN_H_

#include "unit.h"
#include "prim.h"

/* Working namespace of AK5a */
namespace ragl
{
  class tarakan : public ragl::unit
  {
  public:
    /* Unit position */
    DBL Speed, TimeLastUpd;
    vec2 Pos, Dir, OldPos;
    vec2 OldDir;
    vec2 NewDir;
    ragl::primitives sph;
    INT Id;
    static INT Taracounter;

    /* Class constructor.
     * ARGUMENT:
     *   - animation interface pointer:
     *       ragl::anim *Ani;
     *   - unit start position (default vec(0, 0, 0))
     *       const ragl::vec &position = ragl::vec(0);
     */
    tarakan( ragl::anim *Ani, const ragl::vec2 &position = ragl::vec2(0) );

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

  }; /* End of 'tarakan' class */

} /* End of 'ragl' namespace */


#endif // __U_TARAKAN_H_

/* END OF 'U_TARAKAN.H' FILE */
