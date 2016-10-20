/*! @file
 *
 *  @brief Median filter.
 *
 *  Implementation of functions for performing a median filter on half-word-sized data.
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-10-31
 */
/*!
 * @addtogroup Median_module Median module documentation
 * @{
 */
#include "median.h"
#include <stdio.h>
#include <math.h>

//static int16_t* Median_copyArray( int16_t* arrayPt, const uint32_t size, int16_t* arrayCopyPt);
void Partition(int16_t* arraY, int32_t sizE);
void Quicksort(int16_t* arraY, int32_t sizE);

int16_t Median_Filter(const int16_t array[], const uint32_t size)
{
  int16_t value;
  int16_t i, j;
  int16_t copyArray[11];
  uint32_t arrayIndex;

  //copy the constant array
  for (uint32_t arrayIndex = 0; arrayIndex < (size); arrayIndex++)
  {
    // copy each element of the constant array to the copy
    copyArray[arrayIndex] = array[arrayIndex];
  }

  // Sort the array from lowest to highest value
  Quicksort(copyArray,(int32_t)size);

  if ((size % 2) == 0)
  {
    // return average of middle values
    return ((copyArray[(size/2)-1])+(copyArray[size/2]))/2;
  }
  else
  {
    // return middle value of sorted array of odd size, noted integer division
    return copyArray[(size/2)];
  }
}

void Partition(int16_t* arraY, int32_t sizE)
{
  // Partitions of size 0 or 1 are already sorted
  if (sizE <= 1)
  {
    return;
  }

  // Select the pivot half way through the array, but not the first element
  int32_t iPivot = arraY[sizE/2];

  // Indices of the entries to be swapped
  int32_t lowerIndex = -1;
  int32_t upperIndex = sizE;

  // Partition array into sections above and below the pivot
  while (lowerIndex < upperIndex)
  {
    do
    {
      //Increment lower array index
      ++lowerIndex;
    }
    // Compare array value with chosen pivot
    while (arraY[lowerIndex] < iPivot);

    do
    {
      //decrement upper array index
      --upperIndex;
    }
    //compare upper array value with chosen pivot
    while (arraY[upperIndex] > iPivot);

    // Swap the entries at the lower and upper indices
    if (lowerIndex < upperIndex)
    {
      int iTemp         = arraY[lowerIndex];
      arraY[lowerIndex] = arraY[upperIndex];
      arraY[upperIndex] = iTemp;
    }
  }
  // Recursively call partition on each partititon.
  Partition(arraY, lowerIndex);
  Partition(&(arraY[lowerIndex]), sizE - lowerIndex);
}

void Quicksort(int16_t* arraY, int32_t sizE)
{
  // Seed the random number generator
  Partition(arraY, sizE);
}

/*!
 * @}
 */
