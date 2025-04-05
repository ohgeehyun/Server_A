#include "pch.h"
#include "MapManager.h"
#include "Protocol.pb.h"
#include "Player.h"
#include <fstream>
#include <string>
#include "ObjectManager.h"



bool MapManager::CanGo(Vector2Int cellPos, bool checkObject)
{
    if (cellPos.posx < _MinX || cellPos.posx > _MaxX)
        return false;
    if (cellPos.posy < _MinY || cellPos.posy > _MaxY)
        return false;

    int32 x = cellPos.posx - _MinX;
    int32 y = _MaxY - cellPos.posy;

    return !_collision[y][x] && (!checkObject || _objects[y][x] == nullptr);
}

void MapManager::LoadMap(int32 mapId)
{

    // 맵 이름 생성
    std::string mapName = "Map_" + (mapId < 10 ? std::string("00") : (mapId < 100 ? std::string("0") : std::string(""))) + std::to_string(mapId);

    string pathPrefix = "../Common/MapData";

    // 파일 읽기
    ifstream file(pathPrefix + "/" + mapName + ".txt");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file.\n";
        return;
    }

    // 첫 4줄 읽어서 Min/Max 값을 설정
    file >> _MinX >> _MaxX >> _MinY >> _MaxY;

    // xCount와 yCount 계산
    int32 xCount = _MaxX - _MinX + 1;
    int32 yCount = _MaxY - _MinY + 1;
    _sizeX = xCount;
    _sizeY = yCount;

    // _collision 배열 초기화
    _collision.resize(yCount, Vector<bool>(xCount, false));
    _objects.resize(yCount, Vector<GameObjectRef>(xCount));

    // 파일에서 충돌 데이터를 읽어들임
    for (int32 y = 0; y < yCount; ++y) {
        std::string line;
        file >> line; // 한 줄 읽기
        for (int32 x = 0; x < xCount; ++x) {
            _collision[y][x] = (line[x] == '1');
        }
    }

    file.close();
}

GameObjectRef MapManager::Find(Vector2Int cellPos)
{
    if (cellPos.posx < _MinX || cellPos.posx > _MaxX)
        return nullptr;
    if (cellPos.posy < _MinY || cellPos.posy > _MaxY)
        return nullptr;

    int32 x = cellPos.posx - _MinX;
    int32 y = _MaxY - cellPos.posy;
    return _objects[y][x];
}

bool MapManager::ApplyMove(const GameObjectRef& gameobject, Vector2Int dest)
{
    if (CanGo(dest, true) == false)
        return false;

    ApplyLeave(gameobject);

    {
        int32 x = dest.posx - _MinX;
        int32 y = _MaxY - dest.posy;
        _objects[y][x] = gameobject;

        if (gameobject->GetGameObjectType() == Protocol::MONSTER)
        {
            cout << "현재 Monster : " << y << "," << x << "\n";
        }
    }
    //실제 좌표 이동 
    gameobject->SetPosx(dest.posx);
    gameobject->SetPosy(dest.posy);

    return true;
}

bool MapManager::ApplyLeave(const GameObjectRef& gameObject)
{
    if (gameObject->GetRoom() == nullptr)
        return false;

    Protocol::POSITIONINFO* posInfo = gameObject->GetObjectInfo().mutable_posinfo();

    if (gameObject->GetPosx() < _MinX || gameObject->GetPosx() > _MaxX)
        return false;
    if (gameObject->GetPosy() < _MinY || gameObject->GetPosy() > _MaxY)
        return false;

    int32 x = gameObject->GetPosx() - _MinX;
    int32 y = _MaxY - gameObject->GetPosy();
    if (_objects[y][x] == gameObject)
    {
        cout << "위치 삭제된 오브젝트 id :" << _objects[y][x]->GetObjectId();
        cout << "  오브젝트 위치:" << _objects[y][x]->GetPosx() << "," << _objects[y][x]->GetPosy() << "\n";
        _objects[y][x] = nullptr;
    }
    return true;
}

Vector<Vector2Int> MapManager::FindPath(Vector2Int startCellPos, Vector2Int destCellPos, bool checkObjects) {
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

            if (next.Y != dest.Y || next.X != dest.X) {
                if (!CanGo(Pos2Cell(next), checkObjects))
                    continue;
            }

            if (closed[next.Y][next.X])
                continue;

            int32 g = node.G + _cost[i];
            int32 h = 10 * abs(dest.Y - next.Y) + abs(dest.X - next.X);

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

    int32 y = dest.Y;
    int32 x = dest.X;
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

