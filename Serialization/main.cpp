#include "TestEntity.h"

void main()
{
#if 0
  TestEntity t1, t2, t3;

  t1.m_name = "t1";
  t1.m_number = 1;
  t1.m_age = 123;

  t2.m_name = "t2";
  t2.m_number = 2;
  t2.m_age = 122;

  t3.m_name = "t3";
  t3.m_number = 3;
  t3.m_age = 133;

  IArchiveDoc* doc = new XmlArchiveDoc();
  doc->Archive(&t1);
  doc->Archive(&t2);
  doc->Archive(&t3);
  doc->Save("out.xml");
#else
  TestEntity buffer[3];

  IArchiveDoc* doc = new XmlArchiveDoc();
  doc->Load("out.xml");
  for(int i=0; i<3 && doc->HasObjLeft(); i++)
  {
    doc->ParseObj(&buffer[i]);
  }

#endif

  delete doc;
}