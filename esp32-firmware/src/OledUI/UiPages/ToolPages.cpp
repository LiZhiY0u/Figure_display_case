#include "../UiContext.h"
#include "UiPages.h"
#include "../MainPageState.h"
#include "../UiNavigation.h"
#include "HAL/HAL.h"
void OledUI::Knob_Param_Init()
{
    CheckBox_Value_Init(knob.param);
}

void OledUI::Krf_Param_Init()
{
    CheckBox_Single_Init(&knob.param[KNOB_ROT], &knob.param[KNOB_ROT_P]);
}

void OledUI::Kpf_Param_Init()
{
    CheckBox_Single_Init(&knob.param[KNOB_COD], &knob.param[KNOB_COD_P]);
}

void OledUI::Volt_Param_Init()
{
    volt.text_bg_r = 0;
    volt.text_bg_r_trg = VOLT_TEXT_BG_W;
    ResetAnimationTrack(volt.text_bg_r_track);
}

void OledUI::Knob_Proc()
{
    List_Show(knob_menu, M_KNOB);

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
            case 0: Back(); break;
            case 1:
                NavigateTo(M_KRF);
                break;
            case 2:
                NavigateTo(M_KPF);
                break;
            }
            break;
        }
    }
}

void OledUI::Krf_Proc()
{
    List_Show(krf_menu, M_KRF);

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
            case 0: Back(); break;
            case 2: CheckBox_S_Select(KNOB_DISABLE, ui.select[ui.layer]); break;
            case 4: CheckBox_S_Select(KNOB_ROT_VOL, ui.select[ui.layer]); break;
            case 5: CheckBox_S_Select(KNOB_ROT_BRI, ui.select[ui.layer]); break;
            }
            break;
        }
    }
}

void OledUI::Kpf_Proc()
{
    List_Show(kpf_menu, M_KPF);

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
            // Key mapping disabled in this build (stub only)
            if (ui.select[ui.layer] == 0)
            {
                Back();
            }
            break;
        }
    }
}

void OledUI::Volt_Proc()
{
    Volt_Show();

    if (btn.pressed)
    {
        btn.pressed = false;
        switch (btn.id)
        {
        case BTN_ID_CW:
        case BTN_ID_CC:
            UiPages::RotateList();
            break;
        case BTN_ID_SP:
        case BTN_ID_LP:
            Back();
            break;
        }
    }
}
