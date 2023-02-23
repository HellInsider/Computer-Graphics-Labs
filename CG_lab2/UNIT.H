#ifndef __UNIT_H_
#define __UNIT_H_

#include "DEF.H"

/* Working namespace of AK5a */
namespace ragl
{
  /* Forward declarations */
  class anim;

  /* Unit base class */
  class unit
  {
  public:
    unit( VOID )
    {
    }

    virtual ~unit( VOID )
    {
    }

    virtual VOID Response( anim *Ani )
    {
    }

    virtual VOID Render( anim *Ani )
    {
    }
  }; 

} 

#endif // __UNIT_H_
