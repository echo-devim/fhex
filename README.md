# Fhex - A Fucking HexEditor

This project is born with the aim to develop a lightweight, but useful tool. The reason is that the existing hex editors have some different limitations (e.g. too many dependencies, missing hex coloring features, etc.).

![screenshot](screenshot.png) 

It is still under development, but actually it has the basic features to make it acceptable. 

This project is based on **qhexedit2**. New features should be added in the future, PRs are welcomed.

## Features

* Search and replace (UTF-8, HEX, regex, inverse search supported) [`CTRL + F`]
* Colored output (white spaces, ASCII characters, 0xFF, UTF-8 and NULL bytes have different colors)
* Interpret selected bytes as integer, long, unsigned long [`CTRL + B`]
* Copy & Paste  [`CTRL + C` and  `CTRL + V`]
* Copy selected unicode characters [`CTRL + D`]
* Undo & Redo [`CTRL + Z` and `CTRL + Y`]
* Drag & Drop (*Hint:* Drag&Dropping two files the editor will diff them)
* Overwrite the same file or create a new one  [`CTRL + S`]
* Big files supported
* Goto offset  [`CTRL + G`]
* Insert mode supported in order to insert new bytes instead to overwrite the existing one [`INS`]
* Create new instances [`CTRL + N`]
* Basic text viewer for the selected text [`CTRL + T`]
* Reload the current file [`F5`]
* Compare two different files at byte level
* Pattern Matching Engine (see later)
* Shortcuts for all these features

## Pattern Matching Engine
Fhex can read a configuration file (from `~/fhex/config.json`) in JSON format with a list of regex to highlight and a comment/label to add close to the matches.

Example that highlights urls with a basic regex and emails:
```json
{
    "PatternMatching":
    [
        {
            "regex" : "www.[a-zA-Z0-9].",
            "color" : "rgba(250,200,200,50)",
            "message" : "Found url"
        },
        {
            "regex" : "^[a-zA-Z0-9.!#$%&'*+/=?^_`{|}~-]+@[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(?:\\.[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$",
            "color" : "rgba(250,200,200,50)",
            "message" : "Found email address"
        }
    ]
}
```
To activate pattern matching press `CTRL + P`

### License
GPL-3
