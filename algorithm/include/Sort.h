#ifndef __QUICKSORT_H__
#define __QUICKSORT_H__

#include <stdint.h>
#include <stdio.h>

namespace RayCommon
{
  template<typename T>
  void Swap(T& a, T& b)
  {
    T temp = a;
    a = b;
    b = temp;
  }

  template<typename T>
  class Heap
  {
    T* m_data;
    uint64_t m_cap, m_size;
  public:
    Heap(int cap)
    {
      m_cap = cap;
      m_size = 0;

      m_data = new T[cap];
    }

    ~Heap()
    {
      delete [] m_data;
    }

    uint64_t Size()
    {
      return m_size;
    }

    void Clear()
    {
      m_size = 0;
    }


    bool Insert(T& data)
    {
      if(m_size < m_cap){
        m_data[m_size] = data;
        m_size++;

        heapUp(m_data, m_size);
      }
      else
      {
        if(data < m_data[0])
        {
          return false;
        }
        else
        {
          m_data[0] = data;
          heapDown(m_data, m_size);
        }
      }
      return true;
    }

    void Sort()
    {
      for(int i=0; i<m_size-1; i++){
        heapUp(m_data+i, m_size - i);
      }
      for(int i=0; i<m_size; i++){
        printf("%d\t", m_data[i]);
      }
      printf("\n");
    }

    static void heapUp(T* data, int size)
    {
      for(int i=size-1; i>0; i--)
      {
        int parent = (i-1) / 2;
        if(data[parent] > data[i])
          Swap(data[parent], data[i]);
      }
    }

    void heapDown(T* data, int size)
    {
      int crtIdx = 0;
      while(crtIdx < size)
      {
        int leftChild = crtIdx * 2 + 1;
        int rightChild = leftChild + 1;
        int target = -1;

        if(leftChild < size &&
            data[leftChild] < data[crtIdx]){
          target = leftChild;
        }

        if(target < 0 && rightChild < size &&
            data[rightChild] < data[crtIdx]){
          target =rightChild;
        }

        if(target > 0){
          Swap(m_data[target], data[crtIdx]);
          crtIdx = target;
        }
        else
          break;
      }
    }
  };

  template<typename T>
  void QuickSort(T* data, uint64_t count)
  {
    if(!data || count <= 1)
      return;

    if(count == 2){
      if(data[0] > data[1])
        Swap(data[0], data[1]);
      return;
    }

    // choose pivot
    Swap(data[count-1], data[count / 2 -1]);

    int pivot = 0, i = 0;
    while( i<count-1 )
    {
      if(data[i] <= data[count-1])
      {
        if(i != pivot)
          Swap(data[pivot], data[i]);

        pivot++;
      }
      i++;
    }

    // if(pivot < count-1)
    Swap(data[pivot], data[count-1]);

    if(pivot != 0)
      QuickSort(data, pivot);

    if(pivot != count-1)
      QuickSort(data + pivot + 1, count - pivot-1);
  }
}

#endif /* __QUICKSORT_H__ */
