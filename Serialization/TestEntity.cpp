#include "TestEntity.h"

TestEntity::TestEntity()
{

}

void TestEntity::Serialize( IArchiver* ser )
{
  ser->SerValue("name", m_name);
  ser->SerValue("number", m_number);
  ser->SerValue("age", m_age);
}
