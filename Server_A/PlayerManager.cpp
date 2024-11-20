#include "pch.h"
#include "Player.h"
#include "Protocol.pb.h"
#include "PlayerManager.h"

PlayerRef& PlayerManager::Add()
{
    PlayerRef player = Make_Shared<Player>();

    WRITE_LOCK
    {
       player->GetPlayerInfo().set_playerid(_playerid);
       _players[_playerid] = player;
       _playerid++;
    }

    return player;
}

bool PlayerManager::Remove(int32 playerId)
{
    WRITE_LOCK
    {
         return _players.erase(playerId);
    }
}

PlayerRef& PlayerManager::Find(int32 playerId)
{
    WRITE_LOCK
    {
        PlayerRef player = nullptr;
        for (auto it = _players.begin(); it != _players.end(); ++it)
        {
            if (it->first == playerId)
                player = it->second;
        }
        return player;
    }
}
