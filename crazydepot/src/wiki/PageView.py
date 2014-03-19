from common import *
from page import *
from user import *

sys.path.append(os.sep.join((os.getcwd(),'../')))
import genshi
import creoleparser
from creoleparser.dialects import create_dialect, creole10_base, creole11_base
from creoleparser.core import Parser

wiki_parser = Parser(dialect=create_dialect(creole11_base), method='xhtml')

class PagesHandler( BaseRequestHandler ):
  def get( self, page_name , prefix = "/pages" ):
    values = {}
    self.prefix = prefix
    if not page_name:
      page_name = "MainPage"
      
    # find the page
    page_name = page_name.lower()
    query = PageModel.all();
    query.filter( 'name =', page_name );
    page = query.get()
    
    if not page:
      # page not created yet, creating a new one
      values.update( self.create_page(page_name) )
      mode = "edit"
    else:
      # load a page already exists
      modes = ['view', 'edit']
      mode = self.request.get( 'mode' )
      version = self.request.get( 'version' )
      if version:
        version = int( version )
      else:
        version = -1
        
      if not mode in modes:
        mode = "view"
      if not version:
        version = -1
      values.update( self.load_page( page, mode, version ) )

    version_his = PageVersionModel.all()
    version_his.filter("page =", page)
    version_his.order("-date")
    version_his.fetch(50)
    
    values.update({
                  'page': page,
                  'edit_url': self.edit_url( page_name ),
                  'page_versions': version_his
                  })
    self.generate( mode + '.html', values )

  def post( self, page_name, prefix = "/pages" ):
    self.prefix = prefix
    if not users.get_current_user():
      # The GET version of this URI is the view/edit mode, which is a
      # reasonable thing to redirect to
      self.redirect( users.create_login_url( self.request.uri ) )
      return

    self.update_page( page_name )
    self.update_user_info( users.get_current_user(), page_name )
    self.redirect( self.view_url( page_name ) )
    
  def load_page(self, page, mode, version):
    values = {}
    query = PageVersionModel.all()
    query.filter('page =', page)
    if version < 0 or version > page.latest_version:
      version = page.latest_version
    else:
      values['before_view_content'] = '<strong>You are viewing a history version of this page.</strong>'
    query.filter('version =', version)
    wiki_version = query.get()
    
    if wiki_version:
      if mode == "view":
        values['content'] = wiki_version.content
      else:
        values['source'] = wiki_version.source
      
      values['current_version'] = "Version: %s" % wiki_version.version
    values['view_url'] = self.view_url(page.name)  
    return values
    
  def create_page(self, page_name):
    values = {}
    if not users.get_current_user():
        # require use to login before creating
        self.redirect( users.create_login_url( self.request.uri ) )
        return values
    values['source'] = self.default_page_content(page_name)
    values['before_edit_form'] = 'The page <strong>%s</strong> does not exist, Creating a new one.' % page_name
    values['view_url'] = '/pages/'
    return values

  def update_page( self, page_name ):
    # We need an explicit page name for editing
    if not page_name:
      self.redirect( '/' )

    # Create or update the page
    wiki_page = PageModel.find( page_name )
    if not wiki_page:
      wiki_page = PageModel( key_name = page_name,
                             name = page_name,
                             creator = users.get_current_user(),
                             latest_version = 1 )
      wiki_page.author = users.get_current_user()
      wiki_page.latest_version = 1
    else:
      wiki_page.latest_version = wiki_page.latest_version + 1
    
    wiki_version = PageVersionModel( page = wiki_page,
                            version = wiki_page.latest_version,
                            author = users.get_current_user(),
                            source = self.request.get( 'wikicontent' ))
    wiki_version.content = wiki_parser(wiki_version.source)
    wiki_version.put()
    wiki_page.put()

  def update_user_info( self, google_user, page_name ):
    ''' Record the information of users who modified contents on this
    wiki site
    '''
    
    if not google_user or not page_name:
      return
    
    wiki_user = UserModel.find( google_user )
    if not wiki_user:
      wiki_user = UserModel( user_object = google_user )
    wiki_user.commit_count = wiki_user.commit_count + 1
    if not page_name in wiki_user.pages:
      wiki_user.pages.append( page_name )
    wiki_user.put()

  def edit_url( self, page_name ):
    return ( self.prefix + '/%s?mode=edit' ) % ( page_name, )

  def view_url( self, page_name ):
    return self.prefix + '/' + page_name
  
  def default_page_content( self, page_name ):
    return '==%s==' % page_name
