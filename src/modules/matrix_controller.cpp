#include "matrix_controller.h"

#include "log.h"
#include "matrix_font.h"


void MatrixController::overrideText(const char* text)
{
    m_md.setTextBuffer(text);
    m_md.displayReset();
    m_md.displayAnimate();
}

void MatrixController::shutdown(bool shutdown)
{
    m_md.displayShutdown(shutdown);
}

void MatrixController::setBrightness(uint8_t brightness)
{
    m_md.setIntensity(brightness);
}

void MatrixController::setMaxBrightness()
{
    m_md.setIntensity(15);
}

void MatrixController::scrollNext()
{
    m_animation = m_last_animation = scroll_next;
    m_new_tab = &m_tabs[(m_current_tab->index + 1) % m_tabs.size()];
}

void MatrixController::scrollPrev()
{
    m_animation = m_last_animation = scroll_prev;
    m_new_tab = &m_tabs[(m_current_tab->index + m_tabs.size() - 1) % m_tabs.size()];
}

void MatrixController::scrollToStart()
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

void MatrixController::scrollTo(uint8_t index)
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

void MatrixController::runBootProcess()
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

void MatrixController::run()
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
