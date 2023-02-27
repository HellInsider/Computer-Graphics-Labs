#include "WIN.H"

INT win::WindowCounter = 0;
INT win::RefreshTimerID = 23;
INT win::InitTimerID = 9;

LRESULT CALLBACK win::WindowFunc( HWND hWnd, UINT Msg,
                                        WPARAM wParam, LPARAM lParam )
{
  win *Win;
  
  switch (Msg)
  {
  case WM_GETMINMAXINFO:
    ((MINMAXINFO *)lParam)->ptMaxTrackSize.y =
      GetSystemMetrics(SM_CYMAXTRACK) +
      GetSystemMetrics(SM_CYCAPTION) +
      GetSystemMetrics(SM_CYMENU) +
      GetSystemMetrics(SM_CYBORDER) * 2;
    return 0;
  case WM_CREATE:
    /* Attach 'this' pointer to window class to window */
    SetWindowLongPtr(hWnd, 0, (LONG_PTR)((CREATESTRUCT *)lParam)->lpCreateParams);
  default:
    Win = (win *)GetWindowLongPtr(hWnd, 0);
    if (Win != NULL)
      switch (Msg)
      {
      case WM_CREATE:
        Win->hWnd = hWnd;
        return Win->OnCreate((CREATESTRUCT *)lParam) ? 0 : -1;
      case WM_SIZE:
        Win->OnSize(wParam, LOWORD(lParam), HIWORD(lParam));
        return 0;
      case WM_ERASEBKGND:
        return (LRESULT)Win->OnEraseBkgnd((HDC)wParam);
      case WM_TIMER:
        Win->OnTimer((UINT)wParam);
        return 0;
      case WM_PAINT:
        Win->OnPaint();
        return 0;
      case WM_KEYDOWN:
        ///if (wParam == 'F')
        ///  Win->FlipFullScreen();
      case WM_MOUSEWHEEL:
        Win->OnMouseWheel((SHORT)HIWORD(wParam));
        return 0;
      case WM_DESTROY:
        Win->OnDestroy();
        return 0;
      }
    return DefWindowProc(hWnd, Msg, wParam, lParam);
  }
} 

win::win( HINSTANCE hInst ) : hInstance(hInst), IsFullScreen(FALSE), MouseWheel(0)
{
  WNDCLASS wc;

  if (WindowCounter++ == 0)
  {
    /* Window class initialization */
    wc.style = CS_VREDRAW | CS_HREDRAW;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(VOID *);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(NULL, IDC_UPARROW);
    wc.hIcon = LoadIcon(NULL, IDI_QUESTION);
    wc.lpszMenuName = NULL;
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WindowFunc;
    wc.lpszClassName = "My WindowClass";
    
    /* Error register handle */
    if (!RegisterClass(&wc))
    {
      MessageBox(NULL, "Error register window class", "ERROR", MB_OK);
      return;
    }
  }
    

  hWnd = CreateWindow("My WindowClass", "Feels bad man",
    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    NULL, NULL, hInstance, (VOID *)this);
} 


win::~win( VOID )
{
  if (hWnd != NULL)
    DestroyWindow(hWnd);
  WindowCounter--;
} 

VOID win::Run( VOID )
{
  MSG msg;
 
  while (TRUE)
  {
    /* Check message at window message queue */
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      if (msg.message == WM_QUIT)
        break;
      else
      {
        /* Displatch message to window */
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    else
    {
      /* Idle... */
      if (IsInit)
        Idle();
    }
  }
  //return msg.wParam;
} 

VOID win::FlipFullScreen( VOID )
{
  if (IsFullScreen)
  {
  /* restore window size */
  SetWindowPos(hWnd, HWND_NOTOPMOST,
    SaveRect.left, SaveRect.top,
    SaveRect.right - SaveRect.left, SaveRect.bottom - SaveRect.top,
    SWP_NOOWNERZORDER);
  }
  else
  {
    /* Set full screen size to window */
    HMONITOR hmon;
    MONITORINFOEX moninfo;
    RECT rc;

    /* Store window old size */
    GetWindowRect(hWnd, &SaveRect);

    /* Get nearest monitor */
    hmon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

    /* Obtain monitor info */
    moninfo.cbSize = sizeof(moninfo);
    GetMonitorInfo(hmon, (MONITORINFO *)&moninfo);

    /* Set window new size */
    rc = moninfo.rcMonitor;
    AdjustWindowRect(&rc, GetWindowLong(hWnd, GWL_STYLE), FALSE);

    SetWindowPos(hWnd, HWND_NOTOPMOST,
      rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
      SWP_NOOWNERZORDER);
  }
  IsFullScreen = !IsFullScreen;
} 

BOOL win::OnCreate( CREATESTRUCT *CS )
{      
  SetTimer(hWnd, InitTimerID, 10, NULL);
  return TRUE;
} 

VOID win::OnDestroy( VOID )
{
  if (IsInit)
  {
    Close();
    KillTimer(hWnd, RefreshTimerID);
  }
  else
    KillTimer(hWnd, InitTimerID);
  if (WindowCounter == 1)
    PostQuitMessage(0);
  hWnd = NULL;
} 

VOID win::OnSize( UINT State, INT W, INT H )
{
  this->W = W;
  this->H = H;
  if (IsInit)
    Resize();
} 

BOOL win::OnEraseBkgnd( HDC hDC )
{
  if (IsInit)
    Erase(hDC);
  return FALSE;
} 

VOID win::OnPaint( VOID )
{
  HDC hDC;
  PAINTSTRUCT ps;
    
  hDC = BeginPaint(hWnd, &ps);
  if (IsInit)
    Paint(hDC);
  EndPaint(hWnd, &ps); 
} 

VOID win::OnActivate( UINT Reason, HWND hWndActDeact, BOOL IsMinimized )
{
  IsActive = Reason == WA_CLICKACTIVE || Reason == WA_ACTIVE;
  if (IsInit)
    Activate();
} 

VOID win::OnTimer( INT Id ) 
{
  if (!IsInit)
  {
    KillTimer(hWnd, InitTimerID);
    //SetTimer(hWnd, RefreshTimerID, 0, NULL);
    IsInit = TRUE;
    Init();
    Resize();
    Timer();
  }
  else
    Timer();
} 

VOID win::OnMouseWheel( INT Z )
{
  MouseWheel += Z;
} 
    
VOID win::OnClose( VOID )
{
    
}

 

VOID win::Init( VOID )
{
} 

VOID win::Close( VOID )
{
} 

VOID win::Resize( VOID )
{
} 

VOID win::Erase( HDC hDC )
{
} 

VOID win::Paint( HDC hDC )
{
} 

VOID win::Timer( VOID )
{
} 

VOID win::Activate( VOID )
{
} 

VOID win::Idle( VOID )
{
} 
