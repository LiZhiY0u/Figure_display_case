#include <cassert>
#include "OledUI/UiContext.h"
uint32_t test_now=0;
SerialFake Serial;
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
UI_Context ui{};Tile_Context tile{};List_Context list{};Volt_Context volt{};
CheckBox_Context check_box{};Win_Context win{};Knob_Context knob{};volatile Button_Context btn{};
const uint8_t analog_pin[10]={};
namespace { uint8_t buffer[4]={}; }
namespace OledDriver { uint8_t* GetBufferPtr(){return buffer;} uint16_t GetBufferLen(){return 4;} }
bool has(const char* name,std::initializer_list<int> args) {
    for(const auto& c:u8g2.calls) if(c.name==name && c.args==std::vector<int>(args))return true;
    return false;
}
bool text(const char* value) {
    for(const auto& c:u8g2.calls) { if(c.text==value)return true; }
    return false;
}
int main() {
    const M_SELECT items[]={{"Title",MenuItemType::Title},{"Value",MenuItemType::Value,0},{"Toggle",MenuItemType::Toggle,0}};
    const uint8_t icons[3][120]={};
    ui.index=M_MAIN;ui.num[M_MAIN]=3;ui.param[DARK_MODE]=1;
    tile.icon_x=tile.icon_x_trg=36;tile.icon_y=tile.icon_y_trg=0;
    OledUI::Tile_Show(items,icons);
    assert(ui.init && text("Title") && has("bitmap",{49,0,30,30}) && has("bitmap",{85,0,30,30}));
    u8g2.calls.clear();OledUI::Tile_Show(items,icons);assert(has("bitmap",{121,0,30,30}));
    ui.index=M_SETTING;ui.num[M_SETTING]=3;ui.init=true;
    uint8_t value=7,toggle=1;check_box.v=&value;check_box.m=&toggle;
    list.y=list.y_trg=0;
    u8g2.calls.clear();OledUI::List_Show(items,M_SETTING);
    assert(text("Value") && text("7") && has("rframe",{95,34,12,12,1}) && has("box",{98,37,6,6}));
    value=0;win.value=&value;win.min=0;win.max=100;win.bg=items;win.bg_index=M_SETTING;
    win.y=win.y_trg=16;win.bar=0;
    u8g2.calls.clear();OledUI::Window_Show();assert(win.bar_trg==0 && has("rbox",{13,16,102,32,2}));
    value=100;OledUI::Window_Show();assert(win.bar_trg==88);
    for(auto& b:buffer)b=255;
    ui.param[DARK_MODE]=1;ui.fade=1;ui.state=S_FADE;
    OledUI::Fade();assert(buffer[0]==255 && buffer[1]==170 && ui.fade==2);
    OledUI::Fade();assert(buffer[1]==0);
    OledUI::Fade();assert(buffer[0]==85);
    OledUI::Fade();assert(buffer[0]==0);
    OledUI::Fade();assert(ui.state==S_NONE && ui.fade==1);
}
