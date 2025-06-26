#include "audio_controller.h"

#include <SD.h>


AudioController::AudioController(uint8_t pin_data, uint8_t pin_bck, uint8_t pin_lrc)
    : BootProcess("Audio initialized"),
      Thread({.name = "audio", .priority = 10, .coreId = APP_CPU_NUM})
{
    m_i2s_config.pin_data = pin_data;
    m_i2s_config.pin_bck = pin_bck;
    m_i2s_config.pin_ws = pin_lrc;
}

void AudioController::play(const char* path)
{
    if (m_player.setPath(path))
    {
        m_source.setLoop(false);
        m_player.play();
    }
}

void AudioController::playLooped(const char* path)
{
    if (m_player.setPath(path))
    {
        m_source.setLoop(true);
        m_player.play();
    }
}

void AudioController::stop()
{
    m_player.stop();
    suspend();
}

NVV<uint8_t>& AudioController::volume()
{
    return m_volume;
}

void AudioController::runBootProcess()
{
    m_volume.observe([this](uint8_t volume) { m_player.setVolume(static_cast<float>(volume) / 100.f); });

    m_memory_manager.begin(static_cast<int>(ESP.getPsramSize()) / 2);
    m_i2s.begin(m_i2s_config);
    m_player.setVolume(static_cast<float>(*m_volume) / 100.f);
    m_player.begin(-1, false);
    m_player.setAutoNext(false);

    suspend();
}

void AudioController::run()
{
    if (!m_player.copy())
    {
        suspend();
    }
}

void AudioController::AudioSource::begin()
{
    // nothing to do
}

Stream* AudioController::AudioSource::nextStream(int)
{
    file_loop.begin();
    return &file_loop;
}

Stream* AudioController::AudioSource::selectStream(const char* path)
{
    file_loop.end();
    default_mp3.end();

    if (!path)
    {
        default_mp3.begin();
        return &default_mp3;
    }

    if (path != file_loop.file().path())
    {
        if (auto file = SD.open(path))
        {
            file_loop.setFile(file);
            file_loop.begin();
            return &file_loop;
        }
    }

    return nullptr;
}

void AudioController::AudioSource::setLoop(bool loop)
{
    file_loop.setLoopCount(loop ? -1 : 0);
    default_mp3.setLoop(loop);
}
