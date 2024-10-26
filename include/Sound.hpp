#ifndef SOUND_HPP
#define SOUND_HPP


struct Sound {
    uint8_t index{0};
    const char *name{nullptr};
    const char *path{nullptr};
    bool allowRandom{true};
    bool wasPlayed{false};

    [[nodiscard]] File getFile() const { return path ? SD.open(path) : File{}; }

    bool operator==(const Sound &other) const { return name == other.name; }

    void toJson(JsonObject &root) const {
        root["index"] = index;
        root["name"] = name;
        root["path"] = path;
        root["allowRandom"] = allowRandom;
    }

    void fromJson(const JsonVariant &json) {
        index = json["index"];
        name = json["name"];
        path = json["path"];
        allowRandom = json["allowRandom"];
    }

    static const Sound S_RANDOM;
    static const Sound S_DEFAULT;
};

const inline Sound Sound::S_RANDOM{.name = "rnd"};
const inline Sound Sound::S_DEFAULT{.name = "def"};


#endif //SOUND_HPP
