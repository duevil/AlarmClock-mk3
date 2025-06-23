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


//! Alarm events
EVENT_DEFINE(ALARM_EVENT);

enum ALARM_EVENT_ID
{
    //! Any alarm was triggered
    TRIGGERED_ANY,
    //! Alarm 1 was triggered
    TRIGGERED_1,
    //! Alarm 2 was triggerd
    TRIGGERED_2,
    //! The current alarm was snoozed
    SNOOZED,
    //! The current alarm was deactivated
    DEACTIVATED
};


#endif //EVENT_DEFINITIONS_H
