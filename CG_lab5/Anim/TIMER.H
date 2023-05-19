#ifndef __TIMER_H_
#define __TIMER_H_

#include "../DEF.H"
#include "../Math/MTH.H"

/* Timer class */
class timer
{
protected:
    /* Timer data */
DBL
  GlobalTime, GlobalDeltaTime, /* Global time and interframe interval */
  Time, DeltaTime,             /* Time with pause and interframe interval */
  FPS;                         /* Frames per seond value */
BOOL
  IsPause;                     /* Pause flag */
private:
  /* Timer local data */
  UINT64
    StartTime,  /* Start program time */
    OldTime,    /* Time from program start to previous frame */
    OldTimeFPS, /* Old time FPS measurement */
    PauseTime,  /* Time during pause period */
    TimePerSec, /* Timer resolution */
    FrameCounter; /* Frames counter */
public:

  timer( VOID );

  UINT64 GetTimePerSec( VOID );

  DBL GetTime( VOID );

  DBL GetGlobalTime( VOID );

  DBL GetDeltaTime( VOID );

  DBL GetGlobalDeltaTime( VOID );

  VOID Response( VOID );

  VOID SetPause( VOID );

};

#endif // __TIMER_H_

