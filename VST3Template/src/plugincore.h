#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "pluginterfaces/vst/vsttypes.h"
#include <vector>
#include <string>
#include <filesystem>

// Struktura mapująca dynamiczne parametry z pliku .dat
struct DynamicParameter {
    int32 id;
    std::string name;
    float value;
};

class VST3TemplateProcessor : public Steinberg::Vst::AudioEffect
{
public:
    VST3TemplateProcessor();
    ~VST3TemplateProcessor() override = default;

    // Metody fabrykujące Steinberg VST3
    static Steinberg::FUnknown* createInstance(void* context);
    
    // Cykl życia wtyczki i przetwarzanie sygnału
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup& newSetup) override;
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) override;

private:
    // Helper do lokalizowania pliku .dat na dysku na podstawie nazwy binarnej
    std::filesystem::path resolveDatFilePath();
    void loadAndParseLayout();

    float sampleRate = 44100.0f;
    std::vector<DynamicParameter> activeParameters;
    bool isLayoutLoaded = false;
};
