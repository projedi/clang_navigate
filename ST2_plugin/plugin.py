import sublime, sublime_plugin

class VimideCommand(sublime_plugin.TextCommand):
	def getCarret(self):
		sel = self.view.sel()
		if len(sel) != 1:
			return None
		if not sel[0].empty():
			return None
		return sel[0].begin()

	def run(self, edit):
		filaname = self.view.file_name()

		offset = self.getCarret()
		if offset == None:
			return
		row, col = self.view.rowcol(offset)

		# call to clang or database
		self.data1 = [
			['1: Variant 1', '// some cool 1 description', 'text1 text1 text1 text1 text1 text1 text1 text1 text1 text1 text1 text1 text1 text1 text1 text1 text1 text1 text1 text1 text1 text1'],
			['2: Variant 2', '// some cool 2 description', 'text2 text2 text2 text2 text2 text2 text2 text2 text2 text2 text2 text2 text2 text2 text2 text2 text2 text2 text2 text2 text2 text2'],
			['3: Variant 3', '// some cool 3 description', 'text3 text3 text3 text3 text3 text3 text3 text3 text3 text3 text3 text3 text3 text3 text3 text3 text3 text3 text3 text3 text3 text3'],
			['4: Variant 4', '// some cool 4 description', 'text4 text4 text4 text4 text4 text4 text4 text4 text4 text4 text4 text4 text4 text4 text4 text4 text4 text4 text4 text4 text4 text4'],
			['13: Variant', 'some not so long text', 'with few', 'lines']
					 ]
		self.data2 = [1, 2, 3, 4]

		self.view.window().show_quick_panel(self.data1, self.on_done)

		return
		# self.view.show() # Point, Region

	def on_done(self, index):
		if index == -1:
			return
		if self.view.window().active_view():
			self.view.window().active_view().run_command("goto_line", {"line": self.data2[index]} )


