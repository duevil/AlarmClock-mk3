#ifndef EVENT_DEFINITIONS_H
#define EVENT_DEFINITIONS_H

#include "util/events.hpp"


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
    //! Set the current alarm to snooze
    SNOOZE,
    //! Deactivate the current alarm
    DEACTIVATE
};


//! Input events
EVENT_DEFINE(INPUT_EVENT);

enum INPUT_EVENT_ID
{
    //! The left button was clicked
    CLICK_LEFT,
    //! The left button was long pressed
    LONG_PRESS_LEFT,
    //! The left button was released after a long press
    LONG_RELEASE_LEFT,
    //! Fires repeatedly after the left button was long pressed
    REPEATING_LEFT,
    //! The middle button was clicked
    CLICK_MIDDLE,
    //! The middle button was long pressed
    LONG_PRESS_MIDDLE,
    //! The middle button was released after a long press
    LONG_RELEASE_MIDDLE,
    //! Fires repeatedly after the middle button was long pressed
    REPEATING_MIDDLE,
    //! The right button was clicked
    CLICK_RIGHT,
    //! The right button was long pressed
    LONG_PRESS_RIGHT,
    //! The right button was released after a long press
    LONG_RELEASE_RIGHT,
    //! Fires repeatedly after the right button was long pressed
    REPEATING_RIGHT,
};


#endif //EVENT_DEFINITIONS_H
