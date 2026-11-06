#include "plugincore.h"
#include "GeneratedParameters.h"
#include "base/source/fstring.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace {
inline float denormalize(ParamValue value, double minimum, double maximum)
{
    const double normalized = std::clamp(value, 0.0, 1.0);
    return static_cast<float>(minimum + normalized * (maximum - minimum));
}

inline ParamValue normalize(double value, double minimum, double maximum)
{
    if (maximum <= minimum) return 0.0;
    return std::clamp((value - minimum) / (maximum - minimum), 0.0, 1.0);
}
}

VST3TemplateProcessor::VST3TemplateProcessor()
{
    setControllerClass(FUID(0x11111111, 0x22222222, 0x33333333, 0x44444444));
}

FUnknown* VST3TemplateProcessor::createInstance(void*)
{
    return static_cast<IAudioProcessor*>(new VST3TemplateProcessor());
}

tresult PLUGIN_API VST3TemplateProcessor::initialize(FUnknown* context)
{
    const tresult result = AudioEffect::initialize(context);
    if (result != kResultOk)
        return result;

    addAudioOutput(STR16("Audio Output"), SpeakerArr::kStereo);

    // Single source of truth: this table is generated from the SynthEditor .dat file.
    activeParameters.clear();
    activeParameters.reserve(kGeneratedParameters.size());

    ParamID id = 0;
    for (const auto& generated : kGeneratedParameters) {
        const double minimum = generated.minimum;
        const double maximum = generated.maximum;
        const double defaultValue = std::clamp(generated.defaultValue, minimum, maximum);

        activeParameters.push_back({
            id, std::string(generated.id), static_cast<float>(defaultValue),
            minimum, maximum
        });

        const std::string parameterName(generated.name);
        String128 title;
        UString(title, 128).fromAscii(parameterName.c_str());
        parameters.addParameter(
            title,
            nullptr,
            0,
            normalize(defaultValue, minimum, maximum),
            ParameterInfo::kCanAutomate,
            id
        );
        ++id;
    }

    return kResultOk;
}

tresult PLUGIN_API VST3TemplateProcessor::setupProcessing(ProcessSetup& newSetup)
{
    sampleRate = static_cast<float>(newSetup.sampleRate);
    return AudioEffect::setupProcessing(newSetup);
}

tresult PLUGIN_API VST3TemplateProcessor::process(ProcessData& data)
{
    // VST3 automation values are normalized to [0, 1]. Convert them back
    // to the real ranges declared in GeneratedParameters.h.
    if (data.inputParameterChanges) {
        const int32 count = data.inputParameterChanges->getParameterCount();
        for (int32 i = 0; i < count; ++i) {
            IParamValueQueue* queue = data.inputParameterChanges->getParameterData(i);
            if (!queue || queue->getPointCount() == 0)
                continue;

            ParamValue normalizedValue = 0.0;
            int32 sampleOffset = 0;
            if (queue->getPoint(queue->getPointCount() - 1,
                                sampleOffset, normalizedValue) != kResultOk)
                continue;

            const ParamID paramId = queue->getParameterId();
            if (paramId < activeParameters.size()) {
                auto& parameter = activeParameters[paramId];
                parameter.value = denormalize(
                    normalizedValue, parameter.minimum, parameter.maximum);
            }
        }
    }

    if (data.numOutputs <= 0 || !data.outputs)
        return kResultOk;

    for (int32 channel = 0; channel < data.outputs[0].numChannels; ++channel) {
        if (data.outputs[0].channelBuffers32[channel])
            std::fill_n(data.outputs[0].channelBuffers32[channel],
                        data.numSamples, 0.0f);
    }

    // Temporary signal path: keep the template silent until a DSP engine is
    // connected. Generated parameters are now correctly exposed to the host.
    return kResultOk;
}
