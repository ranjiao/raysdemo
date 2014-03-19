from common import *
from page import *
from user import *

class UserSearchHandler( BaseRequestHandler ):
  def get( self, user_name ):
    if not user_name:
      self.list_all_user()
    else:
      self.find_user( user_name )

  def find_user( self, user_name ):
    query = UserModel.all()
    query.filter( "name =", user_name )
    user = query.get()

    mode = "search_user"
    self.generate( mode + '.html', {
      'users': ( user ),
    } )

  def list_all_user( self ):
    query = UserModel.all()
    users = query.fetch( limit = 100 )

    mode = "search_user"
    self.generate( mode + '.html', {
      'users': users,
    } )
