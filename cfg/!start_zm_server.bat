@echo off

set ServerFilename=server_zm.cfg

:://///////////////////////////////////////////////////////////////////////////
::// 	What port do you want the server to run on?			  		         //
:://	You must port forward this port & allow it through your firewall     //
:://///////////////////////////////////////////////////////////////////////////
set port=27016

:: Either put the batch in the Ghosts install dir, or change the value below to the Ghost install dir
set GHOSTS_INSTALL=%~dp0

::///////////////////////////////////////////////////////////////////////
:://You're done!! WARNING!!! Don't mess with anything below this line  //
::///////////////////////////////////////////////////////////////////////

set GAME_EXE=iw6-mod.exe
start %GAME_EXE% -dedicated +set zombiesMode 1 +set net_port "%port%" +exec %ServerFilename% +map_rotate
