from google.appengine.api import datastore
from google.appengine.api import datastore_types
from google.appengine.api import users
from google.appengine.ext import db
import datetime

class UserModel( db.Model ):
  user_object = db.UserProperty( required = True )
  type = db.StringProperty( default = "user", choices = set( ["admin", "user"] ) )
  commit_count = db.IntegerProperty( default = 0 )
  pages = db.StringListProperty()

  @staticmethod
  def find( user ):
    query = UserModel.all()
    query.filter( "user_object = ", user )
    return query.get()
