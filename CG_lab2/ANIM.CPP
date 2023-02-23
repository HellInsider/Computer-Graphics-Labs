#include "anim.h"
#include "list_units.h"


ragl::anim ragl::anim::Instance;

ragl::anim::anim( VOID ) : win(), render(win::hWnd), input(win::hWnd, win::MouseWheel), timer()
{
}

ragl::anim::~anim( VOID )
{
  Units.Walk([](unit *Uni){ delete Uni; });
}

ragl::anim * ragl::anim::GetAnim()
{
  return &Instance;
}

ragl::anim & ragl::anim::operator<<( ragl::unit *Uni )
{
  Units << Uni;
  return *this;
} 

VOID ragl::anim::Init( VOID )
{
  *this << new control(this, 0);
  IsInit = 1;
} 

VOID ragl::anim::Close( VOID )
{
} 

VOID ragl::anim::Resize( VOID )
{
  render::Resize(W, H);
}

VOID ragl::anim::Erase( HDC hDC )
{
}

VOID ragl::anim::Paint( HDC hDC )
{
} 

VOID ragl::anim::Timer( VOID )
{
  Idle();
} 

VOID ragl::anim::Activate( VOID )
{
} 

VOID ragl::anim::Idle( VOID )
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

