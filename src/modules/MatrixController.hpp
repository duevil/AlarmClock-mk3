#ifndef MATRIXCONTROLLER_HPP
#define MATRIXCONTROLLER_HPP

#include "matrix_font.h"
#include <MD_Parola.h>


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
    void overrideText(const char* text)
    {
        m_md.setTextBuffer(text);
        m_md.displayReset();
        m_md.displayAnimate();
    }

    /**
     * @brief Shutdowns the display
     * @param shutdown True to shut down the display, false to turn it on
     */
    void shutdown(bool shutdown)
    {
        m_md.displayShutdown(shutdown);
    }

    /**
     * @brief Sets the brightness of the display
     * @param brightness The brightness to be set (0-15)
     */
    void setBrightness(uint8_t brightness)
    {
        m_md.setIntensity(brightness);
    }

    /**
     * @brief Sets the brightness of the display to the maximum value (15)
     */
    void setMaxBrightness()
    {
        m_md.setIntensity(15);
    }

    /**
     * @brief Scrolls to the next tab
     */
    void scrollNext()
    {
        m_animation = m_last_animation = scroll_next;
        m_new_tab = &m_tabs[(m_current_tab->index + 1) % m_tabs.size()];
    }

    /**
     * @brief Scrolls to the previous tab
     */
    void scrollPrev()
    {
        m_animation = m_last_animation = scroll_prev;
        m_new_tab = &m_tabs[(m_current_tab->index + m_tabs.size() - 1) % m_tabs.size()];
    }

    /**
     * @brief Scrolls to the start of the tab list
     */
    void scrollToStart()
    {
        // scroll to start only if not already at start
        if (m_current_tab->index == 0) return;

        // scroll direction against the last direction
        if (m_last_animation == scroll_next)
            m_animation = scroll_prev;
        else
            m_animation = scroll_next;

        m_new_tab = &m_tabs.front();
    }

    /**
     * Scroll to a specific tab
     * @param index The index of the tab to sroll to
     */
    void scrollTo(uint8_t index)
    {
        if (index >= m_tabs.size() || index == m_current_tab->index)
        {
            return;
        }

        if (_abs(index - m_current_tab->index) < m_tabs.size() / 2)
            m_animation = scroll_next;
        else
            m_animation = scroll_prev;

        m_new_tab = &m_tabs[index];
    }

    // delete copy constructor and assignment operator
    MatrixController(const MatrixController&) = delete;
    MatrixController& operator=(const MatrixController&) = delete;

private:
    void runBootProcess() override
    {
        if (m_md.begin())
        {
            LOG_I("Matrix initialized successfully");
            m_md.setIntensity(7);
            m_md.setFont(matrix_font);
            m_md.setTextAlignment(PA_CENTER);
            m_md.setCharSpacing(0);
            m_md.setScrollSpacing(c_scroll_spacing);
            m_md.setTextEffect(PA_NO_EFFECT, PA_NO_EFFECT);

            m_scroll_home_timer.once(10, [this] { scrollToStart(); });
            m_initialized = true;
            resume();
        }
        else
        {
            LOG_E("Matrix initialization failed");
        }
    }

    void run() override
    {
        if (!m_initialized)
        {
            // wait until the matrix was initialized
            suspend();
        }
        m_current_tab->textSupplier(m_buf, std::size(m_buf));
        m_md.setTextBuffer(m_buf);
        if (m_md.displayAnimate())
        {
            switch (m_animation)
            {
            case none:
                // give the processor some time to do other stuff when not animating
                delay(10);
                break;
            case scroll_next:
                m_md.setTextEffect(PA_NO_EFFECT, PA_SCROLL_LEFT);
                m_animation = scroll_next_ongoing;
                break;
            case scroll_next_ongoing:
                m_md.setTextEffect(PA_SCROLL_LEFT, PA_NO_EFFECT);
                m_current_tab = m_new_tab;
                m_animation = scroll_finish;
                break;
            case scroll_prev:
                m_md.setTextEffect(PA_NO_EFFECT, PA_SCROLL_RIGHT);
                m_animation = scroll_prev_ongoing;
                break;
            case scroll_prev_ongoing:
                m_md.setTextEffect(PA_SCROLL_RIGHT, PA_NO_EFFECT);
                m_current_tab = m_new_tab;
                m_animation = scroll_finish;
                break;
            case scroll_finish:
                m_md.setTextEffect(PA_NO_EFFECT, PA_NO_EFFECT);
                m_animation = none;

                if (m_current_tab->index != 0)
                {
                    m_scroll_home_timer.reset();
                }

                break;
            }
            m_md.displayReset();
        }
    }

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


#endif //MATRIXCONTROLLER_HPP
