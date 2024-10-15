#include "http_request.h"

using namespace http_request;

struct request {
    ResponseCallback cb{};
    const String &url{};
    const JsonDocument &filter{};
};

struct visitor {
    const JsonDocument &doc{};
    const String &payload{};

    void operator()(const StringResponseCallback &cb) const { cb(payload); }

    void operator()(const JsonResponseCallback &cb) const { cb(doc); }
};

static WiFiClient client;
static QueueHandle_t requestQueue;
static StaticTask_t requestTaskHandle;
static StackType_t requestTaskStack[8192];

static void __attribute__((noreturn)) requestTask(void *) {
    request req{};
    while (true) {
        // Wait for a new request to be received
        if (xQueueReceive(requestQueue, &req, portMAX_DELAY) != pdTRUE) {
            log_w("Failed to receive request from queue");
            continue;
        }
        HTTPClient http;
        http.setReuse(false);
        http.setConnectTimeout(5000);
        http.setTimeout(5000);
        if (!http.begin(client, req.url)) {
            log_w("Failed to begin HTTP request");
            http.end();
            continue;
        }
        http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
        auto awaitJson = req.cb.index() == 1;
        if (awaitJson) {
            http.addHeader("Accept", "application/json");
            http.useHTTP10(true);
        }
        log_d("Sending HTTP request to %s", req.url.c_str());
        if (auto result = http.GET(); result <= 0) {
            log_w("Failed to send HTTP request: %s", http.errorToString(result).c_str());
            http.end();
            continue;
        }
        log_i("HTTP request successful");
        log_v("received %d bytes", http.getSize());
        if (awaitJson) {
            // TODO: use chunked stream parsing: https://arduinojson.org/v7/how-to/use-arduinojson-with-httpclient/
            JsonDocument doc{};
            log_v("Parsing JSON with filter: %s", [&req] {
                String s;
                serializeJson(req.filter, s);
                return s;
            }().c_str());
            // filter and parse the JSON response from the HTTP request message stream
            DeserializationError error = deserializeJson(doc,
                                                         http.getStream(),
                                                         DeserializationOption::Filter(req.filter));
            if (error) {
                log_w("Failed to parse JSON: %s", error.c_str());
            } else {
                log_v("Calling JSON callback");
                std::visit(visitor{.doc = doc}, req.cb);
            }
        } else {
            log_v("Calling string callback");
            String payload = http.getString();
            std::visit(visitor{.payload = payload}, req.cb);
        }
        log_d("Finished processing HTTP request, closing connection");
        http.end();
    }
}


/*!
 * @brief Create and send a new HTTP GET request. The request is queued and processed asynchronously by a FreeRTOS task
 * @param url The URL to send the request to
 * @param cb The callback function to call when the request is complete. Can either accept a string plus size or a
 * JSON document
 * @param filter [OPTIONAL] The JSON filter to apply to the response data
 */
void http_request::get(const String &url, const ResponseCallback &cb, const JsonDocument &filter) {
    if (static bool initialized = false; !initialized) {
        // if the request queue and task are not yet created, create them
        // the queue is created using PSRAM memory allocation to not use up the main heap
        // the queue allows up to 5 requests to be queued at once
        requestQueue = xQueueCreateWithCaps(5, sizeof(request), MALLOC_CAP_SPIRAM);
        // create the request task pinned to core 1 (APP_CPU) and statically to avoid heap allocation
        xTaskCreateStaticPinnedToCore(
                requestTask,
                "http_request",
                sizeof(requestTaskStack) / sizeof(StackType_t),
                nullptr,
                configMAX_PRIORITIES - 1,
                requestTaskStack,
                &requestTaskHandle,
                0); // pin to PRO_CPU core
        initialized = true;
    }
    log_d("Queueing HTTP request to %s", url.c_str());
    request req{cb, url, filter};
    if (xQueueSend(requestQueue, &req, 0) != pdTRUE) {
        log_w("Failed to send request to queue");
    }
}
