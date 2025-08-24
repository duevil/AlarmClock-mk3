#include "sound_manager.h"

#include "log.h"
#include "util/std_container_json.hpp"
#include "util/json_file.hpp";
#include <utility>


Sound::Sound(uint8_t number):
    number(number),
    allow_random() {}

Sound::Sound(uint8_t number, const char* path, const char* name, bool allow_random):
    number(number),
    path(path),
    name(name),
    allow_random(allow_random) {}

Sound::Sound(uint8_t number, String path, const char* name, bool allow_random) :
    number(number),
    path(std::move(path)),
    name(name),
    allow_random(allow_random) {}

Sound::Sound(uint8_t number, const char* path, String name, bool allow_random):
    number(number),
    path(path),
    name(std::move(name)),
    allow_random(allow_random) {}

Sound::Sound(uint8_t number, String path, String name, bool allow_random) :
    number(number),
    path(std::move(path)),
    name(std::move(name)),
    allow_random(allow_random) {}


Sound::Proxy::Proxy(const Sound& sound, SoundManager& sound_manager) noexcept:
    m_sound_manager(sound_manager),
    number(sound.number),
    allow_random(sound.allow_random),
    name(sound.name),
    path(sound.path) {}

Sound::Proxy::~Proxy()
{
    m_sound_manager.setSound(Sound{number, path, name, allow_random});
}


SoundManager::SoundManager(const char* config_file):
    BootProcess("Loaded sounds"),
    m_config_file(config_file) {}

void SoundManager::store() const
{
    LOG_D("Writing sounds to config file");
    JsonFile::w(m_config_file).set(m_sounds);
}

SoundManager::Optional SoundManager::operator[](uint8_t number)
{
    if (number == 0)
        return selectRandom();

    if (auto it = m_sounds.find(number); it != m_sounds.end())
        return makeProxy(*it);

    return std::nullopt;
}

void SoundManager::setSounds(const Collection& sounds)
{
    m_sounds = sounds;

    // ensure sound 0 represents random sound selection
    if (auto random_s = m_sounds.find(0); random_s == m_sounds.end())
    {
        m_sounds.emplace(0, nullptr, "RANDOM", false);
    }
    else if (random_s->allow_random)
    {
        if (auto node = m_sounds.extract(random_s))
        {
            auto& value = node.value();
            // sound 0 might never be picked randomly
            value.allow_random = false;
            value.path = static_cast<const char*>(nullptr);
            m_sounds.insert(std::move(node));
        }
    }

    store();
}

void SoundManager::setSound(const Sound& sound)
{
    if (auto node = m_sounds.extract(sound.number))
    {
        if (auto& current = node.value();
            sound.allow_random != current.allow_random ||
            sound.name != current.name ||
            sound.path != current.path)
        {
            current.allow_random = sound.allow_random;
            if (sound.name != current.name) current.name = sound.name;
            if (sound.path != current.path) current.path = sound.path;

            m_sounds.insert(std::move(node));
            store();
        }
    }
}

static void searchRecursive(Sound::Collection& col, File& dir, int d = 0) // NOLINT(*-no-recursion)
{
    for (auto entry = dir.openNextFile(); entry; entry = dir.openNextFile())
    {
        if (entry.isDirectory())
        {
            searchRecursive(col, entry, d + 1);
        }
        else if (String name{entry.name()}; name.endsWith(".mp3"))
        {
            auto index = name.indexOf(".mp3");
            col.emplace(col.size(), entry.path(), name.substring(0, index), true);
        }
        entry.close();
    }
}

void SoundManager::runBootProcess()
{
    auto json = JsonFile::r(m_config_file);
    if (auto err = json.error())
    {
        if (err == DeserializationError::EmptyInput)
        {
            LOG_W("Config file doesn't exist or is empty, generating config");
            m_sounds.clear();
            // sound 0 always represents random sound selection
            m_sounds.emplace(0, nullptr, "RANDOM", false);

            auto root = SD.open("/");
            if (root && root.isDirectory())
            {
                searchRecursive(m_sounds, root);
            }
            root.close();

            // size - 1: sound 0 represents a random sound thus doesn't add to real sound count
            LOG_I("Loaded %d sounds from file system", m_sounds.size() - 1);
            store();
        }
        else
        {
            LOG_E("Error loading config file: %s", err.c_str());
        }
    }
    else
    {
        setSounds(json.as<decltype(m_sounds)>());
        // size - 1: sound 0 represents a random sound thus doesn't add to real sound count
        LOG_I("Loaded %d sounds from config file", m_sounds.size() - 1);
    }
}

SoundManager::Optional SoundManager::selectRandom()
{
    if (auto s = trySelectRandom()) return makeProxy(*s);

    LOG_I("Could not select random sound, resetting played flags");
    for (auto& sound : m_sounds)
    {
        sound.was_played = false;
    }

    if (auto s = trySelectRandom()) return makeProxy(*s);
    return std::nullopt;
}

std::optional<const Sound> SoundManager::trySelectRandom()
{
    std::vector<uint8_t> candidates;
    candidates.reserve(m_sounds.size());

    for (auto& sound : m_sounds)
    {
        if (sound.allow_random && !sound.was_played)
        {
            candidates.push_back(sound.number);
        }
    }

    if (!candidates.empty())
    {
        uint8_t idx = random(static_cast<long>(candidates.size()));
        if (auto selected = m_sounds.find(candidates[idx]); selected != m_sounds.end())
        {
            selected->was_played = true;
            LOG_I("Randomly selected sound number %d", selected->number);
            return *selected;
        }
    }

    LOG_I("No eligible sounds found");
    return std::nullopt;
}

SoundManager::Optional SoundManager::makeProxy(const Sound& sound)
{
    return std::make_optional<Sound::Proxy>(sound, *this);
}

void Converter<Sound>::toJson(const Sound& src, JsonVariant dst)
{
    dst["id"] = src.number;
    dst["path"] = src.path.c_str();
    dst["name"] = src.name.c_str();
    dst["allow_random"] = src.allow_random;
}

Sound Converter<Sound>::fromJson(JsonVariantConst src)
{
    return {
        src["id"],
        src["path"].as<const char*>(),
        src["name"].as<const char*>(),
        src["allow_random"],
    };
}

bool Converter<Sound>::checkJson(JsonVariantConst src)
{
    return
        src["id"].is<uint8_t>() &&
        src["path"].is<const char*>() &&
        src["name"].is<const char*>() &&
        src["allow_random"].is<bool>();
}
