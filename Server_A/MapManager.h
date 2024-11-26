#pragma once

struct Pos
{
     Pos(int32 y, int32 x) { Y = y; X = x; };
     Pos() : Y(0), X(0) {};
     int32 Y;
     int32 X;
};

struct PQNode {
    int32 F;
    int32 G;
    int32 Y;
    int32 X;

    bool operator<(const PQNode& other) const {
        return F < other.F;
    }
};

class Vector2Int
{
public:
     int32 posx;
     int32 posy;

public:
   Vector2Int() : posx(0), posy(0) {}
   Vector2Int(int32 x, int32 y) { posx = x; posy = y; }
   ~Vector2Int() {}

   static Vector2Int up() { return Vector2Int(0, 1); }
   static Vector2Int down() { return Vector2Int(0, -1); }
   static Vector2Int left() { return Vector2Int(-1, 0); }
   static Vector2Int right() { return Vector2Int(+1,0); }

   Vector2Int operator+(const Vector2Int& other) const {
       return Vector2Int(posx + other.posx, posy + other.posy);
   }
   Vector2Int& operator+=(const Vector2Int& other) {
       posx += other.posx;
       posy += other.posy;
       return *this;
   }
};

class MapManager
{
public:

    MapManager() {};
    ~MapManager() {};

    int32 GetMinX() { return _MinX; }
    void SetMinX(int32 x) { _MinX = x; }

    int32 GetMaxX() { return _MaxX; }
    void SetMaxX(int32 x)  { _MaxX = x; }


    int32 GetMinY() { return _MinY; }
    void SetMinY(int32 y)  { _MinY = y; }


    int32 GetMaxY() { return _MaxY; }
    void SetMaxY(int32 y)  { _MaxY = y; }

    int32 GetSizeX() { return _sizeX; }
    int32 GetSizeY() { return _sizeY; }

    bool CanGo(Vector2Int cellPos, bool checkObject = true);
    void  LoadMap(int32 mapId);

    GameObjectRef Find(Vector2Int cellPos);

    bool ApplyMove(const GameObjectRef& gameobject, Vector2Int dest);
    bool ApplyLeave(const GameObjectRef& gameObject);

    Vector<Vector2Int> FindPath(Vector2Int startCellPos, Vector2Int destCellPos, bool ignoreDestCollision = false);

private:
    int32  _MinX;
    int32  _MaxX;
    int32  _MinY;
    int32  _MaxY;
    int32  _sizeX;
    int32  _sizeY;

    Vector<Vector<bool>> _collision;
    Vector<Vector<GameObjectRef>> _objects;
    Pos Cell2Pos(const Vector2Int& cell) const;
    Vector2Int Pos2Cell(const Pos& pos) const;
    Vector<Vector2Int> CalcCellPathFromParent(const Vector<Vector<Pos>>& parent, const Pos& dest);
};

