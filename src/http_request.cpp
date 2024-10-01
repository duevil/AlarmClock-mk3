#include "http_request.h"

using namespace http_request;
using Str = std::string;

struct AsyncRequest {
    AsyncClient client{}; // Client object to handle the connection
    Str responseBuffer{}; // Accumulate data
    size_t contentLength{0}; // Expected length of the response body (if known)
    size_t bodyReceived{0}; // Track how much body data has been received
    bool headersProcessed{false}; // Flag to indicate headers are processed
};

using RequestPtr = std::unique_ptr<AsyncRequest>;

struct UrlParts {
    Str host{};
    uint16_t port{80}; // Default HTTP port
    Str path{};
};


static void clearRequest(const RequestPtr &r);
static void handleConnect(const RequestPtr &r, AsyncClient *client, const char *requestMsg);
static void handleData(const RequestPtr &r, const void *data, size_t len, const ResponseCallback &cb);
static void handleError(const RequestPtr &r, AsyncClient *client, int error);
static void handleTimeout(const RequestPtr &r, uint32_t time);
static void handleDisconnect(const RequestPtr &r);
static UrlParts parseUrl(const Str &url);
static void jsonHandler(const char *data, size_t len, const JsonResponeCallback &cb, JsonDocRef filter);


// Store all active requests in a set to manage their lifetimes while they are in progress.
// Using a set ensures that each request is unique and prevents duplicates, while also
// allowing for efficient lookup and removal of requests using the hash-value of the shared_ptr-wrapper. 
static std::unordered_set<RequestPtr> requests{};


/**
 * @brief Create and send a new asynchronous HTTP GET request
 *
 * @param url URL to send the request to
 * @param cb Callback to handle the response data if the request is successful
 * @note The callback will only be called if the request is successful and the response is fully received
 */
void http_request::get(const char *url, const ResponseCallback &cb) {
    // Create a new request object and store it in the set
    auto &r = *requests.emplace(std::make_unique<AsyncRequest>()).first;

    // Parse URL and prepare the request message
    auto parts = parseUrl(url);
    auto msg = "GET " + parts.path + " HTTP/1.1\r\n" + "Host: " + parts.host + "\r\n" + "Connection: close\r\n\r\n";

    // Set event handlers
    r->client.onConnect([&r, msg](void *, AsyncClient *c) { handleConnect(r, c, msg.c_str()); });
    r->client.onPoll([&r](void *, AsyncClient *c) { log_d("Polling..."); });
    r->client.onData([&r, cb](void *, AsyncClient *, void *data, size_t len) { handleData(r, data, len, cb); });
    r->client.onError([&r](void *, AsyncClient *c, int error) { handleError(r, c, error); });
    r->client.onTimeout([&r](void *, AsyncClient *, uint32_t time) { handleTimeout(r, time); });
    r->client.onDisconnect([&r](void *, AsyncClient *) { handleDisconnect(r); });

    // Start connection to the server
    r->client.setRxTimeout(5000); // 5 seconds timeout
    auto res = r->client.connect(parts.host.c_str(), parts.port);
    if (!res) {
        log_w("Failed to connect to %s:%d", parts.host.c_str(), parts.port);
        clearRequest(r);
    } else {
        log_i("Connected to %s:%d", parts.host.c_str(), parts.port);
    }
}

/**
 * @brief Create and send a new asynchronous HTTP GET request, expecting a JSON response
 *
 * @param url URL to send the request to
 * @param cb Callback to handle the parsed JSON data if the request is successful
 * @param filter JSON filter to select only specific fields from the response
 * @note The callback will only be called if the request is successful and the response is fully received
 */
void http_request::getJson(const char *url, const JsonResponeCallback &cb, JsonDocRef filter) {
    get(url, [cb, filter](const char *data, size_t len) { jsonHandler(data, len, cb, filter); });
}


static void clearRequest(const RequestPtr &r) {
    // Remove the request from the set, deleting it and freeing resources
    auto it = requests.find(r);
    if (it != requests.end()) {
        requests.erase(it);
        log_d("Request cleared, remaining: %d", requests.size());
    } else {
        log_w("Request not found, possible memory leak");
    }
}

void handleConnect(const RequestPtr &r, AsyncClient *client, const char *requestMsg) {
    log_i("Connected to server");
    if (!client->canSend()) {
        log_w("Cannot send data");
        clearRequest(r);
        return;
    }
    client->write(requestMsg);
}

void handleData(const RequestPtr &_r, const void *data, size_t len, const ResponseCallback &cb) {
    auto &r = *_r;
    // Append new data to the response buffer
    r.responseBuffer.append(static_cast<const char *>(data), len);

    // Process headers if not yet processed
    if (!r.headersProcessed) {
        auto headersEnd = r.responseBuffer.find("\r\n\r\n");
        if (headersEnd != Str::npos) {
            r.headersProcessed = true;

            // Check for Content-Length in headers, so we know how much data to expect
            const auto &headers = r.responseBuffer.substr(0, headersEnd);
            auto clPos = headers.find("Content-Length:");
            if (clPos != Str::npos) {
                clPos += 15;  // Move past "Content-Length: "
                auto endPos = headers.find("\r\n", clPos);
                if (endPos != Str::npos) {
                    r.contentLength = std::stoul(headers.substr(clPos, endPos - clPos));
                    log_i("Content-Length: %d", r.contentLength);
                }
            }

            // Remove headers from the buffer, keep only the body part (if any)
            r.responseBuffer.erase(0, headersEnd + 4);
        }
    } else {
        r.bodyReceived += len;
    }

    // If content-length is known and all body data is received, process it
    if (r.contentLength > 0 && r.responseBuffer.length() >= r.contentLength) {
        cb(r.responseBuffer.c_str(), r.contentLength);
        // Clear the buffer after processing the response
        r.responseBuffer.clear();
    }
}

void handleError(const RequestPtr &r, AsyncClient *client, int error) {
    (void) client; // avoid unused parameter warning when log_w is disabled
    (void) error; // avoid unused parameter warning when log_w is disabled
    log_w("Error: %s", client->errorToString(error));
}

void handleTimeout(const RequestPtr &r, uint32_t time) {
    (void) time; // avoid unused parameter warning when log_w is disabled
    log_w("Timeout at %lu", time);
    clearRequest(r);
}

void handleDisconnect(const RequestPtr &r) {
    log_d("Disconnected");
    clearRequest(r);
}

UrlParts parseUrl(const Str &url) {
    UrlParts parts{};

    Str::size_type hostStart = 0;
    Str::size_type hostEnd = 0;

    if (url.find("http://") == 0) {
        hostStart = 7;
    } else if (url.find("https://") == 0) {
        hostStart = 8;
    }

    hostEnd = url.find('/', hostStart);
    if (hostEnd == Str::npos) {
        hostEnd = url.length();
        parts.path = "/";
    } else {
        parts.path = url.substr(hostEnd);
    }

    auto colonIndex = url.find(':', hostStart);
    if (colonIndex != Str::npos) {
        parts.host = url.substr(hostStart, colonIndex - hostStart);
        auto _port = std::stoi(url.substr(colonIndex + 1, hostEnd - colonIndex - 1));
        parts.port = static_cast<uint16_t>(_port);
    } else {
        parts.host = url.substr(hostStart, hostEnd - hostStart);
    }

    return parts;
}

void jsonHandler(const char *data, size_t len, const JsonResponeCallback &cb, JsonDocRef filter) {
    JsonDocument doc{};
    // Parse JSON data and call the callback with the parsed document, filtering out unwanted fields
    auto error = deserializeJson(doc, data, len, DeserializationOption::Filter(filter));
    if (error) {
        log_e("Failed to parse JSON: %s", error.c_str());
    } else {
        cb(doc);
    }
}
