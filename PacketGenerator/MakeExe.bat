pushd %~dp0
pyinstaller --onefile PacketGenerator.py
REM MOVE .\dist\PacketGenerator.exe .\GenPackets.exe
MOVE .\dist\PacketGenerator.exe .\GenPackets_ServerToServer.exe
@RD /S /Q .\build
@RD /S /Q .\dist
DEL /S /F /Q .\PacketGenerator.spec
PAUSE