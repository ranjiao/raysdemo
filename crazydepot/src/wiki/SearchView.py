from common import *
from page import *
from PageView import *

class SearchHandler( BaseRequestHandler ):
  def get( self, param ):
    keyword = self.request.get( 'name' )
    if not keyword:
      self.find_recent_changed()
    else:
      self.find_content( keyword )

  def find_recent_changed( self ):
    query = PageModel.all()

    pages = query.fetch( limit = 100 )

    mode = "search_page"
    self.generate( mode + '.html', {
      'pages': pages,
    } )

  def find_content( self, keyword ):
    mode = "search_page"
    if PageModel.find( keyword ):
      self.redirect( ( '/pages/' + keyword ) )
    else:
      self.generate( mode + '.html', {'search_title': 'Page not found!'} )
