#ifndef __WIN_H_
#define __WIN_H_

#include "DEF.H"

/* Window class */
class win
{
protected:
  HWND hWnd;           // window handle
  INT W, H;            // window size
  HINSTANCE hInstance; // application handle
  BOOL IsFullScreen;   // Full Screen flag
  BOOL IsActive;       // Active flag
  BOOL IsInit;         // Initialization flag
  RECT SaveRect;       // Window size save
  INT MouseWheel;      // Mouse wheel
  static INT WindowCounter;

private:
  static INT InitTimerID, RefreshTimerID;

  static LRESULT CALLBACK WindowFunc( HWND hWnd, UINT Msg,
                                      WPARAM wParam, LPARAM lParam );
  
public:

  win( HINSTANCE hInst = GetModuleHandle(NULL) );

  ~win( VOID ); 


  VOID Run( VOID );

  VOID FlipFullScreen( VOID );

  BOOL OnCreate( CREATESTRUCT *CS );

  VOID OnDestroy( VOID );

  VOID OnSize( UINT State, INT W, INT H );

  BOOL OnEraseBkgnd( HDC hDC );
  
  VOID OnPaint( VOID );
  
  VOID OnActivate( UINT Reason, HWND hWndActDeact, BOOL IsMinimized );
  
  VOID OnTimer( INT Id );
  
  VOID OnMouseWheel( INT Z );

  VOID OnClose( VOID );
  
  virtual VOID Init( VOID );

  virtual VOID Close( VOID );

  virtual VOID Resize( VOID );

  virtual VOID Erase( HDC hDC );

  virtual VOID Paint( HDC hDC );

  virtual VOID Timer( VOID );

  virtual VOID Activate( VOID );

  virtual VOID Idle( VOID );
  
};

#endif // __WIN_H_
