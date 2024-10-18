#include "audio.h"

extern const uint8_t default_sound_start[] asm("_binary_src_default_mp3_start");
extern const uint8_t default_sound_end[] asm("_binary_src_default_mp3_end");

using namespace audio_tools;

static NVSValue<uint8_t> volumeNVS{"volume", 80};
static MemoryManager mem{};
static I2SStream i2s{};
static VolumeStream volumeStream{};
static EncodedAudioOutput decodedOut{new MP3DecoderHelix()};
static Pipeline pipeline{};
static MetaDataOutput metaOut{};
static MultiOutput pipelineAndMetaOut{pipeline, metaOut};
static StreamCopy copier;
static FileLoop fileLoop{};
static MemoryStream defaultMp3{default_sound_start, default_sound_end - default_sound_start};

static const char *current_title = nullptr;

static void metaData(MetaDataType type, const char *data, int) {
    log_d("Metadata: [%s] %s", toStr(type), data);
    if (type == MetaDataType::Title) {
        current_title = data;
    }
}

struct : ModifyingOutput {
    Print *out = nullptr;

    void setOutput(Print &output) override { out = &output; }

    size_t write(const uint8_t *buffer, size_t size) override {
        if (out) {
            return out->write(buffer, size);
        }
        return size;
    }

    void setAudioInfo(AudioInfo newInfo) override {
        ModifyingOutput::setAudioInfo(newInfo);
        log_d("Audio info: sample rate: %d, bits per sample: %d, channels: %d",
              newInfo.sample_rate, newInfo.bits_per_sample, newInfo.channels);
    }
} info;

/*!
 * @brief Initializes the audio system and stream pipeline
 */
void audio::setup() {
#ifdef DEBUG
    AudioLogger::instance().begin(Serial, AudioLogger::Info);
#endif
    log_d("Audio setup");

    mem.begin((int) ESP.getPsramSize() / 2);

    auto cfg = i2s.defaultConfig();
    cfg.pin_ws = pins::I2S_LRC;
    cfg.pin_bck = pins::I2S_BCK;
    cfg.pin_data = pins::I2S_DATA;
    cfg.buffer_count = 8;
    cfg.buffer_size = 1024;
    if (!i2s.begin(cfg)) {
        log_e("Failed to start I2S");
        return;
    }

    log_i("Stored volume: %d", *volumeNVS);
    volumeStream.setVolume((float) *volumeNVS / 100.0f);

    pipeline.add(decodedOut);
    pipeline.add(volumeStream);
    pipeline.add(info);
    pipeline.setOutput(i2s);
    pipeline.begin();

    metaOut.setCallback(metaData);

    copier.setSynchAudioInfo(true);
    copier.setActive(false);

    defaultMp3.begin();
    log_d("Default sound size: %d", defaultMp3.available());

    log_d("Audio setup complete");
#ifdef DEBUG
    AudioLogger::instance().begin(Serial, AudioLogger::Warning);
#endif

    stop();
}

/*!
 * @brief Handles the copying of audio data and stops playback if no data is copied for more than a second
 */
void audio::loop() {
    static auto last = millis();
    auto written = copier.copy();
    if (written > 0) {
        last = millis();
    } else if (audio::isPlaying() && millis() - last > 1000) {
        log_i("No audio data copied for more than a second, stopping playback");
        stop();
        copier.end();
        copier.setActive(false);
    }
}

/*!
 * @brief Plays the specified sound
 * @param sound The sound to play. If not specified, the default sound is played
 * @param loop Whether to loop the sound
 */
void audio::play(const Sound &sound, bool loop) {
    stop();
    BaseStream *s;
    if (sound == Sound::S_DEFAULT) {
        log_i("Playing default sound");
        defaultMp3.begin();
        defaultMp3.setLoop(loop);
        s = &defaultMp3;
    } else if (auto file = sound.getFile()) {
        log_i("Playing file: %s", file.name());
        fileLoop.setFile(file);
        fileLoop.setLoopCount(loop ? -1 : 0);
        s = &fileLoop;
    } else {
        log_w("Sound file not found");
        return;
    }
    (*s).begin();
    metaOut.begin();
    copier.begin(pipelineAndMetaOut, *s);
    copier.setActive(true);
}

/*!
 * @brief Stops the current audio playback
 */
void audio::stop() {
    log_d("Stopping audio");
    current_title = nullptr;
    if (copier.available()) {
        copier.copyN(copier.minCopySize());
    }
    fileLoop.end();
    metaOut.end();
}

/*!
 * @brief Sets the volume of the audio stream
 * @param v The volume to set, in the range [0, 100]
 */
void audio::volume(uint8_t v) {
    volumeNVS = _min(v, 100);
    log_d("Setting volume to %d", *volumeNVS);
    volumeStream.setVolume((float) *volumeNVS / 100.0f);
}

/*!
 * @brief Increases the volume by 10
 */
void audio::volumeUp() { volume(*volumeNVS + 10); }

/*!
 * @brief Decreases the volume by 10
 */
void audio::volumeDown() { volume(*volumeNVS - 10); }

/*!
 * @brief Checks if audio is currently playing, i.e. if audio data is being copied
 * @return Whether audio is playing
 */
bool audio::isPlaying() { return copier.isActive(); }