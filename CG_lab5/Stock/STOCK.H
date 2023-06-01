#ifndef __STOCK_H_
#define __STOCK_H_

#include "../DEF.H"

/* Stock class */
template<class Type>
class stock
{
private:
  INT RealSize,   // Counter of elements
    MaxSize;      // Max num of elements
  Type *Array;    // Allocated memory for elements
public:

  stock( VOID ) : Array(NULL), RealSize(0), MaxSize(0)
  {
  } 

  stock( const stock &Data ) :
    MaxSize(Data.MaxSize), 
    RealSize(Data.RealSize), Array(new Type[MaxSize])
  {
    for (INT i = 0; i < RealSize; i++)
      Array[i] = Data.Array[i];
  }

  ~stock( VOID )
  {
    if (Array != NULL)
      delete[] Array;
  } 

  template<class Type>
  stock<Type> & operator=( const stock<Type> &Data )
  {
    if (Array != NULL)
      delete[] Array;
  
    Array = new type[Data.RealSize];
    MaxSize = Data.RealSize;
    MaxSize = Data.MaxSize;
    for (INT i = 0; i < RealSize; i++)
      Array[i] = Data.Array[i];
    return *this;
  } 

  INT GetSize( VOID ) const
  {
    return RealSize;
  }
  
  stock<Type> & Add( const Type &NewData )
  {
    if (RealSize >= MaxSize)
    {
      if (!MaxSize)
        MaxSize = 1;
      MaxSize *= 2;
      Type *NewArr = new Type[MaxSize];
      if (Array != NULL)
      {
        for (INT i = 0; i < RealSize; i++) 
          NewArr[i] = Array[i];
        delete[] Array;
      }
      Array = NewArr;
    }
    Array[RealSize++] = NewData;
    return *this;
  } 
  
  template<class Type>
  stock<Type> & operator<<( const Type &NewData )
  {
    return Add(NewData);
  } 

  Type & operator[]( UINT Index )
  {
    assert(Index < RealSize);
    return Array[Index];
  } 

  stock<Type> & Clear( VOID )
  {
    RealSize = 0;
    return *this;
  } 

  stock<Type> & DeleteLast( VOID )
  {
    if (RealSize > 0)
      RealSize--;
    return *this;
  } 

  template<class Type2>
  VOID Walk( Type2 &Walker )
  {
    for (INT i = 0; i < RealSize; i++)
      Walker(Array[i]);
  } 
  
}; 

#endif // __STOCK_H_
