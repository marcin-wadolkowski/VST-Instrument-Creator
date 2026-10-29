# Budowanie

## 1. SynthEditor (Qt 6)
Wymagania: CMake 3.21+, kompilator C++17 i Qt 6 Widgets.

```bash
cmake -S SynthEditor -B build-editor
cmake --build build-editor --config Release
```

Jeśli Qt nie jest w standardowej ścieżce, skonfiguruj `CMAKE_PREFIX_PATH` na katalog instalacji Qt.

## 2. Biblioteka SynthEngine (bez Qt i VST3 SDK)
```bash
cmake -S SynthEngine -B build-engine
cmake --build build-engine --config Release
```
Wynikiem jest biblioteka statyczna. Dołącz nagłówek `SynthEngine/include/SynthEngine.h`
i połącz aplikację z biblioteką `SynthEngine`.

## 3. Generator kodu C++
Wymagania: CMake 3.21+ oraz pakiet `nlohmann-json` (np. przez vcpkg).

Windows z vcpkg:
```bat
cmake -S tools -B build-generator -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build build-generator --config Release
```

Uruchomienie:
```bat
build-generator\Release\SynthCodeGen.exe ścieżka\do\instrument.dat GeneratedParameters.h
```
Generator czyta `elements` (SynthEditor) lub `widgets` (starszy format VST3Template).
Duplikaty ID są pomijane, a niepoprawny zakres zatrzymuje generowanie.

## 4. VST3Template
Otwórz `VST3Template/CMakeLists.txt` w środowisku z VST3 SDK/vcpkg i sprawdź konfigurację
zależności w `vcpkg.json`. Następnie skonfiguruj i buduj zgodnie z konfiguracją CMake
własnego środowiska. Dołączanie/instalowanie gotowej wtyczki w DAW zależy od hosta.

**Ważne:** `build.bat` z oryginalnego szkieletu wskazuje na konkretną wersję CMake i generator
Visual Studio. Jeśli nie masz dokładnie tych ścieżek, zmień je na ścieżki z własnej instalacji.
Samo ustawienie rozszerzenia `.vst3` nie wystarcza do poprawnej paczki VST3 na wszystkich
systemach — wtyczka wymaga prawidłowej fabryki, metadanych i struktury pakietu SDK.
