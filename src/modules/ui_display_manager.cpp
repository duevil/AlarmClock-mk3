#include "ui_display_manager.h"

#include "log.h"
#include "u8g2_fonts.h"


UiDisplayManager::UiDisplayManager(
    [[maybe_unused]] uint8_t pin_cs, [[maybe_unused]] uint8_t pin_dc, [[maybe_unused]] uint8_t pin_rst,
    fds_t* form_definitions, std::initializer_list<muif_struct> fields
) : BootProcess("UI display initialized"),
    Thread({.name = "ui task", .priority = 5, .coreId = APP_CPU_NUM}),
    m_fields(fields),
#ifdef WOKWI
    m_display{U8G2_SSD1306_128X64_NONAME_F_HW_I2C{U8G2_R0}}
#else
    m_display{U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI{U8G2_R0, pin_cs, pin_dc, pin_rst}}
#endif
{
    m_ui.begin(m_display, form_definitions, m_fields.data(), m_fields.size());
}

void UiDisplayManager::goTo(uint8_t form_id)
{
    LOG_D("Form goto %d", form_id);
    m_ui.gotoForm(form_id, 0);
    redraw();
}

void UiDisplayManager::enter()
{
    goTo(1);
}

void UiDisplayManager::exit()
{
    LOG_D("Form leave");
    m_ui.leaveForm();
    redraw();
}

void UiDisplayManager::actionPrev()
{
    LOG_D("Form actionPrev");
    m_ui.prevField();
    redraw();
}

void UiDisplayManager::actionNext()
{
    LOG_D("Form actionNext");
    m_ui.nextField();
    redraw();
}

void UiDisplayManager::actionSelect()
{
    LOG_D("Form actionSelect");
    m_ui.sendSelect();
    redraw();
}

void UiDisplayManager::redraw()
{
    if (m_ui.isFormActive())
        m_close_timer.reset();
    else
        m_close_timer.stop();

    resume();
}

bool UiDisplayManager::active()
{
    return m_ui.isFormActive();
}

void UiDisplayManager::runBootProcess()
{
    m_display.begin();

    m_close_timer.once(15, [this]
    {
        m_ui.leaveForm();
        redraw();
    });
}

void UiDisplayManager::run()
{
    suspend();
    m_display.clearBuffer();

    if (m_ui.isFormActive())
        m_ui.draw();

    m_display.sendBuffer();
}
