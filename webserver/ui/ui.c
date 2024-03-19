// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.4.0
// LVGL version: 8.3.11
// Project name: SquareLine_Project

#include "ui.h"
#include "ui_helpers.h"

///////////////////// VARIABLES ////////////////////


// SCREEN: ui_HomeScreen
void ui_HomeScreen_screen_init(void);
lv_obj_t *ui_HomeScreen;
lv_obj_t *ui_TabView1;
lv_obj_t *ui_TabPage1;
lv_obj_t *ui_Container1;
void ui_event_flushButton( lv_event_t * e);
lv_obj_t *ui_flushButton;
lv_obj_t *ui_Label1;
void ui_event_Button1( lv_event_t * e);
lv_obj_t *ui_Button1;
lv_obj_t *ui_Label2;
lv_obj_t *ui_Container2;
lv_obj_t *ui_Label4;
lv_obj_t *ui_profileNameLabel;
lv_obj_t *ui_Container3;
lv_obj_t *ui_currentProfileGraph;
lv_obj_t *ui_tempGauge;
lv_obj_t *ui_tempLabel;
lv_obj_t *ui_waterGauge;
lv_obj_t *ui_waterLabel;
lv_obj_t *ui_TabPage2;
lv_obj_t *ui_TabPage3;


// SCREEN: ui_BrewingScreen
void ui_BrewingScreen_screen_init(void);
void ui_event_BrewingScreen( lv_event_t * e);
lv_obj_t *ui_BrewingScreen;
lv_obj_t *ui_BrewGraphBack;
lv_obj_t *ui_BrewGraph;
void ui_event_closeBrewScreenButton( lv_event_t * e);
lv_obj_t *ui_closeBrewScreenButton;
lv_obj_t *ui_Label3;
lv_obj_t *ui_timeLabel;
lv_obj_t *ui_Panel1;
lv_obj_t *ui_pLabel;
lv_obj_t *ui_pValueLabel;
lv_obj_t *ui_fLabel;
lv_obj_t *ui_fValueLabel;
lv_obj_t *ui_wLabel;
lv_obj_t *ui_wValueLabel;
lv_obj_t *ui_wPsLabel;
lv_obj_t *ui_wPsValueLabel;
lv_obj_t *ui_tLabel;
lv_obj_t *ui_tValueLabel;


// SCREEN: ui_EditProfileScreen
void ui_EditProfileScreen_screen_init(void);
lv_obj_t *ui_EditProfileScreen;
lv_obj_t *ui____initial_actions0;

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
    #error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP !=1
    #error "LV_COLOR_16_SWAP should be 1 to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////
void ui_event_flushButton( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
if ( event_code == LV_EVENT_CLICKED) {
      sendFlushAction( e );
}
}
void ui_event_Button1( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
if ( event_code == LV_EVENT_CLICKED) {
      _ui_screen_change( &ui_BrewingScreen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, &ui_BrewingScreen_screen_init);
}
}
void ui_event_BrewingScreen( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
if ( event_code == LV_EVENT_SCREEN_LOADED) {
      brewingScreenAppear( e );
}
}
void ui_event_closeBrewScreenButton( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
if ( event_code == LV_EVENT_CLICKED) {
      _ui_screen_change( &ui_HomeScreen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, &ui_HomeScreen_screen_init);
}
}

///////////////////// SCREENS ////////////////////

void ui_init( void )
{
lv_disp_t *dispp = lv_disp_get_default();
lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
lv_disp_set_theme(dispp, theme);
ui_HomeScreen_screen_init();
ui_BrewingScreen_screen_init();
ui_EditProfileScreen_screen_init();
ui____initial_actions0 = lv_obj_create(NULL);
lv_disp_load_scr( ui_HomeScreen);
}
