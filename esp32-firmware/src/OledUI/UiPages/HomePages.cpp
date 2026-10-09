#include "../UiContext.h"
#include "UiPages.h"
#include "../MainPageState.h"
#include "../UiNavigation.h"
#include "HAL/HAL.h"
void OledUI::Tile_Param_Init()
{
    MainPageState::Initialize(ui, tile, {
        TILE_ICON_S,
        TILE_ICON_H,
        TILE_INDI_W,
        TILE_INDI_S,
        TILE_INDI_H,
        TILE_B_TITLE_H
    });
    ResetAnimationTrack(tile.icon_x_track);
    ResetAnimationTrack(tile.icon_y_track);
    ResetAnimationTrack(tile.indi_x_track);
    ResetAnimationTrack(tile.title_y_track);
}

void OledUI::Sleep_Param_Init()
{
    u8g2.setDrawColor(0);
    u8g2.drawBox(0, 0, DISP_W, DISP_H);
    u8g2.setPowerSave(1);
    ui.state = S_NONE;
    ui.sleep = true;
}

void OledUI::Sleep_Proc()
{
    if (!ui.sleep || !btn.pressed)
        return;

    // Wake on the next available input event. Keep this handler non-blocking so
    // Key_Scan() continues to run on every UI task iteration.
    btn.pressed = false;
    ui.sleep = false;
    u8g2.setPowerSave(0);

    OpenRoot(M_MAIN);
}

void OledUI::Main_Proc()
{
    Tile_Show(main_menu, main_icon_pic);

    if (btn.pressed)
    {
        btn.pressed = false;
        switch (btn.id)
        {
        case BTN_ID_CW:
        case BTN_ID_CC:
            UiPages::RotateTile();
            break;
        case BTN_ID_SP:
            switch (ui.select[ui.layer])
            {
            case 0: NavigateTo(M_LIGHT_MAIN); break;
            case 1: NavigateTo(M_EDITOR); break;
            case 2: NavigateTo(M_VOLT); break;
            case 3: NavigateTo(M_SETTING); break;
            }
            break;
        }
        if (!tile.select_flag && ui.init)
        {
            tile.indi_x = 0;
            tile.title_y = tile.title_y_calc;
        }
    }
}

void OledUI::Editor_Proc()
{
    List_Show(editor_menu, M_EDITOR);

    if (btn.pressed)
    {
        btn.pressed = false;
        switch (btn.id)
        {
        case BTN_ID_CW:
        case BTN_ID_CC:
            UiPages::RotateList();
            break;
        case BTN_ID_LP:
            ui.select[ui.layer] = 0;
        case BTN_ID_SP:
            switch (ui.select[ui.layer])
            {
            case 0:  Back(); break;
            case 11: NavigateTo(M_KNOB); break;
            }
            break;
        }
    }
}

