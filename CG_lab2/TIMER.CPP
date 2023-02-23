#include "TIMER.H"

ragl::timer::timer( VOID ) : PauseTime(0), FrameCounter(0), IsPause(FALSE), FPS(50)
{
  LARGE_INTEGER t;

  /* Timer initialization */
  QueryPerformanceFrequency(&t);
  TimePerSec = t.QuadPart;
  QueryPerformanceCounter(&t);
  StartTime = OldTime = OldTimeFPS = t.QuadPart;
} 
UINT64 ragl::timer::GetTimePerSec( VOID )
{
 return TimePerSec;
}

DBL ragl::timer::GetTime( VOID )
{
 return Time;
}

DBL ragl::timer::GetGlobalTime( VOID )
{
  return GlobalTime;
} 

DBL ragl::timer::GetDeltaTime( VOID )
{
  return DeltaTime;
} 

DBL ragl::timer::GetGlobalDeltaTime( VOID )
{
  return GlobalDeltaTime;
} 

VOID ragl::timer::Response( VOID )
{
  LARGE_INTEGER t;

  /*** Handle timer ***/
  FrameCounter++;                    // increment frame counter (for FPS)
  QueryPerformanceCounter(&t);           // obtain current timer value
  /* Global time */
  GlobalTime = (DBL)(t.QuadPart - StartTime) / TimePerSec;
  GlobalDeltaTime = (DBL)(t.QuadPart - OldTime) / TimePerSec;
  /* Time with pause */
  if (IsPause)
  {
    PauseTime += t.QuadPart - OldTime;
    DeltaTime = 0;
  }
  else
  {
    Time = (DBL)(t.QuadPart - PauseTime - StartTime) / TimePerSec;
    DeltaTime = GlobalDeltaTime;
  }
  /* FPS */
  if (t.QuadPart - OldTimeFPS > TimePerSec)
  {
    FPS = (DBL)FrameCounter * TimePerSec / (t.QuadPart - OldTimeFPS);
    OldTimeFPS = t.QuadPart;
    FrameCounter = 0;
  }
  OldTime = t.QuadPart;
} 

VOID ragl::timer::SetPause( VOID )
{
  IsPause = !IsPause;
}
