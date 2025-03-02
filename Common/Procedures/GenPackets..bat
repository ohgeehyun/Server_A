pushd %~dp0

GenProcs.exe --path=../../Server_A/GameDB.xml --output=GenProcedures.h

IF ERRORLEVEL 1 PAUSE

XCOPY /Y GenProcedures.h "../../Server_A"

DEL /Q /F *.h

PAUSE