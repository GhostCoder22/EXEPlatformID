# EXEPlatformID
EXE Platform Identifier

This tool will attempt to identify what OS platform the selected executable requires.
I'm developing this tool in the hopes someone will find it useful.

Parts of EXEPlatformID is based on source code provided by answer on StackOverflow
(https://stackoverflow.com/questions/46024914/how-to-parse-exe-file-and-get-data-from-image-dos-header-structure-using-c-and).
Because of this, I do own nor will I claim to own the code provided by said answer code.

Currently the only build project is Code::Blocks with TDM-GCC (which is essentially MinGW32-x64)
but this source code should compile with virtually any IDE/compiler toolset combo for Windows or
on any Linux distro (provided you have WINE installed) with little to no effort.
