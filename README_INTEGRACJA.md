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
