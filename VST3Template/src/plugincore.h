#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "pluginterfaces/vst/vsttypes.h"

#include <cstdint>
#include <string>
#include <vector>

struct DynamicParameter
{
    Steinberg::Vst::ParamID id = 0;
    std::string name;
    float value = 0.0f;
    double minimum = 0.0;
    double maximum = 1.0;
};

class VST3TemplateProcessor : public Steinberg::Vst::AudioEffect
{
public:
    VST3TemplateProcessor();
    ~VST3TemplateProcessor() override = default;

    static Steinberg::FUnknown* createInstance(void* context);

    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API setupProcessing(
        Steinberg::Vst::ProcessSetup& newSetup) override;
    Steinberg::tresult PLUGIN_API process(
        Steinberg::Vst::ProcessData& data) override;

private:
    float sampleRate = 44100.0f;
    std::vector<DynamicParameter> activeParameters;
};
