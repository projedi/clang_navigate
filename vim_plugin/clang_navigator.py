import vim
import os
	
def gotoDeclaration():
  line, col = vim.current.window.cursor
  vim.current.window.cursor = (line - 3, col - 1)
  
def gotoDefinition():
  vim.command("normal m'")
  vim.current.window.cursor = (1, 0)
  
