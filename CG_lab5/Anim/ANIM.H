#ifndef __ANIM_H_
#define __ANIM_H_

#include "../WIN.H"
#include "../Rendering System/RENDER.H"
#include "../Rendering System/PRIM.H"
#include "../Anim/TIMER.H"
#include "../Stock/STOCK.H"
#include "../Units/UNIT.H"
#include "../Anim/Input/INPUT.H"
#include "../Rendering System/RES.H"

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

#endif // __ANIM_H_


