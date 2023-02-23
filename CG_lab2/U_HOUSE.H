/* FILE NAME   : U_HOUSE.H
 * PURPOSE     : house class declaration module
 * PROGRAMMER  : AK5a.
 * LAST UPDATE : 25.07.2017
 */

#ifndef __U_HOUSE_H_
#define __U_HOUSE_H_

#include "unit.h"
#include "prim.h"
/* Working namespace of AK5a */
namespace ragl
{
  class house : public ragl::unit
  {
  public:
    /* Unit position */
    vec Pos[5];
    DBL LastUpdTime;
    ragl::primitives sph, trees;
    std::vector<ragl::prim*> bubbles;
    std::string FileName;

    /* Class constructor.
     * ARGUMENT:
     *   - animation interface pointer:
     *       ragl::anim *Ani;
     *   - unit start position 
     *       const ragl::vec &position = ragl::vec(0);
     */
    house( ragl::anim *Ani, const ragl::vec &position, const std::string &FileName );

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

  }; /* End of 'house' class */
} /* End of 'ragl' namespace */

#endif // __U_HOUSE_H_

/* END OF 'U_HOUSE.H' FILE */
