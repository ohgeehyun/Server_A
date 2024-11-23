#include "pch.h"
#include "MapManager.h"
#include "Protocol.pb.h"
#include "Player.h"
#include <fstream>
#include <string>


bool MapManager::CanGo(Vector2Int cellPos ,bool checkObject)
{
    if (cellPos.posx < _MinX || cellPos.posx > _MaxX)
        return false;
    if (cellPos.posy < _MinY || cellPos.posy > _MaxY)
        return false;

    int32 x = cellPos.posx - _MinX;
    int32 y = _MaxY - cellPos.posy;

    return !_collision[y][x] && (!checkObject || _players[y][x] == nullptr);
}

void MapManager::LoadMap(int32 mapId)
{

    // 맵 이름 생성
    std::string mapName = "Map_" +
        (mapId < 10 ? std::string("00") :
        (mapId < 100 ? std::string("0") : std::string(""))) +
         std::to_string(mapId);

    string pathPrefix = "../Common/MapData";

    // 파일 읽기
    ifstream file(pathPrefix +"/" + mapName + ".txt");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file.\n";
        return;
    }

    // 첫 4줄 읽어서 Min/Max 값을 설정
    file >> _MinX >> _MaxX >> _MinY >> _MaxY;

    // xCount와 yCount 계산
    int xCount = _MaxX - _MinX + 1;
    int yCount = _MaxY - _MinY + 1;

    // _collision 배열 초기화
    _collision.resize(yCount, Vector<bool>(xCount, false));
    _players.resize(yCount,Vector<PlayerRef>(xCount));

    // 파일에서 충돌 데이터를 읽어들임
    for (int y = 0; y < yCount; ++y) {
        std::string line;
        file >> line; // 한 줄 읽기
        for (int x = 0; x < xCount; ++x) {
            _collision[y][x] = (line[x] == '1');
        }
    }

    file.close();
    
}

PlayerRef MapManager::Find(Vector2Int cellPos)
{
    if (cellPos.posx < _MinX || cellPos.posx > _MaxX)
        return nullptr;
    if (cellPos.posy < _MinY || cellPos.posy > _MaxY)
        return nullptr;

    int32 x = cellPos.posx - _MinX;
    int32 y = _MaxY - cellPos.posy;
    return _players[y][x];
}

bool MapManager::ApplyMove(PlayerRef& player, Vector2Int dest)
{
    Protocol::POSITIONINFO* posInfo = player->GetObjectInfo().mutable_posinfo();

    if (posInfo->posx() < _MinX || posInfo->posx() > _MaxX)
        return false;
    if (posInfo->posy() < _MinY || posInfo->posy() > _MaxY)
        return false;

    if (CanGo(dest, true) == false)
        return false;
    {
        int32 x = posInfo->posx() - _MinX;
        int32 y = _MaxY - posInfo->posy();
        if (_players[y][x] == player) 
        {
            cout << "위치 삭제된 플레이어  id :" << _players[y][x]->GetObjectInfo().objectid();
            cout << "위치 삭제된 플레이어 위치:" << _players[y][x]->GetObjectInfo().posinfo().posx() << "," << _players[y][x]->GetObjectInfo().posinfo().posy() << endl;
            _players[y][x] = nullptr;
        }
    }
    {
        int32 x = dest.posx - _MinX;
        int32 y = _MaxY - dest.posy;
        _players[y][x] = player;

    }
    //실제 좌표 이동 
    posInfo->set_posx(dest.posx);
    posInfo->set_posy(dest.posy);
    
    return true;
}


Vector<Vector2Int> MapManager::FindPath(Vector2Int startCellPos, Vector2Int destCellPos, bool ignoreDestCollision) {
    const int32 _deltaY[] = { 1, -1, 0, 0 }; // U D
    const int32 _deltaX[] = { 0, 0, -1, 1 }; // L R
    const int32 _cost[] = { 10, 10, 10, 10 };

    Vector<Vector<bool>> closed(_sizeY, Vector<bool>(_sizeX, false));
    Vector<Vector<int32>> open(_sizeY, Vector<int32>(_sizeX, INT_MAX));
    Vector<Vector<Pos>> parent(_sizeY, Vector<Pos>(_sizeX, Pos()));

    std::priority_queue<PQNode> pq;

    Pos pos = Cell2Pos(startCellPos);
    Pos dest = Cell2Pos(destCellPos);

    open[pos.Y][pos.X] = 10 * (std::abs(dest.Y - pos.Y) + std::abs(dest.X - pos.X));
    pq.push({ open[pos.Y][pos.X], 0, pos.Y, pos.X });
    parent[pos.Y][pos.X] = pos;

    while (!pq.empty()) {
        PQNode node = pq.top();
        pq.pop();

        if (closed[node.Y][node.X])
            continue;

        closed[node.Y][node.X] = true;

        if (node.Y == dest.Y && node.X == dest.X)
            break;

        for (int32 i = 0; i < 4; ++i) {
            Pos next(node.Y + _deltaY[i], node.X + _deltaX[i]);

            if (!ignoreDestCollision || (next.Y != dest.Y || next.X != dest.X)) {
                if (!CanGo(Pos2Cell(next)))
                    continue;
            }

            if (closed[next.Y][next.X])
                continue;

            int32 g = node.G + _cost[i];
            int32 h = 10 * ((dest.Y - next.Y) * (dest.Y - next.Y) + (dest.X - next.X) * (dest.X - next.X));

            if (open[next.Y][next.X] <= g + h)
                continue;

            open[next.Y][next.X] = g + h;
            pq.push({ g + h, g, next.Y, next.X });
            parent[next.Y][next.X] = { node.Y, node.X };
        }
    }

    return CalcCellPathFromParent(parent, dest);
}

Pos MapManager::Cell2Pos(const Vector2Int& cell) const
{
    return Pos(_MaxY - cell.posy, cell.posx - _MinX);
}


Vector2Int MapManager::Pos2Cell(const Pos& pos) const
{
    return Vector2Int(pos.X + _MinX, _MaxY - pos.Y);
}

// 부모 데이터를 통해 최종 경로 계산
Vector<Vector2Int> MapManager::CalcCellPathFromParent(const Vector<Vector<Pos>>& parent, const Pos& dest) {
    Vector<Vector2Int> cells;

    int32 y = dest.Y, x = dest.X;
    while (parent[y][x].Y != y || parent[y][x].X != x) {
        cells.push_back(Pos2Cell({ y, x }));
        Pos p = parent[y][x];
        y = p.Y;
        x = p.X;
    }
    cells.push_back(Pos2Cell({ y, x }));
    std::reverse(cells.begin(), cells.end());

    return cells;
}

