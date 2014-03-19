from google.appengine.api import datastore
from google.appengine.api import datastore_types
from google.appengine.api import users
from google.appengine.ext import db

import datetime

class PageModel( db.Model ):
  name = db.StringProperty( required = True )
  creator = db.UserProperty(required = True )
  creation_date = db.DateTimeProperty( auto_now_add = True )
  latest_version = db.IntegerProperty( required = True )

  @staticmethod
  def get_default_content( name ):
    return "<h1>%s</h1>" % name

  @staticmethod
  def find( name ):
    query = PageModel.all()
    query.filter( "name = ", name )
    return query.get()

class PageVersionModel( db.Model ):
  page = db.ReferenceProperty( PageModel, required = True )
  version = db.IntegerProperty( required = True )
  author = db.UserProperty( required = True )
  date = db.DateTimeProperty( auto_now_add = True )
  source = db.StringProperty( multiline = True, required = True )
  content = db.StringProperty( multiline = True )
    
