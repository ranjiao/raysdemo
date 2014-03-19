#ifndef __SERIALIZATION_H__
#define __SERIALIZATION_H__

#include <string>

class IArchiver;
class IArchiveDoc;

class ISerializable
{
public:
  virtual void Serialize(IArchiver* ser) = 0;
};

class IArchiver
{
protected:
  bool m_reading;
  IArchiveDoc* m_archDoc;
public:
  IArchiver(IArchiveDoc* doc, bool reading)
  {
    m_archDoc = doc;
    m_reading = reading; 
  };
  ~IArchiver(){};

  virtual void SerValue(std::string name, int &value) = 0;
  virtual void SerValue(std::string name, float &value) = 0;
  virtual void SerValue(std::string name, std::string &value) = 0;

  virtual IArchiver* SubArchiver(std::string name);
  
  bool IsReading(){ return m_reading; };
};

class IArchiveDoc
{
protected:
  bool m_reading, m_isOK;
  std::string m_filename;
public:
  IArchiveDoc();
  ~IArchiveDoc(){};

  virtual bool Load(std::string filename){ return false; };
  virtual bool ParseObj(ISerializable* obj){ return false; };
  virtual bool HasObjLeft(){ return false; };

  virtual bool Save(std::string filename){ return false; };
  virtual void Clear() = 0;

  bool IsOK(){ return m_isOK; };
  bool IsReading(){ return m_reading; };
  std::string GetFilename(){ return m_filename; };

  virtual void Archive(ISerializable* obj) = 0;
  virtual IArchiver* NewAchiver(std::string name) = 0;
};

#include "xml/rapidxml.hpp"
#include "xml/rapidxml_iterators.hpp"
#include "xml/rapidxml_print.hpp"
#include "xml/rapidxml_utils.hpp"

typedef rapidxml::xml_node<>* XmlNode;
typedef rapidxml::xml_attribute<>* XmlAttribute;
typedef rapidxml::xml_document<>* XmlDoc;

class XmlArchiver;

class XmlArchiveDoc: public IArchiveDoc
{
protected:
  typedef std::vector<XmlArchiver*> VecArch;
  typedef VecArch::iterator         IterArch;
  VecArch m_archivers;
  rapidxml::file<> *m_infile;
  XmlDoc m_doc;
  XmlNode m_root, m_crtNode;
public:
  XmlArchiveDoc();
  virtual ~XmlArchiveDoc();

  virtual bool Load(std::string filename);
  virtual bool ParseObj(ISerializable* obj);
  virtual bool HasObjLeft();

  virtual bool Save(std::string filename);
  virtual void Clear();

  virtual void Archive(ISerializable* obj);
  virtual IArchiver* NewAchiver(std::string name);

  XmlDoc GetXmlDoc(){ return m_doc; };
};

class XmlArchiver: public IArchiver
{
  XmlNode m_node;
  XmlDoc m_doc;
public:
  XmlArchiver(XmlArchiveDoc* doc, XmlNode node, bool reading);

  virtual void SerValue(std::string name, int &value);
  virtual void SerValue(std::string name, float &value);
  virtual void SerValue(std::string name, std::string &value);
  virtual IArchiver* SubArchiver( std::string name );

  XmlNode GetNode(){ return m_node; };
};

// class BinaryArchiver: public IArchiver
// {
// public:
//   BinaryArchiver(std::string filename);
//   virtual void SerValue(std::string name, int value);
//   virtual void SerValue(std::string name, float value);
//   virtual void SerValue(std::string name, std::string value);
// };


#endif