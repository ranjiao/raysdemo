import re

class BaseConverter():
  def __init__( self ):
    self.output = ''

  def convert( self, string ):
    self.output = ''
    for line in string.splitlines():
      self.deal( line )
    return self.output

  def deal( self, line ):
    patterns1 = {
                r'<h1>':r'^\*\ ',
                r'<h2>':r'^\*\*\ ',
                r'<h3>':r'^\*\*\*\ ',
                r'<h4>':r'^\*\*\*\*\ ',
                }
    patterns2 = {
                 r'<hr/>':r'----',
                 }
    patterns3 = {
                 r'<pre>':r'(^\ \ .*$){1,50}',
                 }
    output = line

    for key in patterns1:
      if re.search( patterns1[key], line ):
        content = re.sub( patterns1[key], '', line ).strip()
        output = r'%s%s%s' % ( key, content, key )
        break

    if output != line:
      for key in patterns2:
        if re.search( patterns2[key], line ):
          content = re.sub( patterns2[key], '', line ).strip()
          output = key
          break

    if output != line:
      for key in patterns3:
        if re.search( patterns3[key], line ):
          content = re.sub( patterns3[key], '', line ).strip()
          output = key
          break
    self.output = self.output + output + '\n'

if __name__ == '__main__':
  test_str = '''
  here are some quotes
  and this is useful
* This is a big title   
and some thing here
----
* Another big title
** Subtitle
if you wanna something more
*** Subsubtitle
and it's here
'''
  converter = BaseConverter()
  print( str( converter.convert( test_str ) ) )
