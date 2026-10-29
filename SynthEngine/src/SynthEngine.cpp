#include "SynthEngine.h"
#include <algorithm>
#include <cmath>

namespace synth {
namespace {
constexpr double kPi = 3.14159265358979323846;
}

SynthEngine::SynthEngine() {
    for (std::size_t i = 0; i < kParameters.size(); ++i)
        m_values[i].store(kParameters[i].defaultValue);
}

void SynthEngine::prepare(double sampleRate, std::size_t) {
    if (sampleRate > 1000.0) m_sampleRate = sampleRate;
    reset();
}

void SynthEngine::reset() {
    m_phase = 0.0;
    m_envelope = 0.0f;
    m_filterState = 0.0f;
    m_gate = false;
}

float SynthEngine::clampParameter(std::size_t i, float value) {
    return std::max(kParameters[i].minimum, std::min(kParameters[i].maximum, value));
}

void SynthEngine::setParameter(const std::string& id, float value) {
    for (std::size_t i = 0; i < kParameters.size(); ++i)
        if (id == kParameters[i].id) {
            m_values[i].store(clampParameter(i, value), std::memory_order_relaxed);
            return;
        }
}

float SynthEngine::parameter(const std::string& id) const {
    for (std::size_t i = 0; i < kParameters.size(); ++i)
        if (id == kParameters[i].id) return m_values[i].load(std::memory_order_relaxed);
    return 0.0f;
}

void SynthEngine::noteOn(int midiNote, float velocity) {
    m_noteFrequency = 440.0f * std::pow(2.0f, (static_cast<float>(midiNote) - 69.0f) / 12.0f);
    m_velocity = std::max(0.0f, std::min(1.0f, velocity));
    m_gate = true;
}

void SynthEngine::noteOff() { m_gate = false; }

float SynthEngine::nextSample() {
    const float attack = parameter("amp.attack");
    const float decay = parameter("amp.decay");
    const float release = parameter("amp.release");
    if (m_gate) {
        if (m_envelope < 1.0f)
            m_envelope = std::min(1.0f, m_envelope + 1.0f / static_cast<float>(attack * m_sampleRate));
        else
            m_envelope = std::max(0.75f, m_envelope - (1.0f - 0.75f) / static_cast<float>(decay * m_sampleRate));
    } else {
        m_envelope = std::max(0.0f, m_envelope - 1.0f / static_cast<float>(release * m_sampleRate));
    }

    const float frequency = std::min(parameter("osc.frequency"), static_cast<float>(m_sampleRate * 0.45));
    const double increment = std::max(0.0f, frequency) / m_sampleRate;
    m_phase += increment;
    if (m_phase >= 1.0) m_phase -= std::floor(m_phase);

    float osc = 0.0f;
    switch (static_cast<int>(parameter("osc.waveform") + 0.5f)) {
        case 1: osc = static_cast<float>(2.0 * m_phase - 1.0); break;
        case 2: osc = m_phase < 0.5 ? 1.0f : -1.0f; break;
        case 3: osc = static_cast<float>(1.0 - 4.0 * std::abs(m_phase - 0.5)); break;
        default: osc = static_cast<float>(std::sin(2.0 * kPi * m_phase)); break;
    }

    // One-pole low-pass filter. This is a starter DSP stage, not a resonant filter.
    const float cutoff = std::min(parameter("filter.cutoff"), static_cast<float>(m_sampleRate * 0.45));
    const float alpha = 1.0f - std::exp(-2.0f * static_cast<float>(kPi) * cutoff / static_cast<float>(m_sampleRate));
    m_filterState += alpha * (osc - m_filterState);
    return m_filterState * m_envelope * m_velocity * parameter("master.gain");
}

void SynthEngine::process(float* left, float* right, std::size_t numSamples) {
    if (!left || !right) return;
    for (std::size_t i = 0; i < numSamples; ++i) {
        const float s = nextSample();
        left[i] += s;
        right[i] += s;
    }
}
} // namespace synth
