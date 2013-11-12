import vim
import utils_cn
# DEF = 0 
# DECL = 1
# USAGE = 2 

def initClangNavigator():
	return
	
def jumpToLocation(filename, line, column):
  if filename != vim.current.buffer.name:
    try:
      vim.command("edit %s" % filename)
    except:
      # For some unknown reason, whenever an exception occurs in
      # vim.command, vim goes crazy and output tons of useless python
      # errors, catch those.
      return
  else:
    vim.command("normal m'")
  vim.current.window.cursor = (line, column - 1)	
  
def gotoDefDecl(type):
  line, col = vim.current.window.cursor
  idx = utils_cn.getItem(line, col)
  if idx is None:
  	return
  filename, line, col = utils_cn.getPosition(idx, type)
  if filename is None:
  	return
  jumpToLocation(filename, line, col)
  
  
def dbRebuild(script):
  filename = vim.current.buffer.name
  utils_cn.buildDB(script + ' ' + filename)
  return