#pragma once
class RedisConnection;
class RoomManager;
extern std::shared_ptr<RedisConnection> GRedisConnection;
extern std::shared_ptr<RoomManager> GRoomManager;