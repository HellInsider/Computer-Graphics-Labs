#ifndef __COLOR_H_
#define __COLOR_H_

#include "MTHDEF.H"

namespace mth
{
  template <class Type>
  class color
  {
  public:
    Type R, G, B, A;
    color( Type r, Type g, Type b, Type a ) : R(r), G(g), B(b), A(a)
    {
    }

    color( Type c = 0 ) : R(c), G(c), B(c), A(1)
    {
    }

    color operator+(const color& c1)
    {
      return color(c1.R + R, c1.G + G, c1.B + B, c1.A + A);
    }

    color& operator+=(const color& c)
    {
      *this = *this + c;
      return *this;
    }

    color operator*(DBL n)
    {
      return color(R * n, G * n, B * n, A * n);
    }
  };
}
#endif // __COLOR_H_

