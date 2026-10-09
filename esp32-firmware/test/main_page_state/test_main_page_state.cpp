#include <cassert>

#include "OledUI/MainPageState.h"

namespace
{
struct UiState
{
    bool init = true;
};

struct TileState
{
    float icon_x = -1;
    float icon_x_trg = -1;
    float icon_y = -1;
    float icon_y_trg = -1;
    float indi_x = -1;
    float indi_x_trg = -1;
    float title_y = -1;
    float title_y_trg = -1;
};
}

int main()
{
    UiState ui;
    TileState tile;

    MainPageState::Initialize(ui, tile, {
        36, // icon spacing
        30, // icon height
        7,  // indicator width
        36, // indicator start
        27, // indicator height
        18  // title height
    });

    assert(!ui.init);
    assert(tile.icon_x == 0.0f);
    assert(tile.icon_x_trg == 36.0f);
    assert(tile.icon_y == -30.0f);
    assert(tile.icon_y_trg == 0.0f);
    assert(tile.indi_x == 0.0f);
    assert(tile.indi_x_trg == 7.0f);
    assert(tile.title_y == 76.0f);
    assert(tile.title_y_trg == 58.0f);

    return 0;
}
