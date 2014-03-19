import cgi
import datetime
import os
import re
import sys
import urllib
import urlparse

from google.appengine.ext import webapp
from google.appengine.api import users
from google.appengine.ext import webapp
from google.appengine.ext.webapp import template

DEBUG = True
AUTHOR = "Raymond"
APPNAME = "Raymond's Wiki"

class BaseRequestHandler( webapp.RequestHandler ):
  """Supplies a common template generation function.

  When you call generate(), we augment the template variables supplied with
  the current user in the 'user' variable and the current webapp request
  in the 'request' variable.
  """
  def generate( self, template_name, template_values = {} ):
    values = {
      'request': self.request,
      'user': users.get_current_user(),
      'login_url': users.create_login_url( self.request.uri ),
      'logout_url': users.create_logout_url( self.request.uri ),
      'application_name': APPNAME,
    }
    values.update( template_values )
    directory = os.path.dirname( __file__ )
    path = os.path.join( directory, os.path.join( '../../templates', template_name ) )
    self.response.out.write( template.render( path, values, debug = DEBUG ) )

  def head( self, *args ):
    pass

  def get( self, *args ):
    pass

  def post( self, *args ):
    pass
