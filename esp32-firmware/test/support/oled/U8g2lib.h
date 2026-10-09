#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
static const uint8_t u8g2_font_5x7_tr[]={1};
static const uint8_t u8g2_font_helvB18_tr[]={2};
static const uint8_t u8g2_font_HelvetiPixel_tr[]={3};
struct DrawCall { std::string name; std::vector<int> args; std::string text; };
struct U8G2_SH1106_128X64_NONAME_F_HW_I2C {
    std::vector<DrawCall> calls;
    struct Data { const uint8_t* font=nullptr; } data;
    uint8_t color=1;
    void record(const char* name,std::initializer_list<int> args={},const std::string& text="") {
        calls.push_back({name,args,text});
    }
    Data* getU8g2() { return &data; }
    uint8_t getDrawColor() { return color; }
    void setDrawColor(uint8_t c) { color=c; record("color",{c}); }
    void setFont(const uint8_t* f) { data.font=f; record("font"); }
    void setFontDirection(int d) { record("direction",{d}); }
    int getStrWidth(const char* s) { return static_cast<int>(std::strlen(s))*5; }
    void drawStr(int x,int y,const char* s) { record("str",{x,y},s); }
    void drawBox(int x,int y,int w,int h) { record("box",{x,y,w,h}); }
    void drawRBox(int x,int y,int w,int h,int r) { record("rbox",{x,y,w,h,r}); }
    void drawRFrame(int x,int y,int w,int h,int r) { record("rframe",{x,y,w,h,r}); }
    void drawFrame(int x,int y,int w,int h) { record("frame",{x,y,w,h}); }
    void drawXBMP(int x,int y,int w,int h,const uint8_t*) { record("bitmap",{x,y,w,h}); }
    void drawHLine(int x,int y,int w) { record("hline",{x,y,w}); }
    void drawVLine(int x,int y,int h) { record("vline",{x,y,h}); }
    void drawLine(int x,int y,int xx,int yy) { record("line",{x,y,xx,yy}); }
    void setCursor(int x,int y) { record("cursor",{x,y}); }
    void print(const char* s) { record("print",{},s); }
    void print(char* s) { print(static_cast<const char*>(s)); }
    void print(char c) { record("print",{},std::string(1,c)); }
    template<typename T> void print(T value) { record("print",{},std::to_string(value)); }
    void setContrast(uint8_t c) { record("contrast",{c}); }
    void setPowerSave(int p) { record("power",{p}); }
};
