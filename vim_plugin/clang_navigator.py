import vim
import os
import threading
import sqlite3

# Declaration type = 1, Definition type = 2	
def initClangNavigator():
	return
	
	
def getPosition(idx, type):
	dbname = "testdb.db"
	conn = sqlite3.connect(dbname)
	c = conn.cursor()
	c.execute("SELECT file, row, col FROM items WHERE id = ? AND type = ?", [idx, type])
	res = c.fetchone()
	if res is None:
		return None, None, None
	return  res

def getItem(line, col):
	dbname = "testdb.db"
	conn = sqlite3.connect(dbname)
	c = conn.cursor()
	c.execute("SELECT id, row, col FROM items WHERE (col <= ?) AND (col + col_offset >= ?) AND (row <= ?) AND (row + row_offset >= ?)", [col, col, line, line])
	res = c.fetchall()
	if not res:
		return None
	res = sorted(res, key = lambda x: x[1], reverse = True)
	return res[0][0]
	
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

def gotoDeclaration():
  line, col = vim.current.window.cursor
  idx = getItem(line, col)
  if idx is None:
  	return
  filename, line, col = getPosition(idx, 1) 
  if filename is None:
  	return
  jumpToLocation(filename, line, col)
  
def gotoDefinition():
  line, col = vim.current.window.cursor
  idx = getItem(line, col)
  if idx is None:
  	return
  filename, line, col = getPosition(idx, 2)
  if filename is None:
  	return
  jumpToLocation(filename, line, col)
  
  
def dbRebuild():
	return
