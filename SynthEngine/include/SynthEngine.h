#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace synth {

enum class Waveform : std::uint8_t { Sine, Saw, Square, Triangle };

struct ParameterInfo {
    const char* id;
    const char* name;
    float minimum;
    float maximum;
    float defaultValue;
};

inline constexpr std::array<ParameterInfo, 8> kParameters{{
    {"master.gain", "Master Gain", 0.0f, 1.0f, 0.70f},
    {"osc.frequency", "Frequency", 20.0f, 20000.0f, 440.0f},
    {"osc.waveform", "Waveform", 0.0f, 3.0f, 0.0f},
    {"filter.cutoff", "Filter Cutoff", 20.0f, 20000.0f, 12000.0f},
    {"filter.resonance", "Filter Resonance", 0.1f, 10.0f, 0.707f},
    {"amp.attack", "Attack", 0.001f, 5.0f, 0.01f},
    {"amp.decay", "Decay", 0.001f, 5.0f, 0.15f},
    {"amp.release", "Release", 0.005f, 10.0f, 0.25f}
}};

class SynthEngine {
public:
    SynthEngine();
    void prepare(double sampleRate, std::size_t maxBlockSize = 512);
    void reset();
    void setParameter(const std::string& id, float value);
    float parameter(const std::string& id) const;
    void noteOn(int midiNote, float velocity = 1.0f);
    void noteOff();
    // Adds audio to output buffers; callers should clear buffers if silence is needed.
    void process(float* left, float* right, std::size_t numSamples);

private:
    static float clampParameter(std::size_t index, float value);
    float nextSample();
    double m_sampleRate = 44100.0;
    double m_phase = 0.0;
    float m_noteFrequency = 440.0f;
    float m_velocity = 0.0f;
    float m_envelope = 0.0f;
    bool m_gate = false;
    float m_filterState = 0.0f;
    std::array<std::atomic<float>, kParameters.size()> m_values;
};

} // namespace synth
