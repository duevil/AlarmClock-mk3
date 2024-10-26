#include "sounds.h"

using namespace sounds;

static SoundCol soundCol{};
static JsonDocument doc{};

static bool loadFromSD();
static bool queryFromSD();


void sounds::setup() {
    if (loadFromSD() || queryFromSD()) {
        log_i("Loaded %d sounds", soundCol.size());
    } else {
        log_w("No sounds found");
    }
}

const SoundCol &sounds::get() { return soundCol; }

const Sound &sounds::get(uint8_t index) { return soundCol[index]; }

JsonDocument sounds::json() {
    doc.clear();
    JsonArray root = doc.to<JsonArray>();
    for (auto const &sound: soundCol) {
        JsonObject obj = root.add<JsonObject>();
        sound.toJson(obj);
    }
    return doc;
}

void sounds::saveToSD() {
    log_i("Saving sounds to SD card");
    auto file = SD.open("/sounds.json", FILE_WRITE);
    if (!file) {
        log_e("Failed to open sounds.json for writing");
        return;
    }
    auto doc = json();
    auto writen = serializeJson(doc, file);
    if (!writen) {
        log_e("Failed to write sounds.json");
    } else {
        log_d("Wrote %d bytes to sounds.json", writen);
    }
}

const Sound &sounds::getRandom() {
    std::vector<std::reference_wrapper<Sound>> eligible{};
    for (auto &sound: soundCol) {
        if (sound.allowRandom && !sound.wasPlayed) eligible.emplace_back(sound);
    }
    if (eligible.empty()) {
        // reset all sounds
        log_i("Resetting all sounds");
        for (auto &sound: soundCol) {
            sound.wasPlayed = false;
            if (sound.allowRandom) eligible.emplace_back(sound);
        }
    }
    if (eligible.empty()) return Sound::S_DEFAULT;
    const auto rIndex = random(0, static_cast<uint8_t>(eligible.size()));
    const auto &sound = eligible[rIndex];
    sound.get().wasPlayed = true;
    log_d("Choose sound %s", sound.get().name);
    return sound;
}


static bool queryFromSD() {
    soundCol.clear();
    // query SD card for any MP3 files
    auto root = SD.open("/");
    if (!root) return false;
    uint8_t index = 0;
    while (auto file = root.openNextFile()) {
        if (file.isDirectory()) continue;
        if (const auto name = String(file.name()); name.endsWith(".mp3")) {
            Sound sound;
            sound.index = index++;
            sound.name = name.c_str();
            sound.path = name.c_str();
            soundCol[sound.index] = sound;
        }
        file.close();
    }
    root.close();
    if (soundCol.empty()) {
        log_w("No sounds found on SD card");
        return false;
    }
    saveToSD();
    return true;
}

static bool loadFromSD() {
    soundCol.clear();
    auto file = SD.open("/sounds.json");
    if (!file) return false;
    if (auto error = deserializeJson(doc, file); error) {
        log_e("Failed to parse sounds.json: %s", error.c_str());
        return false;
    }
    auto root = doc.as<JsonArray>();
    for (auto json: root) {
        Sound sound;
        sound.fromJson(json);
        soundCol[sound.index] = sound;
    }
    return true;
}
