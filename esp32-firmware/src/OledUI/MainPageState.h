#ifndef OLED_UI_MAIN_PAGE_STATE_H
#define OLED_UI_MAIN_PAGE_STATE_H

namespace MainPageState
{
struct Layout
{
    int icon_spacing;
    int icon_height;
    int indicator_width;
    int indicator_start;
    int indicator_height;
    int title_height;
};

template <typename UiContext, typename TileContext>
void Initialize(UiContext &ui, TileContext &tile, const Layout &layout)
{
    ui.init = false;
    tile.icon_x = 0;
    tile.icon_x_trg = layout.icon_spacing;
    tile.icon_y = -layout.icon_height;
    tile.icon_y_trg = 0;
    tile.indi_x = 0;
    tile.indi_x_trg = layout.indicator_width;
    tile.title_y = layout.indicator_start
                 + (layout.indicator_height - layout.title_height) / 2
                 + layout.title_height * 2;
    tile.title_y_trg = layout.indicator_start
                     + (layout.indicator_height - layout.title_height) / 2
                     + layout.title_height;
}
}

#endif
