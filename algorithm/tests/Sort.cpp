#include "Sort.h"
#include "gtest/gtest.h"

using namespace std;
using namespace RayCommon;

const int input1Len = 6;
int input1[6] = {3, 6, 13, 2, 7, 5};

const int input2Len = 10;
int input2[10] = { 23, 11, 242, 3, 134,
                   31, 53, 12, 23, 100 };

TEST(Sorting, QuickSort)
{
  const int input1Len = 6;
  int input1[6] = {3, 6, 13, 2, 7, 5};

  const int input2Len = 10;
  int input2[10] = { 23, 11, 242, 3, 134,
                     31, 53, 12, 23, 100 };

  QuickSort(input1, input1Len);
  for(int i=0; i<6; i++){
    printf("%d\t", input1[i]);
  }
  printf("\n");

  QuickSort(input2, input2Len);
  for(int i=0; i<10; i++){
    printf("%d\t", input2[i]);
  }
  printf("\n");
}

TEST(Sorting, HeapSort)
{
  const int input1Len = 6;
  int input1[6] = {3, 6, 13, 2, 7, 5};

  const int input2Len = 10;
  int input2[10] = { 23, 11, 242, 3, 134,
                     31, 53, 12, 23, 100 };

  Heap<int> heap(5);
  for(int i=0; i<input2Len; i++)
    heap.Insert(input2[i]);

  heap.Sort();
}


