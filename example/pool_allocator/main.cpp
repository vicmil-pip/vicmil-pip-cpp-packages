#include "util_entities.hpp"

class Position
{
public:
    int x;
    int y;
};

int main()
{
    vicmil::PoolAllocator<Position> pos_pool;
    Position *p = pos_pool.allocate();
    p->x = 10;

    pos_pool.deallocate(p);
    Print("Done!");
    return 0;
}