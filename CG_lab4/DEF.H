#ifndef __DEF_H_
#define __DEF_H_

#include <windows.h>

#pragma warning(disable : 4200 4244 4013 4018 4115 4761 4127 4305 4996)

/* WinAPI and Direct3D depended files */
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <d3dcommon.h>

#include <windows.h>
#include <cassert>

/* Math support */
#include "Math/MTH.H"

typedef mth::vec<FLT> vec;
typedef mth::vec2<FLT> vec2;
typedef mth::matr<FLT> matr;
typedef mth::raw_matr<FLT> raw_matr;
typedef mth::color<FLT> color;
typedef mth::camera<FLT> camera;


/* Memory leaks detection: */
/* Debug memory allocation support */ 
#ifndef NDEBUG 
# define _CRTDBG_MAP_ALLOC
# include <crtdbg.h> 
# define SetDbgMemHooks() \
  _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF | \
  _CRTDBG_ALLOC_MEM_DF | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
static class __Dummy 
{ 
public: 

  __Dummy( VOID ) 
  { 
    SetDbgMemHooks(); 
  } 
} __oops;

#endif 

#ifdef _DEBUG 
# ifdef _CRTDBG_MAP_ALLOC 
#   define new new(_NORMAL_BLOCK, __FILE__, __LINE__) 
# endif /* _CRTDBG_MAP_ALLOC */ 
#endif /* _DEBUG */

#endif // __DEF_H_


