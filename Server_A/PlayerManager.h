#pragma once

class PlayerManager
{
public:
    static PlayerManager& GetInstance() {
        static PlayerManager instance;
        return instance;
    }

    PlayerRef& Add();
    bool Remove(int32 playerId);
    PlayerRef& Find(int32 playerId);

private:
    PlayerManager() = default;
    PlayerManager(const PlayerManager&) = delete;
    PlayerManager& operator=(const PlayerManager&) = delete;

    USE_LOCK;
    HashMap<int32, PlayerRef> _players;
    int32 _playerid = 1;
    // TODO : _playerid관리 정의가 확실하게되면 수정
    // 비트플래그 등으로 관리하면 좋을텐데
};

