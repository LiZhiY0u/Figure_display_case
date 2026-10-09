#include <cassert>
#include "OledUI/UiContext.h"
#include "Input/InputQueue.h"
#include "Bluetooth/ConnectionEvents.h"
uint32_t test_now=0;SerialFake Serial;
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
namespace { uint8_t buffer[1024]={}; }
namespace OledDriver {
void Init(){} void SendBuffer(){} void ClearBuffer(){}
void SetContrast(uint8_t){}
uint8_t* GetBufferPtr(){return buffer;} uint16_t GetBufferLen(){return 1024;}
}
namespace InputQueue {
bool Push(const UserInput::Event&){return true;}
bool Read(UserInput::Event&){return false;}
}
namespace ConnectionEvents { bool Read(Event&){return false;} }
namespace ENCODER { int16_t Encoder_GetDiff(){return 0;} }
int main() {
    OledUI::Init();
    assert(ui.index==M_MAIN && ui.layer==0 && ui.state==S_NONE && tile.icon_x_trg==36);
    ui.select[0]=3;
    assert(OledUI::NavigateTo(M_SETTING));OledUI::Layer_Init_In();
    assert(ui.layer==1 && ui.index==M_SETTING && check_box.v==ui.param && check_box.m==ui.param);
    ui.param[DISP_BRI]=91;
    assert(OledUI::NavigateTo(M_ANIM_CURVE));OledUI::Layer_Init_In();
    assert(ui.layer==2 && check_box.s==&ui.param[ANIM_CURVE] && check_box.s_p==&ui.param[ANIM_CURVE_POS]);
    assert(OledUI::Back());OledUI::Layer_Init_Out();
    assert(ui.layer==1 && ui.index==M_SETTING && ui.param[DISP_BRI]==91);
    assert(OledUI::Back());OledUI::Layer_Init_Out();
    assert(ui.layer==0 && ui.index==M_MAIN && ui.select[0]==3);
    OledUI::OpenRoot(M_SLEEP);assert(ui.sleep && ui.index==M_SLEEP);
    btn.pressed=true;OledUI::Sleep_Proc();
    assert(!ui.sleep && !btn.pressed && ui.index==M_MAIN && ui.select[0]==0);
    OledUI::OpenRoot(M_LIGHT_COLOR);auto* selection=check_box.s;
    assert(selection && selection==check_box.s_p);
    *selection=3;
    OledUI::OpenRoot(M_MAIN);OledUI::OpenRoot(M_LIGHT_COLOR);
    assert(check_box.s==selection && *selection==3);
}
