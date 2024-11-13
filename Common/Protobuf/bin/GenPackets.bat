pushd %~dp0
protoc.exe -I=./ --cpp_out=./ --csharp_out=./ ./Enum.proto
protoc.exe -I=./ --cpp_out=./ --csharp_out=./ ./Struct.proto
protoc.exe -I=./ --cpp_out=./ --csharp_out=./ ./Protocol.proto

GenPackets.exe --path=./Protocol.proto --output=ClientPacketHandler --recv=C_ --send=S_


IF ERRORLEVEL 1 PAUSE

XCOPY /Y Enum.pb.h "../../../Server_A"
XCOPY /Y Enum.pb.cc "../../../Server_A"
XCOPY /Y Struct.pb.h "../../../Server_A"
XCOPY /Y Struct.pb.cc "../../../Server_A"
XCOPY /Y Protocol.pb.h "../../../Server_A"
XCOPY /Y Protocol.pb.cc "../../../Server_A"
XCOPY /Y ClientPacketHandler.h "../../../Server_A"

XCOPY /Y Struct.cs "G:\unity\My project\Assets\Script\Network\ProtoPacket"
XCOPY /Y Enum.cs "G:\unity\My project\Assets\Script\Network\ProtoPacket"
XCOPY /Y Protocol.cs "G:\unity\My project\Assets\Script\Network\ProtoPacket"


DEL /Q /F *.pb.h
DEL /Q /F *.pb.cc
DEL /Q /F *.h

PAUSE