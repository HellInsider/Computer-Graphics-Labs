#ifndef __CAMERA_H_
#define __CAMERA_H_

#include "VEC.H"
#include "MATR.H"

namespace mth
{
  /* Camera class */
  template<class Type>
  class camera
  {
  private:
    /* Viewer data */
    mth::vec<Type> Loc, Right, Up, Dir, At;
    /* Matrices */
    mth::matr<Type> MatrProj, MatrView, MatrVP;
    /* Project data */
    DBL Size, ProjDist, FarClip;
    INT W, H;
  public:
    camera( VOID ) : W(1), H(1), ProjDist(1.0), Size(0.5), FarClip(1000),
      Loc(mth::vec<Type>(20)), At(mth::vec<Type>(0)), Up(mth::vec<Type>(0, 1, 0)),
      MatrView(mth::matr<Type>::MatrView(Loc, At, Up))
    {
      UpdateProj();
    }

    mth::matr<Type> GetMatrVP( VOID ) const
    {
      return MatrVP;
    } 

    mth::matr<Type> GetMatrView( VOID ) const
    {
      return MatrView;
    }

    mth::matr<Type> GetMatrProj( VOID ) const
    {
      return MatrProj;
    } 

    mth::vec<Type> & GetLoc( VOID )
    {
      return Loc;
    } 

    mth::vec<Type> & GetAt( VOID )
    {
      return At;
    } 

    mth::vec<Type>& GetRight(VOID)
    {
      return Right = !(Dir % Up);
    } 

    VOID UpdateProj( VOID )
    {
      DBL rx = 1, ry = 1;
      if (W >= H)
        rx = (DBL)W / H;
      else
        ry = (DBL)H / W;
      MatrProj = mth::matr<Type>::MatrFrustum(-rx * Size, rx * Size,
                                              -ry * Size, ry * Size, 
                                               ProjDist, FarClip);
      MatrVP = MatrView * MatrProj;
    } 

    VOID Resize( INT NewW, INT NewH )
    {
      W = NewW;
      H = NewH;
      UpdateProj();
    } 

    VOID SetView( mth::vec<Type> NewLoc, mth::vec<Type> NewAt, mth::vec<Type> NewUp = {0, 1, 0} )
    {
      Loc = NewLoc;
      At = NewAt;
      Up = NewUp;
      MatrView = mth::matr<Type>::MatrView(Loc, At, Up);
      MatrVP = MatrView * MatrProj;
    } 

    VOID SetViewDir( mth::vec<Type> NewLoc, mth::vec<Type> NewDir, mth::vec<Type> NewUp = {0, 1, 0} )
    {
      Loc = NewLoc;
      Dir = NewDir;
      At = Loc + Dir;
      Up = NewUp;
      MatrView = mth::matr<Type>::MatrViewDir(Loc, Dir, Up);
      MatrVP = MatrView * MatrProj;
    } 

    VOID SetProj( Type NewSize, Type NewProjDist, Type NewFarClip )
    {
      Size = NewSize;
      ProjDist = NewProjDist;
      FarClip = NewFarClip;
      UpdateProj();
    }
  };
} 

#endif // __CAMERA_H_


