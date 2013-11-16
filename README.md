clang_navigate
==============

Navigating C code using clang API. (AU devdays project)

## Requirements
* clang v3.3

## Building
* make

## Usage
#### from cmd: 
`./clang_navigate <filename>`

#### plugin for Vim: 

copy files *clang_navigate* and *clang_navigator.py*, *utils_cn.py*, *clang_navigator.vim* from vim_plugin/ to $HOME/.vim/plugin/

`<Ctrl-B>` - Create DB

`<Ctrl-K>` - goto Declaration

`<Ctrl-J>` - goto Definition

#### plugin for Sublime-Text2: 

copy *vimide.py* to $HOME/.config/sublime-text-2/Packages/User

Add Key Bindings:

  { "keys": ["*hotkey1*"], "command": "vimide_definition" },

  { "keys": ["*hotkey2*"], "command": "vimide_declaration" },

  { "keys": ["*hotkey3*"], "command": "vimide_usages" }

Before using create DB `./clang_navigate` 
