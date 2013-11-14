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
	c.execute("SELECT id FROM items WHERE (row_b < ? OR (row_b = ? AND col_b <= ?)) AND (row_e > ? OR (row_e = ? AND col_e >= ?)) " +
        "ORDER BY row_b DESC, col_b DESC, row_e ASC, col_e ASC LIMIT 1", [line, line, col + 1, line, line, col + 1])
	res = c.fetchone()
	if not res:
		return None
	return res[0]

def buildDB(str):
  os.system("%s 2> %s > %s" % (str, "/dev/null", "/dev/null"))
  return