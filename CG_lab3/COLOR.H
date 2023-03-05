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
  };
}
#endif // __COLOR_H_

