#ifndef __MATR_H_
#define __MATR_H_

#include "MTHDEF.H"
#include "VEC.H"

/* Math library namespace (continue) */
namespace mth
{
  /* Matrix class */
  template<class Type>
  class matr
  {
    friend class vec<Type>;
  private:
    Type M[4][4];                       /* matrix array */
    mutable DBL InvM[4][4];          /* inverce matrix array */
    mutable BOOL IsInverseEvaluated;    /* flag */                                   /*________wtf________*/
  public:
    explicit matr( VOID )
    {
      M[0][0] = M[1][1] = M[2][2] = M[3][3] = 1;
      M[0][1] = M[0][2] = M[0][3] = M[1][0] = 0;
      M[1][2] = M[1][3] = M[2][0] = M[2][1] = 0;
      M[2][3] = M[3][0] = M[3][1] = M[3][2] = 0;
      InvM[0][0] = InvM[1][1] = InvM[2][2] = InvM[3][3] = 1;
      InvM[0][1] = InvM[0][2] = InvM[0][3] = InvM[1][0] = 0;
      InvM[1][2] = InvM[1][3] = InvM[2][0] = InvM[2][1] = 0;
      InvM[2][3] = InvM[3][0] = InvM[3][1] = InvM[3][2] = 0;
      IsInverseEvaluated = 1;
    } 

    Type GetElement( INT n, INT m )
    {
      if (n < 0 || n > 3 || m < 0 || m > 3)
        return M[0][0];
      return M[n][m];
    }

    matr operator*( const matr & m ) const
    {
      INT k;
      matr r;

      for (INT i = 0; i < 4; i++)
        for (INT j = 0; j < 4; j++)
          for (k = 0, r.M[i][j] = 0; k < 4; k++)
            r.M[i][j] += M[i][k] * m.M[k][j];
      return r;
    } 

    matr & operator=( const matr & m )
    {
      for (INT i = 0; i < 4; i++)
        for (INT j = 0; j < 4; j++)
          M[i][j] = m.M[i][j];
      IsInverseEvaluated = 0;
      return *this;
    } 

    operator Type *( VOID )
    {
      return &(M[0][0]);
    } 

    matr & operator*=( const matr & m )
    {    
      IsInverseEvaluated = 0;
      *this = *this * m;
    } 

    static Type MatrDet3x3( Type A11, Type A12, Type A13, 
                     Type A21, Type A22, Type A23, 
                     Type A31, Type A32, Type A33 )
    {
      return A11 * A22 * A33 - A11 * A23 * A32 - A12 * A21 * A33 + 
             A12 * A23 * A31 + A13 * A21 * A32 - A13 * A22 * A31;
    } 

    Type MatrDeterm( VOID ) const
    {
      return M[0][0] * MatrDet3x3(M[1][1], M[1][2], M[1][3], 
                                  M[2][1], M[2][2], M[2][3], 
                                  M[3][1], M[3][2], M[3][3]) - 
             M[0][1] * MatrDet3x3(M[1][0], M[1][2], M[1][3], 
                                  M[2][0], M[2][2], M[2][3], 
                                  M[3][0], M[3][2], M[3][3]) +
             M[0][2] * MatrDet3x3(M[1][0], M[1][1], M[1][3], 
                                  M[2][0], M[2][1], M[2][3], 
                                  M[3][0], M[3][1], M[3][3]) -
             M[0][3] * MatrDet3x3(M[1][0], M[1][1], M[1][2], 
                                  M[2][0], M[2][1], M[2][2], 
                                  M[3][0], M[3][1], M[3][2]);
    } 

    matr operator!( VOID ) const
    {
      matr r, in;
      if (!IsInverseEvaluated)
      {
        memcpy(r.M, M, 4 * 4 * sizeof(Type));
        r.EvInverse();
        return !r;
      }
      memcpy(r.M, InvM, 4 * 4 * sizeof(Type));
      memcpy(r.InvM, M, 4 * 4 * sizeof(Type));
      return r;
    } 

    VOID EvInverse( VOID )
    {
      Type d = MatrDeterm();
      INT p[4][3] = {{1, 2, 3}, {0, 2, 3}, {0, 1, 3}, {0, 1, 2}};

      if (!d)
        return;
      for (INT i = 0; i < 4; i++)
        for (INT j = 0; j < 4; j++)
          InvM[j][i] = matr::MatrDet3x3(
                       M[p[i][0]][p[j][0]], M[p[i][0]][p[j][1]], M[p[i][0]][p[j][2]], 
                       M[p[i][1]][p[j][0]], M[p[i][1]][p[j][1]], M[p[i][1]][p[j][2]], 
                       M[p[i][2]][p[j][0]], M[p[i][2]][p[j][1]], M[p[i][2]][p[j][2]]) / d;
      IsInverseEvaluated = 1;
    } 

    matr MatrTranslate( const vec<Type> &t ) const
    {
      matr<Type> r;

      r.M[3][0] = t.GetX();
      r.M[3][1] = t.GetY();
      r.M[3][2] = t.GetZ();

      return *this * r;
    } 

    matr MatrScale( const vec<Type> &t ) const
    {
      matr<Type> r;

      r.M[0][0] = t.GetX();
      r.M[1][1] = t.GetY();
      r.M[2][2] = t.GetZ();

      return *this * r;
    } 

    matr MatrRotateX( DBL AngleInDegrees ) const
    {
      matr<Type> r;
      DBL a = vec<Type>::DegreeToRadian(AngleInDegrees);

      r.M[2][2] = r.M[1][1] = cos(a);
      r.M[2][1] = -sin(a);
      r.M[1][2] = -r.M[2][1];

      return *this * r;
    } 

    matr MatrRotateY( DBL AngleInDegrees ) const
    {
      matr<Type> r;
      DBL a = vec<Type>::DegreeToRadian(AngleInDegrees);

      r.M[2][2] = r.M[0][0] = cos(a);
      r.M[0][2] = -sin(a);
      r.M[2][0] = -r.M[0][2];

      return *this * r;
    } 

    matr MatrRotateZ( DBL AngleInDegrees ) const
    {
      matr<Type> r;
      DBL a = vec<Type>::DegreeToRadian(AngleInDegrees);

      r.M[0][0] = r.M[1][1] = cos(a);
      r.M[1][0] = -sin(a);
      r.M[0][1] = -r.M[1][0];

      return *this * r;
    } 

    matr MatrRotate( DBL AngleInDegrees, const vec<Type> &a ) const
    {
      DBL rad = vec<Type>::DegreeToRadian(AngleInDegrees);
      vec<Type> n = !a;
      DBL si = sin(rad), co = cos(rad), x = n.GetX(), y = n.GetY(), z = n.GetZ();
      matr<Type> r;

      r.M[0][0] = co + x * x * (1 - co);
      r.M[0][1] = x * y * (1 - co) - z * si;
      r.M[0][2] = x * z * (1 - co) + y * si;
      r.M[1][0] = y * x * (1 - co) + z * si;
      r.M[1][1] = co + y * y * (1 - co);
      r.M[1][2] = y * z * (1 - co) - x * si;
      r.M[2][0] = z * x * (1 - co) - y * si;
      r.M[2][1] = z * y * (1 - co) + x * si;
      r.M[2][2] = co + z * z * (1 - co);

      return *this * r;
    } 

    matr MatrTranspose( VOID ) const
    {
      matr<Type> r;
      for (INT i = 0; i < 4; i++)
        for (INT j = 0; j < 4; j++)
          r.M[i][j] = M[j][i];
      return r;
    } 

    template<class VecType>
    static matr MatrView( const vec<VecType> &Loc, const vec<VecType> &At, const vec<VecType> &Up1 )
    {
      vec<VecType> Dir = !(At - Loc), 
                   Right = !(Dir % Up1), 
                   Up = !(Right % Dir);
      VecType rx = Right.GetX(), ry = Right.GetY(), rz = Right.GetZ(),
              ux = Up.GetX(), uy = Up.GetY(), uz = Up.GetZ(),
              dx = Dir.GetX(), dy = Dir.GetY(), dz = Dir.GetZ(),
              lx = Loc.GetX(), ly  = Loc.GetY(), lz = Loc.GetZ();
      matr V;
      V.M[0][0] = rx, V.M[0][1] = ux, V.M[0][2] = - dx, V.M[0][3] = 0;
      V.M[1][0] = ry, V.M[1][1] = uy, V.M[1][2] = - dy, V.M[1][3] = 0;
      V.M[2][0] = rz, V.M[2][1] = uz, V.M[2][2] = - dz, V.M[2][3] = 0;
      V.M[3][0] = -Loc & Right; 
      V.M[3][1] = -Loc & Up; 
      V.M[3][2] = Loc & Dir; 
      V.M[3][3] = 1;
      return V;
    } 

    template<class VecType>
    static matr MatrViewDir( const vec<VecType> &Loc, const vec<VecType> &Dir, const vec<VecType> &Up1 )
    {
      vec<VecType> d = !Dir,
                   Right = !(Dir % Up1), 
                   Up = !(Right % Dir);
      VecType rx = Right.GetX(), ry = Right.GetY(), rz = Right.GetZ(),
              ux = Up.GetX(), uy = Up.GetY(), uz = Up.GetZ(),
              dx = d.GetX(), dy = d.GetY(), dz = d.GetZ(),
              lx = Loc.GetX(), ly  = Loc.GetY(), lz = Loc.GetZ();
      matr V;
      V.M[0][0] = rx, V.M[0][1] = ux, V.M[0][2] = - dx, V.M[0][3] = 0;
      V.M[1][0] = ry, V.M[1][1] = uy, V.M[1][2] = - dy, V.M[1][3] = 0;
      V.M[2][0] = rz, V.M[2][1] = uz, V.M[2][2] = - dz, V.M[2][3] = 0;
      V.M[3][0] = -Loc & Right; 
      V.M[3][1] = -Loc & Up; 
      V.M[3][2] = Loc & d; 
      V.M[3][3] = 1;
      return V;
    } 

    template<class Type>
    static matr MatrFrustum( Type l, Type r, Type b, Type t, Type n, Type f )
    {
      matr F;
      F.M[0][0] = 2 * n / (r - l), F.M[0][1] = 0, F.M[0][2] = 0, F.M[0][3] = 0;
      F.M[1][0] = 0, F.M[1][1] = 2 * n / (t - b), F.M[1][2] = 0, F.M[1][3] = 0;
      F.M[2][0] = (r + l) / (r - l), F.M[2][1] = (t + b) / (t - b), F.M[2][2] = -(f + n) / (f - n), F.M[2][3] = -1;
      F.M[3][0] = 0, F.M[3][1] = 0, F.M[3][2] = -2 * n * f / (f - n), F.M[3][3] = 0;
      return F;
    } 

    static matr MatrRotation( DBL AngleInDegrees, const vec<Type> &a )
    {
      matr<Type> r;
      r = r.MatrRotate(AngleInDegrees, a);
      return r;
    } 

    static matr MatrRotationX( DBL AngleInDegrees )
    {
      matr<Type> r;
      r = r.MatrRotateX(AngleInDegrees);
      return r;
    } 

    static matr MatrRotationY( DBL AngleInDegrees )
    {
      matr<Type> r;
      r = r.MatrRotateY(AngleInDegrees);
      return r;
    } 

    static matr MatrRotationZ( DBL AngleInDegrees )
    {
      matr<Type> r;
      r = r.MatrRotateZ(AngleInDegrees);
      return r;
    } 

    static matr MatrTranslation( const vec<Type> &t )
    {
      matr<Type> r;
      r = r.MatrTranslate(t);
      return r;
    } 

    static matr MatrScaling( const vec<Type> &t )
    {
      matr<Type> r;
      r = r.MatrScale(t);
      return r;
    } 
    
  }; 

  template<class Type>
  std::ostream & operator<<( std::ostream &C, const matr<Type> &m )
  {
    Type a[4][4];
    a = m.ToArray();
    C << a[0][0] << ' ' << a[0][1] <<' '<< a[0][2] << ' ' << a[0][3] << std::endl
      << a[1][0] << ' ' << a[1][1] <<' '<< a[1][2] << ' ' << a[1][3] << std::endl
      << a[2][0] << ' ' << a[2][1] <<' '<< a[2][2] << ' ' << a[2][3] << std::endl
      << a[3][0] << ' ' << a[3][1] <<' '<< a[3][2] << ' ' << a[3][3] << std::endl;
    return C;
  } 

  template<class Type>
  struct raw_matr
  {
    Type m[16];
    raw_matr& fromPtr(const Type* s)
    {
      memcpy(m, s, sizeof(Type) * 16);
      return *this;
    }
  };

}
#endif // !__MATR_H_


