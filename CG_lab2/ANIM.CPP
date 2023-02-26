#include "anim.h"
#include "list_units.h"


anim anim::Instance;

anim::anim( VOID ) : win(), render(win::hWnd), input(win::hWnd, win::MouseWheel), timer()
{
}

anim::~anim( VOID )
{
  Units.Walk([](unit *Uni){ delete Uni; });
}

anim * anim::GetAnim()
{
  return &Instance;
}

anim & anim::operator<<( unit *Uni )
{
  Units << Uni;
  return *this;
} 

VOID anim::Init( VOID )
{
  *this << new control(this, 0);
  IsInit = 1;
} 

VOID anim::Close( VOID )
{
} 

VOID anim::Resize( VOID )
{
  render::Resize(W, H);
}

VOID anim::Erase( HDC hDC )
{
}

VOID anim::Paint( HDC hDC )
{
} 

VOID anim::Timer( VOID )
{
  Idle();
} 

VOID anim::Activate( VOID )
{
} 

VOID anim::Idle( VOID )
{
  input::Response(win::MouseWheel);
  win::MouseWheel = 0;
  timer::Response();
  Units.Walk([this]( unit *Uni )
             {
               Uni->Response(this);
             });
  render::FrameStart();
  Units.Walk([this](unit *Uni)
             {
               Uni->Render(this);
             });
  render::FrameEnd();
  render::FrameCopy();
} 

