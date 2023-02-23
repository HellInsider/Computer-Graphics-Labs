#ifndef __ANIM_H_
#define __ANIM_H_

#include "win.h"
#include "render.h"
#include "prim.h"
#include "timer.h"
#include "stock.h"
#include "unit.h"
#include "input.h"
#include "res.h"

/* Working namespace of AK5a */
namespace ragl
{
  class anim : public win, public render, public input, public timer
  {
  private:
    static anim Instance;
    stock<unit *> Units; 

    anim( VOID );
  public:
    ~anim( VOID );

    static anim * GetAnim();

    anim & operator<<( unit *Uni );

    VOID Init( VOID );

    VOID Close( VOID );

    VOID Resize( VOID );

    VOID Erase( HDC hDC );

    VOID Paint( HDC hDC );

    VOID Timer( VOID );

    VOID Activate( VOID );

     VOID Idle( VOID );
  };
} 

#endif // __ANIM_H_


