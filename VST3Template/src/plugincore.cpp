#include "plugincore.h"
#include <nlohmann/json.hpp> // Biblioteka nlohmann/json
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include <fstream>

#if NTDDI_VERSION
#include <windows.h>
#endif

using namespace Steinberg;
using namespace Steinberg::Vst;

// Helper pobierający ścieżkę do samego siebie w pamięci (kompatybilny z Windows)
std::filesystem::path VST3TemplateProcessor::resolveDatFilePath() {
#if defined(_WIN32) || defined(__WIN32__)
    wchar_t path[MAX_PATH];
    HMODULE hm = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
                       GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&createInstance, &hm);
    
    GetModuleFileNameW(hm, path, MAX_PATH);
    std::filesystem::path dllPath(path);
    
    // Bezpiecznik: najpierw szukamy w x86_64-win/, jeśli brak, to obok folderu .vst3
    std::filesystem::path internalDat = dllPath.replace_extension(".dat");
    if (std::filesystem::exists(internalDat)) {
        return internalDat;
    }
    
    // Alternatywna ścieżka: wyjście poza Contents/x86_64-win/ i szukanie w katalogu Generated/
    return dllPath.parent_path().parent_path().parent_path().replace_extension(".dat");
#else
    // Domyślny fallback dla innych systemów operacyjnych
    return std::filesystem::current_path() / "plugin.dat";
#endif
}

VST3TemplateProcessor::VST3TemplateProcessor() {
    // Rejestracja klasy kontrolera i procesora w jednym (Single-Component Model)
    setControllerClass(FUID(0x11111111, 0x22222222, 0x33333333, 0x44444444));
}

FUnknown* VST3TemplateProcessor::createInstance(void* context) {
    return (IAudioProcessor*)new VST3TemplateProcessor();
}

// dynamiczne wczytywanie i parsowanie pliku .dat zapisanego przez SynthEditor
void VST3TemplateProcessor::loadAndParseLayout() {
    if (isLayoutLoaded) return;

    std::filesystem::path datPath = resolveDatFilePath();
    std::ifstream file(datPath);
    if (!file.is_open()) return;

    try {
        nlohmann::json jsonDoc;
        file >> jsonDoc;

        if (jsonDoc.contains("widgets") && jsonDoc["widgets"].is_array()) {
            int currentId = 0;
            for (const auto& widget : jsonDoc["widgets"]) {
                if (widget.contains("parameter")) {
                    std::string paramName = widget["parameter"].get<std::string>();
                    if (!paramName.empty()) {
                        // Rejestrujemy parametr w wewnętrznym wektorze silnika
                        activeParameters.push_back({currentId, paramName, 0.5f});
                        
                        // Automatyczne dodanie parametru do VST3 SDK
                        // Dzięki temu DAW (np. FL Studio) od razu zobaczy ten suwak i pozwoli na automatyzację
                        parameters.addParameter(
                            STR16(paramName.c_str()), 
                            nullptr, 0, 0.5f, 
                            ParameterInfo::kCanAutomate, 
                            currentId
                        );
                        currentId++;
                    }
                }
            }
        }
        isLayoutLoaded = true;
    } catch (...) {
        // Obsługa błędów formatu pliku .dat
        isLayoutLoaded = false;
    }
}

tresult PLUGIN_API VST3TemplateProcessor::initialize(FUnknown* context) {
    tresult result = AudioEffect::initialize(context);
    if (result == kResultTrue) {
        // Konfiguracja wyjścia stereo wtyczki
        addAudioOutput(STR16("Audio Output"), SpeakerArr::kStereo);
        
        // Krytyczne: Wczytanie pliku .dat i automatyczne utworzenie suwaków
        loadAndParseLayout();
    }
    return result;
}

tresult PLUGIN_API VST3TemplateProcessor::setupProcessing(ProcessSetup& newSetup) {
    sampleRate = newSetup.sampleRate;
    return AudioEffect::setupProcessing(newSetup);
}

// Główna pętla przetwarzania Audio DSP
tresult PLUGIN_API VST3TemplateProcessor::process(ProcessData& data) {
    // 1. Odczytanie ruchów suwaków wysłanych z DAW (Automatyzacja, Myszka)
    if (data.inputParameterChanges) {
        int32 numActualChanges = data.inputParameterChanges->getParameterCount();
        for (int32 i = 0; i < numActualChanges; ++i) {
            if (auto* queue = data.inputParameterChanges->getParameterData(i)) {
                int32 numPoints = queue->getPointCount();
                ParamValue value;
                int32 sampleOffset;
                if (queue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                    int32 paramId = queue->getParameterId();
                    if (paramId < activeParameters.size()) {
                        // Aktualizacja wartości parametru w czasie rzeczywistym
                        activeParameters[paramId].value = static_cast<float>(value);
                    }
                }
            }
        }
    }

    // 2. Przetwarzanie i synteza próbek audio
    int32 numSamples = data.numSamples;
    float** outChannels = data.outputs.channelBuffers32;

    if (data.numOutputs > 0 && outChannels) {
        float* leftChannel = outChannels[0];
        float* rightChannel = outChannels[1];

        for (int32 sample = 0; sample < numSamples; ++sample) {
            float signal = 0.0f;

            // --- PROSTY GENERATOR SZUMU / BYPASS MODULOWANY PIERWSZYM PARAMETREM ---
            // W kolejnych etapach rozbudujemy to o graf połączeń elementów syntezatora
            if (!activeParameters.empty()) {
                float gain = activeParameters[0].value; // Pobranie wartości z suwaka 1
                signal = ((float)rand() / RAND_MAX) * 0.1f * gain;
            }

            leftChannel[sample] = signal;
            rightChannel[sample] = signal;
        }
    }

    return kResultTrue;
}
