#ifndef SOUND_HPP
#define SOUND_HPP


class Sound {
    const char *name;
    const char *path;
    bool allowRandom;

public:
    explicit Sound(const char *name, const char *path = nullptr, bool allowRandom = false)
            : name(name), path(path), allowRandom(allowRandom) {}

    [[nodiscard]] const char *getName() const { return name; }

    [[nodiscard]] File getFile() const { return path ? SD.open(path) : File{}; }

    [[nodiscard]] bool isRandomAllowed() const { return allowRandom; }

    bool operator==(const Sound &other) const { return name == other.name; }

    static const Sound S_RANDOM;
    static const Sound S_DEFAULT;
};

const inline Sound Sound::S_RANDOM{"rnd"};
const inline Sound Sound::S_DEFAULT{"def"};


#endif //SOUND_HPP
