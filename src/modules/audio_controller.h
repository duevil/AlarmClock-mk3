#ifndef AUDIO_CONTROLLER_H
#define AUDIO_CONTROLLER_H

#include "util/boot_process.hpp"
#include "util/thread.hpp"
#include "util/nvs.hpp"
#include "default_mp3.h"
#include <AudioTools.h>
#include <AudioTools/AudioLibs/MemoryManager.h>
#include <AudioTools/AudioCodecs/CodecMP3Helix.h>
#include <AudioTools/Disk/FileLoop.h>


/**
 * Class for controlling the audio playback and volume
 */
class AudioController final : BootProcess, Thread<>
{
public:
    AudioController(uint8_t pin_data, uint8_t pin_bck, uint8_t pin_lrc);

    /**
     * Play a sound file at the given path once
     * @param path The path of the sound file to play; if omitted, the default sound is played
     */
    void play(const char* path= nullptr);

    /**
     * Play a sound file at the given path in a loop
     * @param path The path of the sound file to play; if omitted, the default sound is played
     */
    void playLooped(const char* path= nullptr);

    /**
     * Stops the current playback
     */
    void stop();

    /**
     * Get an accessor the audio volume NVS value
     * @return A reference to the audio volume value
     */
    NVV<uint8_t>& volume();

private:
    void runBootProcess() override;
    void run() override;

    // ReSharper disable once CppPolymorphicClassWithNonVirtualPublicDestructor
    struct AudioSource final : audio_tools::AudioSource
    {
        void begin() override;
        Stream* nextStream(int) override;
        Stream* selectStream(const char* path) override;
        void setLoop(bool loop);

    private:
        audio_tools::FileLoop file_loop{};
        audio_tools::MemoryStream default_mp3{default_mp3_start, default_mp3_end - default_mp3_start};
    };

    NVV<uint8_t> m_volume{"volume", 50};
    audio_tools::I2SConfig m_i2s_config{};
    audio_tools::MemoryManager m_memory_manager{};
    audio_tools::I2SStream m_i2s{};
    audio_tools::MP3DecoderHelix m_decoder{};
    AudioSource m_source{};
    audio_tools::AudioPlayer m_player{m_source, m_i2s, m_decoder};
};


#endif //AUDIO_CONTROLLER_H
