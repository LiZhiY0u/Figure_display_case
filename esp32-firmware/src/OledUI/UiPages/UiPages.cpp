#include "UiPages.h"
#include "../UiContext.h"
void UiPages::Process(UI_PAGE page) {
    switch(page) {
    case M_WINDOW: OledUI::Window_Proc();break;
    case M_SLEEP: OledUI::Sleep_Proc();break;
    case M_MAIN: OledUI::Main_Proc();break;
    case M_EDITOR: OledUI::Editor_Proc();break;
    case M_KNOB: OledUI::Knob_Proc();break;
    case M_KRF: OledUI::Krf_Proc();break;
    case M_KPF: OledUI::Kpf_Proc();break;
    case M_VOLT: OledUI::Volt_Proc();break;
    case M_SETTING: OledUI::Setting_Proc();break;
    case M_ANIM_CURVE: OledUI::AnimationCurve_Proc();break;
    case M_ABOUT: OledUI::About_Proc();break;
    case M_LIGHT_MAIN: OledUI::Light_Main_Proc();break;
    case M_LIGHT_BRI: OledUI::Light_Bri_Proc();break;
    case M_LIGHT_COLOR: OledUI::Light_Color_Proc();break;
    case M_LIGHT_EFFECT: OledUI::Light_Effect_Proc();break;
    default:break;
    }
}
