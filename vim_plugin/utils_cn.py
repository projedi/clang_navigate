import sqlite3
import os

def getDBName():
  return "vimide.db"


def getPosition(idx, type):
	if not os.path.exists(getDBName()):
		return None, None, None
	conn = sqlite3.connect(getDBName())
	c = conn.cursor()
	c.execute("SELECT file, row_b, col_b FROM items WHERE id = ? AND type = ?", [idx, type])
	res = c.fetchone()
	if res is None:
		return None, None, None
	return  res

def getItem(line, col):
	if not os.path.exists(getDBName()):
		return None
	conn = sqlite3.connect(getDBName())
	c = conn.cursor()
	c.execute("SELECT id, row_b, col_b FROM items WHERE (col_b <= ?) AND (col_e >= ?) AND (row_b <= ?) AND (row_e >= ?)", [col, col, line, line])
	res = c.fetchall()
	if not res:
		return None
	res = sorted(res, key = lambda x: x[1], reverse = True)
	return res[0][0]

def buildDB(str):
  os.system(str)
  return