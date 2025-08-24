#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include "util/boot_process.hpp"
#include <optional>
#include <ArduinoJson/Variant/JsonVariant.hpp>


// TODO: Test with real hardware!


// forward declaration necessary for Sound::Proxy class
class SoundManager;


/**
* Class representing MP3 sounds stored on the SD card
*/
class Sound
{
    friend class SoundManager;
    friend class Proxy;
    // for using number to find a sound in the collection
    explicit(false) Sound(uint8_t number);
    // allow modification through const iterator/reference returned from unordered_set
    mutable bool was_played{};

public:
    class Proxy;
    using Collection = std::unordered_set<Sound>;
    using Optional = std::optional<Proxy>;

    Sound(uint8_t number, const char* path, const char* name, bool allow_random);
    Sound(uint8_t number, String path, const char* name, bool allow_random);
    Sound(uint8_t number, const char* path, String name, bool allow_random);
    Sound(uint8_t number, String path, String name, bool allow_random);
    // needed for unordered_set
    bool operator==(const Sound& other) const { return number == other.number; }

    uint8_t number;
    String path;
    String name;
    bool allow_random;


    /**
     * Sound proxy for using RAII to automatically write attribute changes to the config file;
     * instances of this class must not be stored but only be used as temporary objects
     */
    class Proxy
    {
        SoundManager& m_sound_manager;

    public:
        Proxy(const Sound& sound, SoundManager& sound_manager) noexcept;
        // don't allow copying
        Proxy(Proxy&) = delete;
        // allow moving for use as with optional
        Proxy(Proxy&&) = delete;

        /**
         * Destructor checks the proxied sound for changes and updates the config file if necessary
         */
        ~Proxy();

        //! The sound number
        const uint8_t number;
        //! Whether this sound is eligible to be played randomly
        bool allow_random;
        //! the display name of the sound
        String name;
        //! The path to the sound's MP3 file
        String path;
    };
};


// hash specialization for Sound hashing; a sound is hashed by its id number
template <>
struct std::hash<Sound>
{
    size_t operator()(const Sound& sound) const noexcept { return std::hash<uint8_t>()(sound.number); }
};

// JSON converter specialization for converting sounds from and to JSON
template <>
struct ArduinoJson::Converter<Sound>
{
    static void toJson(const Sound& src, JsonVariant dst);
    static Sound fromJson(JsonVariantConst src);
    static bool checkJson(JsonVariantConst src);
};


/**
 * Class for managing sounds stored on the SD card
 * and a corresponding configuration file specifying sound attributes;
 * its boot process tries to load the sound configuration from a file
 * or else generates the configuration
 */
class SoundManager final : BootProcess
{
    using Collection = Sound::Collection;
    using Optional = Sound::Optional;

public:
    /**
     * Constructor
     * @param config_file The path to the sound configuration file
     */
    explicit SoundManager(const char* config_file);
    /**
     * Stores the sound attributes held by the underlying collection
     */
    void store() const;
    /**
     * Gets an accessor to a requested sound
     * or to a random sound if the number equals 0
     * @param number The sound id number to access; 0 for getting a random sound
     * @return An optional accessor to the requested sound
     *         that will automatically write changes performed to the sound when going out of scope
     *         or an empty optional if the requested sound wasn't found
     *         or no sound could be randomly selected
     * @note A sound is eligible for random selection if its 'allow_random' flag is set to true
     *       and wasn't yet selected in the current selection cycle,
     *       resulting in all possible sounds being selected once without any double occurrence
     */
    Optional operator[](uint8_t number);
    /**
     * Gets the number of managed sounds
     * @return The size of the underlying sound collection
     */
    [[nodiscard]] size_t size() const { return m_sounds.size(); }
    /**
     * Gets the collection of the sounds managed
     * @return An unmodifiable reference to the underlying sound collection
     */
    [[nodiscard]] const Collection& getSounds() const { return m_sounds; }
    /**
     * Overwrites the collection of managed sounds
     * @param sounds The sound collection to overwrite the current one with
     */
    void setSounds(const Collection& sounds);
    void setSound(const Sound& sound);

private:
    void runBootProcess() override;
    Optional selectRandom();
    std::optional<const Sound> trySelectRandom();
    inline Optional makeProxy(const Sound& sound);

    const char* m_config_file;
    Collection m_sounds{};
};


#endif //SOUND_MANAGER_H
