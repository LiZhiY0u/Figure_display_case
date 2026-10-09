#include <cassert>
#include <vector>
#include "OledUI/UiContext.h"
#include "OledUI/UiPages/UiPages.h"
uint32_t test_now=0;SerialFake Serial;
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
UI_Context ui{};Tile_Context tile{};List_Context list{};Volt_Context volt{};
CheckBox_Context check_box{};Win_Context win{};Knob_Context knob{};volatile Button_Context btn{};
namespace { std::vector<int> calls; }
namespace OledDriver {
void SendBuffer(){calls.push_back(101);} void ClearBuffer(){calls.push_back(102);}
void Init(){} void SetContrast(uint8_t){}
}
namespace ConnectionToast { void Draw(){calls.push_back(103);} }
namespace OledUI {
void Key_Scan(){calls.push_back(100);}
void Fade(){calls.push_back(200);} void Window_Param_Init(){calls.push_back(201);}
void Layer_Init_In(){calls.push_back(202);} void Layer_Init_Out(){calls.push_back(203);}
void OpenRoot(UI_PAGE){}
void SetAnimationCurve(AnimationCurve){}
#define PAGE(fn,id) void fn(){calls.push_back(id);}
PAGE(Window_Proc,0) PAGE(Sleep_Proc,1) PAGE(Main_Proc,2) PAGE(Editor_Proc,3)
PAGE(Knob_Proc,4) PAGE(Krf_Proc,5) PAGE(Kpf_Proc,6) PAGE(Volt_Proc,7)
PAGE(Setting_Proc,8) PAGE(AnimationCurve_Proc,9) PAGE(About_Proc,10)
PAGE(Light_Main_Proc,11) PAGE(Light_Bri_Proc,12) PAGE(Light_Color_Proc,13) PAGE(Light_Effect_Proc,14)
}
int main() {
    for(int page=0;page<15;++page) {
        calls.clear();UiPages::Process(static_cast<UI_PAGE>(page));assert(calls==std::vector<int>{page});
        calls.clear();ui.state=S_NONE;ui.index=static_cast<uint8_t>(page);OledUI::Task();
        assert(calls==std::vector<int>({100,101,102,page,103}));
    }
    calls.clear();UiPages::Process(static_cast<UI_PAGE>(99));assert(calls.empty());
    for(int state=0;state<4;++state) {
        calls.clear();ui.state=static_cast<uint8_t>(state);OledUI::Task();
        assert(calls==std::vector<int>({100,101,200+state}));
    }
}
