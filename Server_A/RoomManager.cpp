#include "pch.h"
#include "Room.h"
#include "RoomManager.h"
#include "RedisConnection.h"
#include "ServerProtocol.pb.h"
#include "ServerSessionManager.h"
#include "ServerPacketHandler.h"
#include "ServerSession.h"
#include "JsonUtils.h"


RoomRef RoomManager::Add(int32 mapId, string name, string pwd, string rootUser)
{
    RoomRef gameRoom = Make_Shared<Room>();
    
    WRITE_LOCK
    {
       if (pwd != "")
            gameRoom->SetPwdYn(true);

       int32 _roomid = roomid.GetRoomId();
       gameRoom->SetRoomId(_roomid);
       gameRoom->SetRoomName(name);
       gameRoom->SetRoomPwd(pwd);
       gameRoom->SetRootUser(rootUser);
       _rooms[_roomid] = gameRoom;

       // RoomServer에게 방 생성 요청
       ServerProtocol::C_CREATE_ROOM packet;
       packet.set_roomid(_roomid);
       packet.set_roomname(name);
       packet.set_roompwd(pwd);
       packet.set_pwdyn(gameRoom->GetPwdYn());
       packet.set_rootuser(rootUser);

       ServerSessionRef session = GServerSessionManager->Pop_Session();

       auto Packet = ServerPacketHandler::MakeSendBuffer(packet);
       session->Send(Packet);
           
       nlohmann::json json_obj = JsonUtils::createJson(
           std::make_pair("id", _roomid),
           std::make_pair("pwdYn",pwd != ""),
           std::make_pair("name", name.c_str()),
           std::make_pair("password", pwd.c_str()),
           std::make_pair("rootUser", rootUser.c_str())
       );
     
       std::string json_str = json_obj.dump();

       const char* query = "SET room:%d %s";
       RedisUtils::RAsyncCommand(GRedisConnection->GetContext(), query, _roomid, json_str.c_str());

       std::cout << "Room 번호 : " << _roomid << " Room 생성 방 이름 : " << name << " Room pwd : " << pwd << endl;

       gameRoom->Init(mapId);

       roomid.Add_Room();
    }
    return gameRoom;
}

//이미 만들어저있는 방을 생성하는 add 오버로딩
void RoomManager::Add(int32 mapId, string name, string pwd, int32 Roomid, string rootUser)
{
    RoomRef gameRoom = Make_Shared<Room>();

        gameRoom->SetRoomId(Roomid);
        gameRoom->SetRoomName(name);
        gameRoom->SetRoomPwd(pwd);
        gameRoom->SetRootUser(rootUser);

        _rooms[Roomid] = gameRoom;

        gameRoom->Init(mapId);
        roomid.Add_Room(Roomid);
    cout << gameRoom->GetRoomId() << " " << gameRoom->GetRoomName() << "\n";
}

bool RoomManager::Remove(int32 roomId)
{
  
    return _rooms.erase(roomId);
    
}

RoomRef RoomManager::Find(int32 roomId)
{
   RoomRef gameRoom = nullptr;
   for (auto it = _rooms.begin(); it != _rooms.end(); ++it)
   {
       if (it->first == roomId)
           gameRoom = it->second;
   }
   return gameRoom;
}

void RoomManager::DoRoomUpdate()
{
    if (_rooms.empty())
        return;

    for (const auto &item : _rooms)
    {
       //item.second->DoAsync(std::bind(&Room::Update, item.second));
        item.second->Update();
    }
}

RoomIdManager::RoomIdManager()
{
   for (int32 i = 1; i <= 100; i++)
       _roomids.insert(i);
}

RoomIdManager::~RoomIdManager()
{
}

int32 RoomIdManager::Add_Room()
{
    int32 result;

    result = *_roomids.begin();
    use_room.insert(result);
    _roomids.erase(result);
    return result;
    
}

void RoomIdManager::Add_Room(int32 Roomid)
{
    _roomids.erase(Roomid);
    use_room.insert(Roomid); 
}

void RoomIdManager::Delete_Room(int32 Roomid)
{
  use_room.erase(Roomid);
  _roomids.insert(Roomid);  
}
