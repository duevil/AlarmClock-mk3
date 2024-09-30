#include "alarm_clock.h"
#include "fonts.h"

//#region loading processes

static constexpr ui_bp_t uiBootProcessList[] = {
        {5,   "Booting...",      nullptr},
        {10,  "lights::setup()", lights::setup},
        {20,  "RTC setup",       rtc::setup},
        {100, "Done",            nullptr},
};

//#endregion
//#region functions

static constexpr muif_t uiFunctionList[] = {
        MUIF_U8G2_LABEL(),
        MUIF_U8G2_FONT_STYLE(1, fonts::cour<8>),
        MUIF_BUTTON("BN", mui_u8g2_btn_exit_wm_fi)
};

//#endregion
//#region form definitions

fds_t *UI_FORM_DEFINITION =
        ""
        MUI_FORM(1)
        MUI_STYLE(1)
        MUI_LABEL(5, 10, "Hello U8g2")
        MUI_XYT("BN", 20, 30, " Ok ")
        MUI_XYT("BN", 70, 30, " Cancel ")
        "";

//#endregion

const std::span<ui_bp_t> UI_BOOT_PROCESS_LIST(uiBootProcessList);
const std::span<muif_t> UI_FUNCTION_LIST(uiFunctionList);