#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

/**!
 * @file http_request.h
 * @brief Implementation of a simple asynchronous HTTP GET request handler
 */

#include "alarm_clock.h"

namespace http_request {
    using ResponseCallback = std::function<void(const char *, size_t)>;
    using JsonResponeCallback = std::function<void(const JsonDocument &)>;
    using JsonDocRef = const JsonDocument &;
    void get(const char *, const ResponseCallback &);
    void getJson(const char *, const JsonResponeCallback &, JsonDocRef = JsonDocument{});
}

#endif //HTTP_REQUEST_H
