#include "util_bin_packing.hpp"

int main()
{
    smol_atlas_t *atlas = sma_atlas_create(100, 100);
    // add a 70x30 item
    smol_atlas_item_t *item = sma_item_add(atlas, 70, 30);
    if (item)
    {
        // where did it end up?
        int x = sma_item_x(item);
        int y = sma_item_y(item);
        PrintExpr(x);
        PrintExpr(y);

        // can also remove it at some point
        sma_item_remove(atlas, item);
    }
    sma_atlas_destroy(atlas);
    return 0;
}