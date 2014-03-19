#include "Serialization.h"
#include <sstream>
#include <iostream>

IArchiveDoc::IArchiveDoc()
{
  m_filename = "";
  m_reading = false;
  m_isOK = false;
}

IArchiver* IArchiver::SubArchiver( std::string name )
{
  return m_archDoc->NewAchiver(name);
}

XmlArchiveDoc::XmlArchiveDoc():
  IArchiveDoc()
{
  m_infile = NULL;
  m_doc = new rapidxml::xml_document<>();
  m_root = m_crtNode = NULL;
}

XmlArchiveDoc::~XmlArchiveDoc()
{
  delete m_doc;
};

void XmlArchiveDoc::Clear()
{
  if(m_doc)
  {
    delete m_doc;
    m_doc = new rapidxml::xml_document<>();
  }
  for(IterArch iter = m_archivers.begin(); iter != m_archivers.end(); iter++)
  {
    delete *iter;
  }
  m_archivers.clear();
}

bool XmlArchiveDoc::Load( std::string filename )
{
  Clear();
  m_infile = new rapidxml::file<>(filename.c_str());
  m_doc->parse<0>(m_infile->data());
  m_root = m_doc->first_node("objects");
  if(!m_root)
    return false;

  m_crtNode = m_root->first_node();
  if(!m_crtNode)
    return false;
  
  return true;
}

bool XmlArchiveDoc::ParseObj(ISerializable* obj)
{
  int count = 0;
  if(!m_crtNode)
    return false;

  XmlArchiver* arch = new XmlArchiver(this, m_crtNode, true);
  m_archivers.push_back(arch);

  obj->Serialize( arch );

  if(!m_crtNode)
    m_crtNode = m_root->first_node();
  else
    m_crtNode = m_crtNode->next_sibling();

  return true;
}

bool XmlArchiveDoc::HasObjLeft()
{
  return m_crtNode != NULL;
}

bool XmlArchiveDoc::Save( std::string filename )
{
  std::ofstream outfile("out.xml");
  std::string text;
  rapidxml::print(std::back_inserter(text), *m_doc, 0);
  outfile << text;
  return true;
}

void XmlArchiveDoc::Archive( ISerializable* obj )
{
  if(!m_root)
  {
    m_root = m_doc->allocate_node(rapidxml::node_element, "objects");
    m_doc->append_node(m_root);
  }

  XmlNode node = m_doc->allocate_node(rapidxml::node_element, "object");
  m_root->append_node(node);

  XmlArchiver* objArch = new XmlArchiver(this, node, m_reading);
  obj->Serialize(objArch);
  
  m_archivers.push_back(objArch);
}

IArchiver* XmlArchiveDoc::NewAchiver(std::string name)
{
  XmlNode node = m_doc->allocate_node(rapidxml::node_element, name.c_str());
  XmlArchiver* archiver = new XmlArchiver(this, node, m_reading);
  m_archivers.push_back(archiver);
  return archiver;
}

XmlArchiver::XmlArchiver(XmlArchiveDoc *doc, XmlNode node, bool reading):
  IArchiver(doc, reading)
{
  m_node = node;
  m_doc = doc->GetXmlDoc();
}

void XmlArchiver::SerValue( std::string name, int  &value )
{
  if(m_reading)
  {
    const char* buffer;

    XmlAttribute attr = m_node->first_attribute(name.c_str());
    if(!attr)
      value = 0;
    else
    {
      buffer = attr->value();
      value = atoi(buffer);
    }
  }
  else
  {
    std::stringstream ss;
    ss << value;
    
    XmlAttribute attr = m_doc->allocate_attribute(
      m_doc->allocate_string(name.c_str()), 
      m_doc->allocate_string(ss.str().c_str()));
    m_node->append_attribute(attr);
  }
}

void XmlArchiver::SerValue( std::string name, float &value )
{
  if(m_reading)
  {
    const char* buffer;

    XmlAttribute attr = m_node->first_attribute(name.c_str());
    if(!attr)
      value = 0.f;
    else
    {
      buffer = attr->value();
      value = (float)atof(buffer);
    }
  }
  else
  {
    std::stringstream ss;
    ss << value;

    XmlAttribute attr = m_doc->allocate_attribute(
      m_doc->allocate_string(name.c_str()),
      m_doc->allocate_string(ss.str().c_str()));
    m_node->append_attribute(attr);
  }
}

void XmlArchiver::SerValue( std::string name, std::string &value )
{
  if(m_reading)
  {
    XmlAttribute attr = m_node->first_attribute(name.c_str());
    if(!attr)
      value = "";
    else
    {
      value = attr->value();
    }
  }
  else
  {
    XmlAttribute attr = m_doc->allocate_attribute(
      m_doc->allocate_string(name.c_str()), 
      m_doc->allocate_string(value.c_str()));
    m_node->append_attribute(attr);
  }
}

IArchiver* XmlArchiver::SubArchiver( std::string name )
{
  XmlArchiver* arch = static_cast<XmlArchiver*>(IArchiver::SubArchiver(name));
  m_node->append_node(arch->GetNode());
  return arch;
} 
