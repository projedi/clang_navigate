import sublime, sublime_plugin

import sqlite3

dbname = "vimide.db"

def getRecords(idx, type):
    conn = sqlite3.connect(dbname)
    c = conn.cursor()
    c.execute("SELECT file, row_b, col_b, row_e, col_e, data FROM items WHERE id = ? AND type = ?", [idx, type])
    res = c.fetchall()
    return res

def getItemId(row, col):
    conn = sqlite3.connect(dbname)
    c = conn.cursor()
    c.execute("SELECT id FROM items WHERE (row_b < ? OR (row_b = ? AND col_b <= ?)) AND (row_e > ? OR (row_e = ? AND col_e >= ?)) " +
        "ORDER BY row_b DESC, col_b DESC, row_e ASC, col_e ASC LIMIT 1", [row, row, col, row, row, col])
    res = c.fetchone()
    if not res:
        print "VIM_IDE: record id not found"
        return None
    return res[0]

def getCarret(sel):
        if len(sel) != 1:
            print "VIM_IDE: multiple cursors not supported"
            return None
        if not sel[0].empty():
            print "VIM_IDE: selection not supported"
            return None
        return sel[0].begin()

def goto(view, pos):
    (filename, row_b, col_b, row_e, col_e) = pos

    view.run_command("goto_pos", {"filename": filename, "row_b": row_b, "col_b": col_b, "row_e": row_e, "col_e": col_e} )

class GotoPosCommand(sublime_plugin.TextCommand):
    def run(self, edit, filename, row_b, col_b, row_e, col_e):
        view = self.view.window().open_file(filename);

        pt_b = view.text_point(row_b - 1, col_b)
        pt_e = view.text_point(row_e - 1, col_e)

        r = sublime.Region(pt_b, pt_e)

        view.sel().clear()
        view.sel().add(r)

        view.show(r)

class VimideDefinitionCommand(sublime_plugin.TextCommand):
    def run(self, edit):
        filaname = self.view.file_name()

        offset = getCarret(self.view.sel())
        if offset == None:
            return
        row, col = self.view.rowcol(offset)

        idx = getItemId(row + 1, col)
        if idx is None:
            return
        records = getRecords(idx, 0)

        if records is None or len(records) == 0:
            return

        goto(self.view, (records[0][0], records[0][1], records[0][2], records[0][3], records[0][4]) )

class VimideDeclarationCommand(sublime_plugin.TextCommand):
    def run(self, edit):
        filaname = self.view.file_name()

        offset = getCarret(self.view.sel())
        if offset == None:
            return
        row, col = self.view.rowcol(offset)

        idx = getItemId(row + 1, col)
        if idx is None:
            return
        records = getRecords(idx, 1)

        if records is None or len(records) == 0:
            return

        self.data1 = []
        self.data2 = []

        for record in records:
            self.data1.append([str(record[0]) + ':' + str(record[1]) + ':' + str(record[2]), record[5]])
            self.data2.append((record[0], record[1], record[2], record[3], record[4]))

        self.view.window().show_quick_panel(self.data1, self.on_done)

        return

    def on_done(self, index):
        if index == -1:
            return
        goto(self.view, self.data2[index][0])

class VimideUsagesCommand(sublime_plugin.TextCommand):
    def run(self, edit):
        filaname = self.view.file_name()

        offset = getCarret(self.view.sel())
        if offset == None:
            return
        row, col = self.view.rowcol(offset)

        idx = getItemId(row + 1, col)
        if idx is None:
            return
        records = getRecords(idx, 2)

        if records is None or len(records) == 0:
            return

        self.data1 = []
        self.data2 = []

        for record in records:
            self.data1.append([str(record[0]) + ':' + str(record[1]) + ':' + str(record[2]), record[5]])
            self.data2.append((record[0], record[1], record[2], record[3], record[4]))

        self.view.window().show_quick_panel(self.data1, self.on_done)

        return

    def on_done(self, index):
        if index == -1:
            return
        goto(self.view, self.data2[index])
