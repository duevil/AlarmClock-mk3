#include "webserver.h"

// TODO: add wifi management

static AsyncWebServer server(80);

static void onOTAStart();
static void onOTAProgress(size_t, size_t);
static void onOTAEnd(bool);

void webserver::setup() {
    // setup WiFi
    WiFiClass::mode(WIFI_MODE_STA);
    WiFiClass::setHostname("AlarmClock-mk3");
    WiFi.begin();
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        log_w("WiFi Failed, falling back to AP mode");
        // TODO: setup AP mode
    } else {
        log_i("Connected to %s, IP address: %s", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    }

    // setup ElegantOTA
    ElegantOTA.begin(&server);
    ElegantOTA.onStart(onOTAStart);
    ElegantOTA.onProgress(onOTAProgress);
    ElegantOTA.onEnd(onOTAEnd);

    // setup server
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Hello, world");
    });
    server.begin();

    // setup mDNS
    if (!MDNS.begin("alarmclock")) {
        log_w("Error setting up mDNS");
    }
}

void webserver::loop() { ElegantOTA.loop(); }


static void onOTAStart() { log_i("OTA Start"); }

static void onOTAProgress(size_t progress, size_t total) {
    if (static auto last = millis(); millis() - last < 1000) return;
    log_v("OTA Progress: %u%%", (progress / total) * 100);
}

static void onOTAEnd(bool success) { log_i("OTA End, update %s", success ? "succeeded" : "failed"); }
