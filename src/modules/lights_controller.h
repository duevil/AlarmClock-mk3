#ifndef LIGHTS_CONTROLLER_H
#define LIGHTS_CONTROLLER_H

#include "util/nvs.hpp"
#include "util/timer.h"
#include "util/boot_process.hpp"
#include "util/thread.hpp"

/**
 * Class for controller LEDC lights using events
 *
 * The lights can either be set to maximum brightness turned off
 * or set to a given value using the predefined events
 */
class LightsController final : BootProcess, Thread<1024>
{
public:
    LightsController() : BootProcess("Lights initialized"),
                         Thread({.name = "lights", .priority = 5, .coreId = APP_CPU_NUM}) {}

    /**
     * Get the current light value
     * @return The current light duty in percent
     */
    [[nodiscard]] uint8_t getCurrentValue() const;

    /**
     * Get an accessor to the NVS duration value
     * @return Reference to the duration value
     */
    [[nodiscard]] auto& getDuration();

    // delete copy constructor and assignment operator

    LightsController(const LightsController&) = delete;
    LightsController& operator=(const LightsController&) = delete;

private:
    void runBootProcess() override;
    void run() override;
    void max();
    void off();
    void set(uint8_t value);

    uint32_t m_target = 0;
    uint32_t m_current = 0;
    NVV<uint8_t> m_autoOffDuration{"light_duration"};
    Timer m_autoOffTimer;
};


#endif //LIGHTS_CONTROLLER_H
