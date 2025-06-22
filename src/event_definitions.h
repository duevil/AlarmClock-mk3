#ifndef EVENT_DEFINITIONS_H
#define EVENT_DEFINITIONS_H

#include "util/events.hpp"


//! Events for controlling the lights
EVENT_DEFINE(LIGHTS_EVENT);

enum LIGHTS_EVENT_ID
{
    //! Turn the lights off
    SET_OFF,
    //! Turn the lights to their maximum brightness
    SET_MAX,
    //! Set the lights to the value provided in the event data (in %)
    SET_VALUE,
    //! Lights fading has completed
    FADE_DONE
};


#endif //EVENT_DEFINITIONS_H
