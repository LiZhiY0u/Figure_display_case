/**
 * @file   OledUI.h
 * @author LZY (original author: RQNG, WouoUI)
 * @brief  OLED UI layer - state machines, animations, page processing
 *
 * Contains all UI logic: page enums, state machine, animation structs,
 * per-page init/show/proc functions, and fade/tile/list/spotlight effects.
 */

#ifndef __OLED_UI_H
#define __OLED_UI_H

#include <Arduino.h>
#include "AnimationCurve.h"
#include "UiTypes.h"
#include "OledDriver.h"
#include "OledMenus.h"

// ===========================
// UI param indices
// ===========================
enum UI_PARAM
{
    DISP_BRI,   // Display brightness (contrast 0-255)
    TILE_ANI,   // Tile animation speed
    LIST_ANI,   // List animation speed
    WIN_ANI,    // Window popup animation speed
    SPOT_ANI,   // Spotlight animation speed
    TAG_ANI,    // Tag / voltage tag animation speed
    FADE_ANI,   // Fade-out transition speed
    BTN_SPT,    // Button short-press threshold (ms)
    BTN_LPT,    // Button long-press threshold (ms)
    TILE_UFD,   // Tile unfilled start from top toggle
    LIST_UFD,   // List unfilled start from top toggle
    TILE_LOOP,  // Tile icon loop mode
    LIST_LOOP,  // List loop mode
    WIN_BOK,    // Window background bokeh (blur) toggle
    KNOB_DIR,   // Knob rotation direction invert toggle
    DARK_MODE,  // Dark mode toggle
    ANIM_CURVE, // Runtime AnimationCurve value
    ANIM_CURVE_POS, // Selected row in the animation curve menu
    UI_PARAM_COUNT,
};

// ===========================
// Knob / button constants
// ===========================
#define KNOB_PARAM 4

// Knob param indices
enum { KNOB_ROT = 0, KNOB_COD = 1, KNOB_ROT_P = 2, KNOB_COD_P = 3 };
enum { KNOB_DISABLE = 0, KNOB_ROT_VOL = 1, KNOB_ROT_BRI = 2 };

// Button IDs
enum { BTN_ID_CC = 0, BTN_ID_CW = 1, BTN_ID_SP = 2, BTN_ID_LP = 3 };

// ===========================
// UI dimensions
// ===========================
#define UI_MNUMB 100  // Menu item count max
#define BTN_PARAM_TIMES 2

// Tile dimensions (for M_MAIN)
#define TILE_B_FONT u8g2_font_helvB18_tr
#define TILE_B_TITLE_H 18
#define TILE_ICON_H 30
#define TILE_ICON_W 30
#define TILE_ICON_S 36
#define TILE_INDI_H 27
#define TILE_INDI_W 7
#define TILE_INDI_S 36

// List dimensions
#define LIST_FONT u8g2_font_HelvetiPixel_tr
#define LIST_TEXT_H 8
#define LIST_LINE_H 16
#define LIST_TEXT_S 4
#define LIST_BAR_W 5
#define LIST_BOX_R 0.5f

// Checkbox dimensions
#define CHECK_BOX_L_S 95
#define CHECK_BOX_U_S 2
#define CHECK_BOX_F_W 12
#define CHECK_BOX_F_H 12
#define CHECK_BOX_D_S 2

// Window popup dimensions
#define WIN_FONT u8g2_font_HelvetiPixel_tr
#define WIN_H 32
#define WIN_W 102
#define WIN_BAR_W 92
#define WIN_BAR_H 7
#define WIN_Y (-WIN_H - 2)
#define WIN_Y_TRG (-WIN_H - 2)

// Voltage waveform dimensions
#define WAVE_SAMPLE 20
#define WAVE_W 94
#define WAVE_L 24
#define WAVE_U 0
#define WAVE_MAX 27
#define WAVE_MIN 4
#define WAVE_BOX_H 32
#define WAVE_BOX_W 94
#define WAVE_BOX_L_S 24
#define VOLT_FONT u8g2_font_helvB18_tr
#define VOLT_TEXT_BG_L_S 24
#define VOLT_TEXT_BG_W 94
#define VOLT_TEXT_BG_H 29

// Analog channel pins for voltage monitor
extern const uint8_t analog_pin[10];

// ===========================
// Core UI struct
// ===========================
typedef struct
{
    bool init;                   // Page-initialized flag (cleared on layer change)
    uint8_t num[UI_MNUMB];       // Menu item counts per page
    uint8_t select[UI_DEPTH];     // Selected item per layer
    uint8_t layer;               // Current layer depth
    uint8_t index;               // Current page index
    uint8_t state;               // Current UI state
    bool sleep;                  // Sleep/idle mode flag
    uint8_t fade;                // Fade animation step counter
    uint8_t param[UI_PARAM_COUNT]; // UI parameter values
} UI_Context;

// ===========================
// Tile animation struct
// ===========================
typedef struct
{
    float title_y_calc     = TILE_INDI_S + (TILE_INDI_H - TILE_B_TITLE_H) / 2 + TILE_B_TITLE_H * 2;
    float title_y_trg_calc  = TILE_INDI_S + (TILE_INDI_H - TILE_B_TITLE_H) / 2 + TILE_B_TITLE_H;
    int16_t temp;
    bool select_flag;
    float icon_x;
    float icon_x_trg;
    AnimationTrack icon_x_track;
    float icon_y;
    float icon_y_trg;
    AnimationTrack icon_y_track;
    float indi_x;
    float indi_x_trg;
    AnimationTrack indi_x_track;
    float title_y;
    float title_y_trg;
    AnimationTrack title_y_track;
} Tile_Context;

// ===========================
// List animation struct
// ===========================
typedef struct
{
    uint8_t line_n;
    int16_t temp;
    bool loop;
    float y;
    float y_trg;
    AnimationTrack y_track;
    float box_x;
    float box_x_trg;
    AnimationTrack box_x_track;
    float box_y;
    float box_y_trg[UI_DEPTH];
    AnimationTrack box_y_track;
    float bar_y;
    float bar_y_trg;
    AnimationTrack bar_y_track;
} List_Context;

// ===========================
// Voltage monitor struct
// ===========================
typedef struct
{
    int ch0_adc[WAVE_SAMPLE * WAVE_W];
    int ch0_wave[WAVE_W];
    int val;
    float text_bg_r;
    float text_bg_r_trg;
    AnimationTrack text_bg_r_track;
} Volt_Context;

// ===========================
// Checkbox context
// ===========================
typedef struct
{
    uint8_t* v;    // value pointer
    uint8_t* m;    // multi-select flag pointer
    uint8_t* s;    // single-select value pointer
    uint8_t* s_p;  // single-select position pointer
} CheckBox_Context;

// ===========================
// Window popup struct
// ===========================
typedef struct
{
    uint8_t* value;
    uint8_t max;
    uint8_t min;
    uint8_t step;
    const M_SELECT* bg;
    uint8_t bg_index;
    char title[20];
    uint8_t select;
    uint8_t l;
    uint8_t u;
    float bar;
    float bar_trg;
    AnimationTrack bar_track;
    float y;
    float y_trg;
    AnimationTrack y_track;
} Win_Context;

// ===========================
// Knob config struct
// ===========================
typedef struct
{
    uint8_t param[KNOB_PARAM];
} Knob_Context;

// ===========================
// Button struct
// ===========================
typedef struct
{
    uint8_t id;
    bool flag;
    bool pressed;
    bool CW_1;
    bool CW_2;
    bool val;
    bool val_last;
    bool alv;
    bool blv;
    long count;
} Button_Context;

// ===========================
// External globals (from main.h)
// ===========================
extern bool key_en;        // encoder button pressed flag
extern uint8_t gamepad_axis; // gamepad axis input (1/2/3)

// ===========================
// Global UI contexts
// ===========================
extern UI_Context ui;
extern Tile_Context tile;
extern List_Context list;
extern Volt_Context volt;
extern CheckBox_Context check_box;
extern Win_Context win;
extern Knob_Context knob;
extern volatile Button_Context btn;

// ===========================
// OledUI API
// ===========================
namespace OledUI
{
    /**
     * @brief Initialize the entire UI subsystem
     *        Calls: ui_param_init() -> ui_init() -> OledDriver::Init()
     */
    void Init();

    /**
     * @brief Main UI task entry - call this repeatedly in a loop or task
     *        Handles: key_scan -> page render/proc -> sendBuffer
     */
    void Task();

    /**
     * @brief Scan button/encoder input
     */
    void Key_Scan();

    // ---- Low-level init ----
    void UI_Param_Init();
    void UI_Init();

    // ---- Page init functions ----
    void Tile_Param_Init();
    void Sleep_Param_Init();
    void Knob_Param_Init();
    void Krf_Param_Init();
    void Kpf_Param_Init();
    void Volt_Param_Init();
    void Setting_Param_Init();
    void AnimationCurve_Param_Init();

    // ---- Checkbox helpers ----
    void CheckBox_Value_Init(uint8_t* param);
    void CheckBox_Multi_Init(uint8_t* param);
    void CheckBox_Single_Init(uint8_t* param, uint8_t* param_pos);
    void CheckBox_M_Select(uint8_t idx);
    void CheckBox_S_Select(uint8_t val, uint8_t pos);

    // ---- Window helpers ----
    void Window_Value_Init(
        const char* title, uint8_t sel_param, uint8_t* value,
        uint8_t max, uint8_t min, uint8_t step,
        const M_SELECT* bg, uint8_t bg_index);

    // ---- Animation functions ----
    void Animation(AnimationTrack& track, float& value, float target, uint8_t speed_param);
    void SetAnimationCurve(AnimationCurve curve);
    AnimationCurve GetAnimationCurve();
    void Fade();

    // ---- Layer transition init ----
    bool NavigateTo(UI_PAGE page);
    bool Back();
    void OpenRoot(UI_PAGE page);
    void Layer_Init_In();
    void Layer_Init_Out();
    void Window_Param_Init();

    // ---- Per-page show functions ----
    void Tile_Show(const M_SELECT arr[], const uint8_t icon_pic[][120]);
    void List_Show(const M_SELECT arr[], uint8_t page_idx);
    void Volt_Show();
    void Window_Show();

    // ---- Per-page process functions ----
    void Sleep_Proc();
    void Main_Proc();
    void Editor_Proc();
    void Knob_Proc();
    void Krf_Proc();
    void Kpf_Proc();
    void Volt_Proc();
    void Setting_Proc();
    void AnimationCurve_Proc();
    void About_Proc();
    void Window_Proc();

    // ---- Light 灯带 ----
    void Light_Main_Show();
    void Light_Main_Proc();
    void Light_Bri_Show();
    void Light_Bri_Proc();
    void Light_Color_Show();
    void Light_Color_Proc();
    void Light_Effect_Show();
    void Light_Effect_Proc();
}

#endif // __OLED_UI_H
