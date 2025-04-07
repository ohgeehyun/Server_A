pushd %~dp0

protoc.exe -I=./ --cpp_out=./ --csharp_out=./ ./Protocol.proto
protoc.exe -I=./ --cpp_out=./ ./ServerProtocol.proto

GenPackets.exe --path=./Protocol.proto --output=ClientPacketHandler --recv=C_ --send=S_
GenPackets_ServerToServer.exe --path=./ServerProtocol.proto --output=ServerPacketHandler --recv=S_ --send=C_
GenPackets_ServerToServer.exe --path=./ServerProtocol.proto --output=RoomPacketHandler --recv=C_ --send=S_

IF ERRORLEVEL 1 PAUSE

XCOPY /Y Protocol.pb.h "../../../Server_A"
XCOPY /Y Protocol.pb.cc "../../../Server_A"

XCOPY /Y ServerProtocol.pb.h "../../../Server_A"
XCOPY /Y ServerProtocol.pb.cc "../../../Server_A"

XCOPY /Y ServerProtocol.pb.h "../../../RoomServer"
XCOPY /Y ServerProtocol.pb.cc "../../../RoomServer"

XCOPY /Y ClientPacketHandler.h "../../../Server_A"
XCOPY /Y ServerPacketHandler.h "../../../Server_A"

XCOPY /Y RoomPacketHandler.h "../../../RoomServer"

XCOPY /Y Protocol.cs "G:\unity\My project\Assets\Script\Network\ProtoPacket"


DEL /Q /F *.pb.h
DEL /Q /F *.pb.cc
DEL /Q /F *.h

PAUSE