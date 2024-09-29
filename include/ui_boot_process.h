#ifndef UI_BOOT_PROCESS_H
#define UI_BOOT_PROCESS_H

#include <Arduino.h>

//! @brief Loading process structure.
struct BootProcess {
    uint8_t progress; //!< Progress of the loading process in percent.
    const char *message; //!< Message to display during the loading process.
    void (*func)(); //!< Function to execute during fot this loading process.
} __attribute__((packed));

using ui_bp_t = const struct BootProcess;

#endif //UI_BOOT_PROCESS_H
