#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

/*!
 * @file http_request.h
 * @brief Implementation of a simple HTTP GET request handler
 * @note The implementation uses a FreeRTOS queue to handle multiple requests simultaneously and a FreeRTOS task to
 * actually perform the requests.
 */

#include "alarm_clock.h"

namespace http_request {
    static const JsonDocument EMPTY_FILTER{}; //!< Empty filter for JSON responses
    using StringResponseCallback = std::function<void(const String &)>; //!< Callback for string responses
    using JsonResponseCallback = std::function<void(const JsonDocument &)>; //!< Callback for JSON responses
    using ResponseCallback = std::variant<StringResponseCallback, JsonResponseCallback>; //!< Callback for responses
    void get(const String &, const ResponseCallback &, const JsonDocument & = EMPTY_FILTER);
}

#endif //HTTP_REQUEST_H
