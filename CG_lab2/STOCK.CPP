/* FILE NAME   : STOCK.CPP
 * PURPOSE     : stock of data
 * PROGRAMMER  : AK5a.
 * LAST UPDATE : 21.07.2017
 */

#include "STOCK.H"

/* Default constructor method 
 *   ARGUMENTS: none.
 *   RETURNS  : 
 *     - new stock;
 */
template<class Type>
ragl::stock<Type>::stock( VOID ) : Array(NULL), RealSize(0), MaxSize(0)
{
} /* End of default constructor */

/* Destructor method 
 *   ARGUMENTS: none.
 *   RETURNS  : none.
 */
template<class Type>
ragl::stock<Type>::~stock( VOID )
{
  if (Array != NULL)
    delete[] Array;
} /* End of destructor */

/* Copy constructor method 
 *   ARGUMENTS: 
 *     - sourse stock
 *         const stock &Data;
 *   RETURNS  : 
 *     - new stock;
 */
template<class Type>
ragl::stock<Type>::stock( const stock &Data ) :
  MaxSize(Data.MaxSize), 
  RealSize(Data.RealSize), Array(new Type[MaxSize])
{
  for (INT i = 0; i < RealSize; i++)
    Array[i] = Data.Array[i];
} /* End of copy constructor */

/* Operator= method 
 *   ARGUMENTS: 
 *     - stock
 *         const stock<Type> &Data;
 *   RETURNS  : 
 *     - reference to changed stock
 *         stock<Type> & *this;
 */
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
} /* End of operator= method */

/* Element addition method 
 *   ARGUMENTS: 
 *     - element to addition
 *         const Type &NewData;
 *   RETURNS  : 
 *     - reference to changed stock
 *         stock<Type> & *this;
 */
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
} /* End of 'Add' method */

/* Operator<< method 
 *   ARGUMENTS: 
 *     - element to addition
 *         const Type &NewData;
 *   RETURNS  : 
 *     - reference to changed stock
 *         stock<Type> & *this;
 */
template<class Type>
ragl::stock<Type> & ragl::stock<Type>::operator<<( const Type &NewData )
{
  return Add(NewData);
} /* End of operator<< method */

/* Operator[] method 
 *   ARGUMENTS: 
 *     - index of element
 *         UINT Index;
 *   RETURNS  : 
 *     - reference to element
 *         Type & Array[index];
 */
template<class Type>
Type & ragl::stock<Type>::operator[]( UINT Index )
{
  assert(Index < RealSize);
  return Array[Index];
} /* End of operator[] method */

/* Stock clearing method 
 *   ARGUMENTS: none.
 *   RETURNS  : 
 *     - reference to changed stock
 *         stock<Type> & *this;
 */
template<class Type>
ragl::stock<Type> & ragl::stock<Type>::Clear( VOID )
{
  RealSize = 0;
} /* End of 'Clear' method */

/* Remove last stock element function.
 * ARGUMENTS: none.
 * RETURNS:
 *   (stock &) self reference.
 */
template<class Type>
ragl::stock<Type> & ragl::stock<Type>::DeleteLast( VOID )
{
  if (RealSize > 0)
    RealSize--;
  return *this;
} /* End of 'DeleteLast' function */

/* Stock walking method 
 *   ARGUMENTS: reference to walker function.
 *     - Type2 &Walker;
 *   RETURNS  : none.;
 */
template<class Type>
template<class Type2>
VOID ragl::stock<Type>::Walk( Type2 &Walker )
{
  for (INT i = 0; i < RealSize; i++)
    Walker(Array[i]);
} /* End of 'Walk' method */

/* END OF 'STOCK.CP' FILE */