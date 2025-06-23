#ifndef MATRIXCONTROLLER_H
#define MATRIXCONTROLLER_H

#include <MD_Parola.h>
#include "util/timer.h"
#include "util/boot_process.hpp"
#include "util/thread.hpp"


/**
 * @brief A class for controlling a 32x8 LED matrix display using the MD_Parola library.
 */
class MatrixController final : BootProcess, Thread<1792>
{
public:
    /**
     * Creates a new display controller and initializes the tab list
     * @param cs_pin The CS pin of the matrix's SPI interface
     * @param text_suppliers List of text suppliers representing each tab;
     * a text supplier must have the signature <code>void (char*, size_t)</code>
     */
    explicit MatrixController(uint8_t cs_pin, auto&&... text_suppliers)
        : BootProcess("Matrix initialized"),
          Thread({.name = "matrix loop", .priority = 2, .coreId = APP_CPU_NUM}),
          m_md(MD_MAX72XX::FC16_HW, cs_pin, 4)
    {
        m_tabs.reserve(sizeof...(text_suppliers) + 1);
        (m_tabs.emplace_back(m_tabs.size(), std::forward<decltype(text_suppliers)>(text_suppliers)), ...);
        m_current_tab = &m_tabs.front();
    }

    /**
     * @brief Overrides the text to be displayed, ignoring the text supplier and the scrolling animation
     * @param text The text to be displayed
     */
    void overrideText(const char* text);

    /**
     * @brief Shutdowns the display
     * @param shutdown True to shut down the display, false to turn it on
     */
    void shutdown(bool shutdown);

    /**
     * @brief Sets the brightness of the display
     * @param brightness The brightness to be set (0-15)
     */
    void setBrightness(uint8_t brightness);

    /**
     * @brief Sets the brightness of the display to the maximum value (15)
     */
    void setMaxBrightness();

    /**
     * @brief Scrolls to the next tab
     */
    void scrollNext();

    /**
     * @brief Scrolls to the previous tab
     */
    void scrollPrev();

    /**
     * @brief Scrolls to the start of the tab list
     */
    void scrollToStart();

    /**
     * Scroll to a specific tab
     * @param index The index of the tab to sroll to
     */
    void scrollTo(uint8_t index);

    // delete copy constructor and assignment operator

    MatrixController(const MatrixController&) = delete;
    MatrixController& operator=(const MatrixController&) = delete;

private:
    void runBootProcess() override;

    void run() override;

    enum Animation
    {
        none, scroll_next, scroll_next_ongoing, scroll_prev, scroll_prev_ongoing, scroll_finish,
    };

    struct Tab
    {
        uint8_t index;
        std::function<void(char*, size_t)> textSupplier;
    };

    static constexpr uint8_t c_scroll_spacing{32};

    MD_Parola m_md;
    Animation m_animation{none};
    Animation m_last_animation{none};
    std::vector<Tab> m_tabs{};
    Tab* m_current_tab{nullptr};
    Tab* m_new_tab{nullptr};
    char m_buf[12]{};
    Timer m_scroll_home_timer{};
    bool m_initialized{false};
};


#endif //MATRIXCONTROLLER_H
