#ifndef __VEC_H_
#define __VEC_H_

#include <ctime>
#include <iostream>
#include "MTHDEF.H"

/* Math library namespace */
namespace mth
{
  template<class TypeM>
    class matr;                             /* Forward declarations */

  /* Vector on plane class */
  template<class Type>
    class vec2
    {
    private:
      Type X, Z;                            /* Vector coordinates */ 
    public:
          vec2( Type x, Type z ) : X(x), Z(z)
      {
      } 

      vec2( Type c = 0 ) : X(c), Z(c)
      {
      } 

      Type GetX( VOID ) const
      {
        return X;
      } 

      Type GetZ( VOID ) const
      {
        return Z;
      } 

      DBL VecLen( VOID ) const
      {
        return sqrt(X * X + Z * Z);
      } 

      vec2 operator!( VOID ) const
      {
        DBL l = this->VecLen();
        if (!l)
          l = 1;
        return vec2(X / l, Z / l);
      } 

      bool operator==(const vec2& ot)
      {
        return X == ot.X && Z == ot.Z;
      }

      vec2 operator-( VOID ) const
      {
        return vec2(-X, -Z);
      } 

      vec2 operator+( const vec2 & v ) const
      {
        return vec2(X + v.X, Z + v.Z);
      } 

      vec2 operator-( const vec2 & v ) const
      {
        return vec2(X - v.X, Z - v.Z);
      } 

      vec2 operator*( DBL n ) const
      {
        return vec2(X * n, Z * n);
      } 

      vec2 operator/( DBL n ) const
      {
        return vec2(X / n, Z / n);
      } 

      static DBL AngleVecPlane( const vec2 & v1, const vec2 & v2 )
      {
        vec2<Type> t1 = !v1, t2 = !v2;
        DBL c = acos((t1 & t2) / (t1.VecLen() * t2.VecLen()));

        return (t1.X * t2.Z - t2.X * t1.Z >= 0)? c : -c ;
      } 

      Type operator&( const vec2 & v ) const
      {
        return X * v.X + Z * v.Z;
      } 

      vec2 & operator=( const vec2 & v )
      {
        X = v.X;
        Z = v.Z;
        return *this;
      } 

      vec2 & operator+=( const vec2 & v )
      {
        *this = *this + v;
        return *this;
      }

      vec2 & operator-=( const vec2 & v )
      {
        *this = *this - v;
        return *this;
      } 

      vec2 & operator*=( DBL n )
      {
        *this = *this * n;
        return *this;
      } 
    };
  /* Vector class */
  template<class Type>
    class vec
    {
    private:
      Type X, Y, Z;                            /* Vector coordinates */
    public:
    

      vec( Type x, Type y, Type z ) : X(x), Y(y), Z(z)
      {
      } 

      vec( Type c = 0 ) : X(c), Y(c), Z(c)
      {
      } 

      DBL VecLen( VOID ) const
      {
        return sqrt(X * X + Y * Y + Z * Z);
      } 

      static DBL DegreeToRadian( DBL a )
      {
        return a * PI / 180;
      } 

      static DBL RadianToDegree( DBL r )
      {
        return r * 180 / PI;
      } 

      static DBL AngleVec( const vec & v1, const vec & v2 )
      {
        vec<Type> t1 = !v1, t2 = !v2;
        DBL c = acos((t1 & t2) / (t1.VecLen() * t2.VecLen()));

        return (t1.X * t2.Y - t2.X * t1.Y >= 0)?:;
      } 

      operator Type *( VOID )
      {
        return &X;
      } 

      Type & operator[]( UINT Index )
      {
        switch (Index)
        {
        case 1:
          return Y;
        case 2:
          return Z;
        default:
          return X;
        }
      } 

      vec operator-( VOID ) const
      {
        return vec(-X, -Y, -Z);
      } 

      vec operator+( const vec & v ) const
      {
        return vec(X + v.X, Y + v.Y, Z + v.Z);
      } 

      vec operator-( const vec & v ) const
      {
        return vec(X - v.X, Y - v.Y, Z - v.Z);
      } 

      vec operator*( DBL n ) const
      {
        return vec(X * n, Y * n, Z * n);
      } 

      vec operator/( DBL n ) const
      {
        return vec(X / n, Y / n, Z / n);
      } 
    
      /*
      static vec operator*( DBL n, const vec & v )
      {
        return v * n;
      }
      */

      Type operator&( const vec & v ) const
      {
        return X * v.X + Y * v.Y + Z * v.Z;
      } 

      vec operator%( const vec & v ) const
      {
        return vec(Y * v.Z - Z * v.Y, Z * v.X - X * v.Z, X * v.Y - Y * v.X);
      } 

      vec operator!( VOID ) const
      {
        DBL l = this->VecLen();
        if (!l)
          l = 1;
        return vec(X / l, Y / l, Z / l);
      } 

      vec & Normalize( VOID )
      {
        *this = !(*this);
        return *this;
      } 

      vec & operator=( const vec & v )
      {
        X = v.X;
        Y = v.Y;
        Z = v.Z;
        return *this;
      } 

      vec & operator+=( const vec & v )
      {
        *this = *this + v;
        return *this;
      } 

      vec & operator-=( const vec & v )
      {
        *this = *this - v;
        return *this;
      } 

      vec & operator*=( DBL n )
      {
        *this = *this * n;
        return *this;
      } 

      vec Min( const vec & v ) const
      {
        return vec((X < v.X)? X : v.X, 
                   (Y < v.Y)? Y : v.Y, 
                   (Z < v.Z)? Z : v.Z);
      } 

      vec Max( const vec & v ) const
      {
        return vec((X > v.X)? X : v.X, 
                   (Y > v.Y)? Y : v.Y, 
                   (Z > v.Z)? Z : v.Z);
      } 

      static vec Rnd( VOID )
      {
        return vec(rand(), rand(), rand());
      } 

      Type GetX( VOID ) const
      {
        return X;
      } 

      Type GetY( VOID ) const
      {
        return Y;
      } 

      Type GetZ( VOID ) const
      {
        return Z;
      } 

      vec PointTransform( const matr<Type> &m ) const
      {
        Type x = this->GetX(), y = this->GetY(), z = this->GetZ();
        DBL w = x * m.M[0][3] + y * m.M[1][3] + z * m.M[2][3] + m.M[3][3];
    
        return vec( (x * m.M[0][0] + y * m.M[1][0] + z * m.M[2][0] + m.M[3][0]) / w, 
                    (x * m.M[0][1] + y * m.M[1][1] + z * m.M[2][1] + m.M[3][1]) / w, 
                    (x * m.M[0][2] + y * m.M[1][2] + z * m.M[2][2] + m.M[3][2]) / w);
      } 

      vec VectorTransform( const matr<Type> &m ) const
      {
        Type p[4][4], x = this->GetX(), y = this->GetY(), z = this->GetZ();
    
        return vec<Type>(x * m.M[0][0] + y * m.M[1][0] + z * m.M[2][0], 
                         x * m.M[0][1] + y * m.M[1][1] + z * m.M[2][1], 
                         x * m.M[0][2] + y * m.M[1][2] + z * m.M[2][2]);
      } 

      vec NormalTransform( const matr<Type> &m ) const
      {
        matr<Type> w = m;
        Type  x = this->GetX(), y = this->GetY(), z = this->GetZ();
        if (!w.IsInverseEvaluated)
          w.EvInverse();
        w = w.MatrTranspose();
        return vec<Type>(x * w.M[0][0] + y * w.M[0][1] + z * w.M[0][2], 
                         x * w.M[1][0] + y * w.M[1][1] + z * w.M[1][2], 
                         x * w.M[2][0] + y * w.M[2][1] + z * w.M[2][2]);
      } 

      vec operator*( const matr<Type> &m ) const
      {
        return PointTransform(m);
      } 
  }; 

    template<class Type>
    std::ostream & operator<<( std::ostream &C, const vec<Type> &V )
    {
      C << '(' << V.GetX() << ',' << V.GetY() << ',' << V.GetZ << ')';
      return C;
    } 
}

#endif // __VEC_H_

