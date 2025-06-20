#ifndef EVENTS_HPP
#define EVENTS_HPP


#ifdef ARDUINO
extern "C" {
#include <esp_event_base.h>
}
#else
using esp_event_base_t = const char *;
#endif


namespace events {
    void post(esp_event_base_t base, int32_t id, void *args);
    void handle(esp_event_base_t base, int32_t id, void *args);
}


#endif //EVENTS_HPP