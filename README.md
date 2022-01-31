# EXEPlatformID
EXE Platform Identifier

This tool will attempt to identify what OS platform the selected executable requires.
I'm developing this tool in the hopes someone will find it useful.

Parts of EXEPlatformID is based on source code provided as an answer on StackOverflow
(https://stackoverflow.com/questions/46024914/how-to-parse-exe-file-and-get-data-from-image-dos-header-structure-using-c-and).
Because of this, I do own nor will I claim to own the code provided by said answer code.

Currently the only build project is Code::Blocks with TDM-GCC (which is essentially MinGW32-x64)
but this source code should compile with virtually any IDE/compiler toolset combo for Windows with
little to no effort.

A NOTE OF WARNING: While it might be possible for Linux users to compile this EXEPlatformID provided
WINE is installed, I make no garentee's as to the correctiveness of OS platform reporting produced by
this tool.
