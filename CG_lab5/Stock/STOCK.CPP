#include "STOCK.H"

template<class Type>
ragl::stock<Type>::stock( VOID ) : Array(NULL), RealSize(0), MaxSize(0)
{
}

template<class Type>
ragl::stock<Type>::~stock( VOID )
{
  if (Array != NULL)
    delete[] Array;
} 

template<class Type>
ragl::stock<Type>::stock( const stock &Data ) :
  MaxSize(Data.MaxSize), 
  RealSize(Data.RealSize), Array(new Type[MaxSize])
{
  for (INT i = 0; i < RealSize; i++)
    Array[i] = Data.Array[i];
}

template<class Type>
ragl::stock<Type> & ragl::stock<Type>::operator=( const ragl::stock<Type> &Data )
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

template<class Type>
ragl::stock<Type> & ragl::stock<Type>::Add( const Type &NewData )
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
ragl::stock<Type> & ragl::stock<Type>::operator<<( const Type &NewData )
{
  return Add(NewData);
} 

template<class Type>
Type & ragl::stock<Type>::operator[]( UINT Index )
{
  assert(Index < RealSize);
  return Array[Index];
} 

template<class Type>
ragl::stock<Type> & ragl::stock<Type>::Clear( VOID )
{
  RealSize = 0;
} 

template<class Type>
ragl::stock<Type> & ragl::stock<Type>::DeleteLast( VOID )
{
  if (RealSize > 0)
    RealSize--;
  return *this;
} 

template<class Type>
template<class Type2>
VOID ragl::stock<Type>::Walk( Type2 &Walker )
{
  for (INT i = 0; i < RealSize; i++)
    Walker(Array[i]);
} 