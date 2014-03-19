#ifndef __TESTENTITY_H__
#define __TESTENTITY_H__

#include <string>
#include "Serialization.h"

class TestEntity: public ISerializable
{
public:
  std::string m_name;
  int m_number;
  float m_age;
public:
  TestEntity();

  virtual void Serialize(IArchiver* ser);
};

#endif