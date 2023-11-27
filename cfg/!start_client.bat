@echo off

:: Either put the batch in the Ghosts install dir, or change the value below to the Ghosts install dir
set GHOSTS_INSTALL=%~dp0

::///////////////////////////////////////////////////////////////////////
:://You're done!! WARNING!!! Don't mess with anything below this line  //
::///////////////////////////////////////////////////////////////////////

set GAME_EXE=iw6-mod.exe
start %GAME_EXE%
