#include "OledUI.h"
void OledUI::Animation(
    AnimationTrack& track,
    float& value,
    float target,
    uint8_t speed_param)
{
    UpdateAnimation(
        track,
        value,
        target,
        AnimationDurationMs(ui.param[speed_param]),
        GetAnimationCurve(),
        millis());
}

void OledUI::SetAnimationCurve(AnimationCurve curve)
{
    const uint8_t position = AnimationCurveSelectionPosition(curve);
    const AnimationCurve validated = AnimationCurveFromSelectionPosition(position);
    ui.param[ANIM_CURVE] = static_cast<uint8_t>(validated);
    ui.param[ANIM_CURVE_POS] = position;
}

AnimationCurve OledUI::GetAnimationCurve()
{
    return AnimationCurveFromValue(ui.param[ANIM_CURVE]);
}

void OledUI::Fade()
{
    uint8_t* buf = OledDriver::GetBufferPtr();
    uint16_t len  = OledDriver::GetBufferLen();

    delay(ui.param[FADE_ANI]);

    if (ui.param[DARK_MODE]) // Dark mode
    {
        switch (ui.fade)
        {
        case 1:
            for (uint16_t i = 0; i < len; ++i)
                if (i % 2 != 0) buf[i] = buf[i] & 0xAA;
            break;
        case 2:
            for (uint16_t i = 0; i < len; ++i)
                if (i % 2 != 0) buf[i] = buf[i] & 0x00;
            break;
        case 3:
            for (uint16_t i = 0; i < len; ++i)
                if (i % 2 == 0) buf[i] = buf[i] & 0x55;
            break;
        case 4:
            for (uint16_t i = 0; i < len; ++i)
                if (i % 2 == 0) buf[i] = buf[i] & 0x00;
            break;
        default:
            ui.state = S_NONE;
            ui.fade = 0;
            break;
        }
    }
    else // Light mode
    {
        switch (ui.fade)
        {
        case 1:
            for (uint16_t i = 0; i < len; ++i)
                if (i % 2 != 0) buf[i] = buf[i] | 0xAA;
            break;
        case 2:
            for (uint16_t i = 0; i < len; ++i)
                if (i % 2 != 0) buf[i] = buf[i] | 0x00;
            break;
        case 3:
            for (uint16_t i = 0; i < len; ++i)
                if (i % 2 == 0) buf[i] = buf[i] | 0x55;
            break;
        case 4:
            for (uint16_t i = 0; i < len; ++i)
                if (i % 2 == 0) buf[i] = buf[i] | 0x00;
            break;
        default:
            ui.state = S_NONE;
            ui.fade = 0;
            break;
        }
    }
    ui.fade++;
}

void OledUI::Tile_Show(const M_SELECT arr[], const uint8_t icon_pic[][120])
{
    Animation(tile.icon_x_track,  tile.icon_x,  tile.icon_x_trg,  TILE_ANI);
    Animation(tile.icon_y_track,  tile.icon_y,  tile.icon_y_trg,  TILE_ANI);
    Animation(tile.indi_x_track,  tile.indi_x,  tile.indi_x_trg,  TILE_ANI);
    Animation(tile.title_y_track, tile.title_y, tile.title_y_trg, TILE_ANI);

    u8g2.setDrawColor(1);

    // Draw title
    u8g2.setFont(TILE_B_FONT);
    u8g2.drawStr(
        ((DISP_W - TILE_INDI_W) - u8g2.getStrWidth(arr[ui.select[ui.layer]].m_select)) / 2 + TILE_INDI_W,
        tile.title_y,
        arr[ui.select[ui.layer]].m_select);

    // Draw indicator bar
    u8g2.drawBox(0, TILE_ICON_S, tile.indi_x, TILE_INDI_H);

    // Draw icons
    if (!ui.init)
    {
        for (uint8_t i = 0; i < ui.num[ui.index]; ++i)
        {
            if (ui.param[TILE_UFD])
                tile.temp = (DISP_W - TILE_ICON_W) / 2 + i * tile.icon_x - TILE_ICON_S * ui.select[ui.layer];
            else
                tile.temp = (DISP_W - TILE_ICON_W) / 2 + (i - ui.select[ui.layer]) * tile.icon_x;

            u8g2.drawXBMP(tile.temp, (int16_t)tile.icon_y, TILE_ICON_W, TILE_ICON_H, icon_pic[i]);
        }
        if (tile.icon_x == tile.icon_x_trg)
        {
            ui.init = true;
            tile.icon_x = tile.icon_x_trg = -ui.select[ui.layer] * TILE_ICON_S;
        }
    }
    else
    {
        for (uint8_t i = 0; i < ui.num[ui.index]; ++i)
            u8g2.drawXBMP(
                (DISP_W - TILE_ICON_W) / 2 + (int16_t)tile.icon_x + i * TILE_ICON_S,
                0, TILE_ICON_W, TILE_ICON_H, icon_pic[i]);
    }

    // Invert for dark mode mask
    u8g2.setDrawColor(2);
    if (!ui.param[DARK_MODE])
        u8g2.drawBox(0, 0, DISP_W, DISP_H);
}

static void List_Draw_Value(uint8_t data_index)
{
    if (check_box.v == nullptr)
    {
        u8g2.print("--");
        return;
    }
    u8g2.print(check_box.v[data_index]);
}

static void List_Draw_CheckBox_Frame()
{
    u8g2.drawRFrame(CHECK_BOX_L_S, list.temp + CHECK_BOX_U_S, CHECK_BOX_F_W, CHECK_BOX_F_H, 1);
}

static void List_Draw_CheckBox_Dot()
{
    u8g2.drawBox(
        CHECK_BOX_L_S + CHECK_BOX_D_S + 1,
        list.temp + CHECK_BOX_U_S + CHECK_BOX_D_S + 1,
        CHECK_BOX_F_W - (CHECK_BOX_D_S + 1) * 2,
        CHECK_BOX_F_H - (CHECK_BOX_D_S + 1) * 2);
}

static void List_Draw_Krf(uint8_t data_index)
{
    switch (check_box.v[data_index])
    {
    case 0: u8g2.print("OFF"); break;
    case 1: u8g2.print("VOL"); break;
    case 2: u8g2.print("BRI"); break;
    }
}

static void List_Draw_Kpf(uint8_t data_index)
{
    if (check_box.v[data_index] == 0)
        u8g2.print("OFF");
    else if (check_box.v[data_index] <= 90)
        u8g2.print((char)check_box.v[data_index]);
    else
        u8g2.print("?");
}

static void List_Draw_Text_And_CheckBox(const M_SELECT arr[], int i)
{
    const M_SELECT &item = arr[i];

    u8g2.drawStr(LIST_TEXT_S, list.temp + LIST_TEXT_H + LIST_TEXT_S, item.m_select);
    u8g2.setCursor(CHECK_BOX_L_S, list.temp + LIST_TEXT_H + LIST_TEXT_S);

    switch (RenderModeFor(item))
    {
    case MenuItemRenderMode::Value:
        List_Draw_Value(item.data_index);
        break;
    case MenuItemRenderMode::Toggle:
        List_Draw_CheckBox_Frame();
        if (check_box.m != nullptr && check_box.m[item.data_index] == 1)
            List_Draw_CheckBox_Dot();
        break;
    case MenuItemRenderMode::Choice:
        List_Draw_CheckBox_Frame();
        if (check_box.s_p != nullptr && *check_box.s_p == item.data_index)
            List_Draw_CheckBox_Dot();
        break;
    case MenuItemRenderMode::RotateBinding:
        List_Draw_Krf(item.data_index);
        break;
    case MenuItemRenderMode::PressBinding:
        List_Draw_Kpf(item.data_index);
        break;
    case MenuItemRenderMode::None:
        break;
    }
}

void OledUI::List_Show(const M_SELECT arr[], uint8_t ui_index)
{
    u8g2.setFont(LIST_FONT);

    list.box_x_trg = u8g2.getStrWidth(arr[ui.select[ui.layer]].m_select) + LIST_TEXT_S * 2;
    list.bar_y_trg = ceil((ui.select[ui.layer]) * ((float)DISP_H / (ui.num[ui_index] - 1)));

    Animation(list.y_track,     list.y,     list.y_trg,                 LIST_ANI);
    Animation(list.box_x_track, list.box_x, list.box_x_trg,             LIST_ANI);
    Animation(list.box_y_track, list.box_y, list.box_y_trg[ui.layer],   LIST_ANI);
    Animation(list.bar_y_track, list.bar_y, list.bar_y_trg,             LIST_ANI);

    if (list.loop && list.box_y == list.box_y_trg[ui.layer])
        list.loop = false;

    u8g2.setDrawColor(1);

    // Progress bar
    u8g2.drawHLine(DISP_W - LIST_BAR_W, 0, LIST_BAR_W);
    u8g2.drawHLine(DISP_W - LIST_BAR_W, DISP_H - 1, LIST_BAR_W);
    u8g2.drawVLine(DISP_W - ceil((float)LIST_BAR_W / 2), 0, DISP_H);
    u8g2.drawBox(DISP_W - LIST_BAR_W, 0, LIST_BAR_W, list.bar_y);

    // List text
    if (!ui.init)
    {
        for (int i = 0; i < ui.num[ui_index]; ++i)
        {
            if (ui.param[LIST_UFD])
                list.temp = i * list.y - LIST_LINE_H * ui.select[ui.layer] + list.box_y_trg[ui.layer];
            else
                list.temp = (i - ui.select[ui.layer]) * list.y + list.box_y_trg[ui.layer];

            List_Draw_Text_And_CheckBox(arr, i);
        }
        if (list.y == list.y_trg)
        {
            ui.init = true;
            list.y = list.y_trg = -LIST_LINE_H * ui.select[ui.layer] + list.box_y_trg[ui.layer];
        }
    }
    else
    {
        for (int i = 0; i < ui.num[ui_index]; ++i)
        {
            list.temp = LIST_LINE_H * i + list.y;
            List_Draw_Text_And_CheckBox(arr, i);
        }
    }

    // Selection box
    u8g2.setDrawColor(2);
    u8g2.drawRBox(0, list.box_y, list.box_x, LIST_LINE_H, LIST_BOX_R);

    // Dark mode mask
    if (!ui.param[DARK_MODE])
    {
        u8g2.drawBox(0, 0, DISP_W, DISP_H);
        if (ui.index == M_WINDOW || ui.index == M_VOLT)
            u8g2.drawBox(0, 0, DISP_W, DISP_H);
    }
}

void OledUI::Volt_Show()
{
    List_Show(volt_menu, M_VOLT);

    Animation(volt.text_bg_r_track, volt.text_bg_r, volt.text_bg_r_trg, TAG_ANI);

    u8g2.setDrawColor(1);

    // Waveform frame
    u8g2.drawFrame(WAVE_BOX_L_S, 0, WAVE_BOX_W, WAVE_BOX_H);
    u8g2.drawFrame(WAVE_BOX_L_S + 1, 1, WAVE_BOX_W - 2, WAVE_BOX_H - 2);

    if (list.box_y == list.box_y_trg[ui.layer] && list.y == list.y_trg)
    {
        for (int i = 0; i < WAVE_SAMPLE * WAVE_W; i++)
            // volt.ch0_adc[i] = volt.val = analogRead(analog_pin[ui.select[ui.layer]]);
            volt.ch0_adc[i] = volt.val = random(500, 1023);

        for (int i = 1; i < WAVE_W - 1; i++)
        {
            volt.ch0_wave[i] = map(volt.ch0_adc[int(5 * i)], 0, 4095, WAVE_MAX, WAVE_MIN);
            u8g2.drawLine(WAVE_L + i - 1, WAVE_U + volt.ch0_wave[i - 1],
                          WAVE_L + i,       WAVE_U + volt.ch0_wave[i]);
        }
    }

    // Voltage value
    u8g2.setFontDirection(0);
    u8g2.setFont(VOLT_FONT);
    u8g2.setCursor(39, DISP_H - 6);
    u8g2.print(volt.val / 4096.0f * 3.3f);
    u8g2.print("V");

    // Text background + dark mode
    u8g2.setDrawColor(2);
    u8g2.drawBox(VOLT_TEXT_BG_L_S, DISP_H - VOLT_TEXT_BG_H, volt.text_bg_r, VOLT_TEXT_BG_H);

    if (!ui.param[DARK_MODE])
        u8g2.drawBox(0, 0, DISP_W, DISP_H);
}

void OledUI::Window_Show()
{
    List_Show(win.bg, win.bg_index);

    if (ui.param[WIN_BOK])
    {
        uint8_t* buf = OledDriver::GetBufferPtr();
        uint16_t len  = OledDriver::GetBufferLen();
        for (uint16_t i = 0; i < len; ++i)
            buf[i] = buf[i] & (i % 2 == 0 ? 0x55 : 0xAA);
    }

    u8g2.setFont(WIN_FONT);
    win.bar_trg = (float)(*win.value - win.min) / (float)(win.max - win.min) * (WIN_BAR_W - 4);

    Animation(win.bar_track, win.bar, win.bar_trg, WIN_ANI);
    Animation(win.y_track,   win.y,   win.y_trg,   WIN_ANI);

    win.l = (DISP_W - WIN_W) / 2;
    win.u = (DISP_H - WIN_H) / 2;

    u8g2.setDrawColor(0);
    u8g2.drawRBox(win.l, (int16_t)win.y, WIN_W, WIN_H, 2);
    u8g2.setDrawColor(1);
    u8g2.drawRFrame(win.l, (int16_t)win.y, WIN_W, WIN_H, 2);
    u8g2.drawRFrame(win.l + 5, (int16_t)win.y + 20, WIN_BAR_W, WIN_BAR_H, 1);
    u8g2.drawBox(win.l + 7, (int16_t)win.y + 22, win.bar, WIN_BAR_H - 4);
    u8g2.setCursor(win.l + 5, (int16_t)win.y + 14);
    u8g2.print(win.title);
    u8g2.setCursor(win.l + 78, (int16_t)win.y + 14);
    u8g2.print(*win.value);

    // Apply brightness immediately on "Disp Bri"
    if (!strcmp(win.title, "Disp Bri"))
        u8g2.setContrast(ui.param[DISP_BRI]);

    u8g2.setDrawColor(2);
    if (!ui.param[DARK_MODE])
        u8g2.drawBox(0, 0, DISP_W, DISP_H);
}

