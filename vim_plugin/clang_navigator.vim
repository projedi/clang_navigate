
au FileType c,cpp,objc,objcpp call <SID>ClangNavigatorInit()

let s:plugin_path = escape(expand('<sfile>:p:h'), '\')

function! s:ClangNavigatorInit()

	if s:initPython() != 1
    return
  endif
	if !exists('g:clang_jumpto_declaration_key')
    let g:clang_jumpto_declaration_key = '<C-K>'
  endif
  if !exists('g:clang_jumpto_definition_key')
    let g:clang_jumpto_definition_key = '<C-J>'
  endif
  if !exists('g:clang_rebuild_db_key')
    let g:clang_rebuild_db_key = '<C-B>'
  endif
  
  execute "nnoremap <buffer> <silent> " . g:clang_jumpto_declaration_key . " :call <SID>GotoDeclaration()<CR><Esc>"  
  execute "nnoremap <buffer> <silent> " . g:clang_jumpto_definition_key . " :call <SID>GotoDefinition()<CR><Esc>"
  
  execute "nnoremap <buffer> <silent> " . g:clang_rebuild_db_key . " :call <SID>DBRebuild()<CR><Esc>"
  
endfunction


function! s:initPython()
  if !has('python')
    echoe 'No python support available.'
    return 0
  endif

  " Only parse the python library once
  if !exists('s:lib_navigator_loaded')
    python import sys

    exe 'python sys.path = ["' . s:plugin_path . '"] + sys.path'
    exe 'pyfile ' . fnameescape(s:plugin_path) . '/clang_navigator.py'

    let s:lib_navigator_loaded = 1
  endif
  return 1
endfunction


function! s:GotoDeclaration()
  try
    python gotoDeclaration()
  catch /^Vim\%((\a\+)\)\=:E37/
    echoe "The current file is not saved, and 'hidden' is not set."
          \ "Either save the file or add 'set hidden' in your vimrc."
  endtry
  return ''
endfunction

function! s:GotoDefinition()
try
    python gotoDefinition()
  catch /^Vim\%((\a\+)\)\=:E37/
    echoe "The current file is not saved, and 'hidden' is not set."
          \ "Either save the file or add 'set hidden' in your vimrc."
  endtry
  return ''
endfunction

function! s:LaunchCompletion()  
  return ''
endfunction

function! DBRebuild()
	python dbRebuild()
endfunction
