#ifndef __HASH_H__
#define __HASH_H__

namespace RayCommon
{
  template<typename T>
  class LRUCache
  {
  private:
    struct <
  };

  class Bitmap
  {
  private:
    uint64_t m_cap, m_size;
    char *m_data;
  public:
    Bitmap(uint64_t cap = 1024)
    {
      m_cap = cap;
      m_size = 0;

      m_data = new char[cap];
      memset(m_data, 0, cap);
    }

    ~Bitmap()
    {
      delete [] m_data;
    }

    void Set(uint64_t bit)
    {
      if(m_size == m_cap)
        resize(m_cap * 2);

      m_data[bit/8] &= (1 << (bit %8));
      m_size++;
    }

    bool Get(uint64_t bit)
    {
      return (m_data[bit/8] | (1 << (bit % 8))) != 0;
      m_size--;
    }

    uint64_t Size()
    {
      return m_size;
    }

    void resize(uint64_t newCap)
    {
      char *newData = new[newSize];
      memcpy(newData, m_data, m_cap);
      memset(newData+m_cap, 0, newSize - m_cap);

      m_cap = newCap;
    }
  };

  class BloomFilter
  {
  private:
    Bitmap m_bitmap;

  public:
    BloomFilter(uint64_t cap = 1024):m_bitmap(cap)
    {
    }

    virtual ~BloomFilter()
    {
    }

    void Set(uint64_t *keys, int keyCount)
    {
      for(int i=0; i<keyCount; i++)
        m_bitmap.Set(keys[i]);
    }

    bool Get(uint64_t *keys, int keyCount)
    {
      bool exists = true;
      for(int i=0; i<keyCount; i++){
        if(!m_bitmap.Get(keys[i])){
          exists = false;
          break;
        }
      }

      return exists;
    }
  };
}

#endif /* __HASH_H__ */
