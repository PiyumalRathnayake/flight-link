#include "ui.h"

extern void startCalibration(void);
extern void startBinding(void);
extern void setEncryptionState(bool enable);
extern void setSensitivity(int val);
extern void setRFPower(int level);

static void updatePowerButtonStyles(int level)
{
    if (!ui_lowbtn || !ui_medbtn || !ui_higbtn) return;

    lv_obj_set_style_bg_opa(ui_lowbtn, 40,  LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_lowbtn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_medbtn, 40,  LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_medbtn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_higbtn, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_higbtn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    if (level == 0) {
        lv_obj_set_style_bg_color(ui_lowbtn, lv_color_hex(0xFDDA32), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_lowbtn, 220, LV_PART_MAIN | LV_STATE_DEFAULT);
    } else if (level == 1) {
        lv_obj_set_style_bg_color(ui_medbtn, lv_color_hex(0xFDDA32), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_medbtn, 220, LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_bg_color(ui_higbtn, lv_color_hex(0xFDDA32), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_higbtn, 220, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

void profileChange(lv_event_t * e)
{
    lv_obj_t * dd = lv_event_get_target(e);
    uint16_t sel = lv_dropdown_get_selected(dd);

    char buf[32];
    lv_dropdown_get_selected_str(dd, buf, sizeof(buf));

    if (ui_profileL) lv_label_set_text(ui_profileL, buf);

    extern void setProfile(uint8_t p);
    setProfile((uint8_t)sel);
}

void encEvtCh(lv_event_t * e)
{
    lv_obj_t * sw = lv_event_get_target(e);
    bool is_encrypted = lv_obj_has_state(sw, LV_STATE_CHECKED);
    setEncryptionState(is_encrypted);
}

void sensivityevt(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    int val = lv_slider_get_value(slider);
    setSensitivity(val);
}

void startcal(lv_event_t * e)
{
    (void)e;
    startCalibration();
}

void bindTXevt(lv_event_t * e)
{
    (void)e;
    startBinding();
}

void rxChange(lv_event_t * e)
{
    lv_obj_t * dd = lv_event_get_target(e);
    uint16_t sel = lv_dropdown_get_selected(dd);

    extern void setActiveRX(uint8_t rx);
    setActiveRX((uint8_t)sel);
}

void lowPowerEvt(lv_event_t * e)
{
    (void)e;
    setRFPower(0);
    updatePowerButtonStyles(0);
}

void medPowerEvt(lv_event_t * e)
{
    (void)e;
    setRFPower(1);
    updatePowerButtonStyles(1);
}

void higPowerEvt(lv_event_t * e)
{
    (void)e;
    setRFPower(2);
    updatePowerButtonStyles(2);
}
