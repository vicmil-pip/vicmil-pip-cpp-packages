#include "util_entities.hpp"

int main()
{
    // Setup
    vicmil::EntityManager em;

    // Create entity
    vicmil::EntityHandle e = em.create_entity();

    int chunksize = 16;

    vicmil::GridChunkHashmap2D chunk_map = vicmil::GridChunkHashmap2D(16);

    int e_x = 10;
    int e_y = 10;
    chunk_map.set_obj(e_x, e_y, e);

    std::cout << chunk_map.get_obj(e_x, e_y).is_valid() << std::endl;     // Should return true
    std::cout << chunk_map.get_obj(e_x + 1, e_y).is_valid() << std::endl; // Should return false

    return 0;
}