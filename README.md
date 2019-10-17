# Fhex - A Fucking HexEditor

This project is born with the aim to develop a lightweight, but useful tool. The reason is that the existing hex editors have some different limitations (e.g. too many dependencies, missing hex coloring features, etc.).

![screenshot](screenshot.png) 

It is still under development, but actually it has the basic features to make it acceptable. 

This project is based on **qhexedit2**. New features should be added in the future, PRs are welcomed.

## Features

* Search and replace (UTF-8, HEX, regex, inverse search supported) [`CTRL + F`]
* Colored output (white spaces, ASCII characters, UTF-8 and NULL bytes have different colors)
* Interpret selected bytes as integer, long, unsigned long [`CTRL + B`]
* Copy & Paste  [`CTRL + C` and  `CTRL + V`]
* Drag & Drop
* Basic comparison between two binaries
* Overwrite the same file or create a new one  [`CTRL + S`]
* Big files supported
* Goto offset  [`CTRL + G`]
* Insert mode supported in order to insert new bytes instead to overwrite the existing one [`INS`]
* Create new istances [`CTRL + N`]
* Basic text viewer for the selected text [`CTRL + T`]
* Reload the current file [`F5`]
* Compare two different files at byte level (**Work In Progress**)
* Shortcuts for all these features

### License
GPL-3
