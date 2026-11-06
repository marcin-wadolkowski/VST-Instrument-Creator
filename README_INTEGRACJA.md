# SynthEditor + SynthEngine — pakiet integracyjny

Pakiet bazuje na przesłanych projektach `SynthEditor_v03` i `VST3Template`.
Edytor zapisuje dokument JSON z rozszerzeniem `.dat`; generator rozumie zarówno
nowy format `elements`, jak i starszy format `widgets`.

## Zawartość
- `SynthEditor/` — edytor Qt 6 (projekt bazowy, bez przebudowy UI).
- `SynthEngine/` — biblioteka C++17 z prostym oscylatorem, obwiednią, filtrem dolnoprzepustowym i API noteOn/noteOff.
- `tools/` — generator nagłówka C++ z parametrów w pliku `.dat`.
- `VST3Template/` — przesłany szkielet wtyczki VST3.
- `PARAMETRY.md` — tabela parametrów.
- `BUILD.md` — instrukcja kompilacji.

## Uwaga o stanie
To szkielet integracyjny, nie gotowy komercyjny syntezator. `SynthEngine` generuje
dźwięk po `noteOn()`, a `noteOff()` uruchamia fazę wybrzmiewania. Filtr jest
jednobiegunowy (parametr rezonansu jest na razie tylko zarezerwowany). Kod generatora
tworzy tabelę parametrów z UI, ale nie generuje automatycznie pełnej implementacji DSP.
Przed wdrożeniem produkcyjnym należy dodać testy, wygładzanie parametrów i obsługę
wątków/automatyzacji zgodną z API hosta VST3.

## Integracja nagłówka z VST3Template

1. Zbuduj `SynthCodeGen` zgodnie z `BUILD.md`.
2. Wygeneruj nagłówek z dokumentu zapisanego w SynthEditor:
   `SynthCodeGen.exe instrument.dat VST3Template/generated/GeneratedParameters.h`
3. Skonfiguruj VST3Template. Jeśli nagłówek jest w innym miejscu, podaj
   `-DGENERATED_PARAMETERS_HEADER="pełna/ścieżka/GeneratedParameters.h"`.

`VST3Template` nie czyta już starego klucza JSON `widgets` w czasie uruchomienia.
Korzysta z wygenerowanej tabeli `kGeneratedParameters`; źródłem danych jest tablica
`elements` z SynthEditor. Po zmianie instrumentu trzeba ponownie uruchomić generator
i przebudować wtyczkę.

### GUI wtyczki

Edytor SynthEditor używa Qt 6, ale nie oznacza to, że sam plugin VST3 powinien linkować Qt.
GUI wtyczki VST3 powinno być osadzaną przez hosta kontrolką `IPlugView` (np. przez VSTGUI,
które jest już zależnością szablonu). Qt w pluginie może powodować konflikty wersji Qt,
platform pluginów i cyklu życia GUI w różnych DAW. Obecny szablon nie implementuje jeszcze
widoku `IPlugView` ani renderowania layoutu z `.dat`; samo linkowanie `vstgui_support`
nie tworzy interfejsu. To osobny etap integracji.
