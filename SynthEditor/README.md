# SynthEditor 0.4 — edytor layoutów instrumentów

Prototyp Qt 6 Widgets / C++17.

## Funkcje
1. Zaznaczanie elementu oraz zmiana rozmiaru uchwytem w prawym dolnym rogu.
2. Przeciąganie pokręteł, suwaków i przycisków z biblioteki na obszar roboczy.
3. Zmiana koloru akcentu, koloru tekstu, rozmiaru czcionki, koloru tła i obrazu tła.
4. Pole `parameter` jest zapisywane jako identyfikator parametru silnika.
5. Walidacja dokumentu przed zapisem i po otwarciu: wersja formatu, ID, nazwy parametrów, zakresy, kolory, rozmiary i położenie elementów.

## Wymagania
- Qt 6 z modułem Widgets
- CMake 3.21+
- Kompilator C++17 (np. MSVC na Windows)

## Uruchomienie
Otwórz `CMakeLists.txt` w Qt Creator, wybierz zestaw Qt 6 i skompiluj.

Alternatywnie:
```bash
cmake -S . -B build
cmake --build build --config Release
```

## Format `.dat`
Rozszerzenie `.dat` zawiera JSON. Format ma `formatVersion: 1`.
Ścieżka obrazu tła jest obecnie zapisywana jako ścieżka lokalna; przed dystrybucją instrumentu warto dodać pakowanie zasobów i ścieżki względne.

## Uwagi
To prototyp źródłowy. Nie został skompilowany w Twoim lokalnym środowisku; po uruchomieniu mogą być potrzebne drobne poprawki zależne od wersji Qt i kompilatora. Nie implementuje jeszcze syntezy ani VST3.


## v0.3
- Poprawiono zmianę rozmiaru elementów: rozmiar jest liczony od wymiarów z chwili rozpoczęcia przeciągania, dzięki czemu nie narasta chaotycznie przy kolejnych ruchach myszy.


## Pakiet integracyjny
Zobacz `../README_INTEGRACJA.md`, `../PARAMETRY.md` i `../BUILD.md`. Generator C++ znajduje się w `../tools/` a biblioteka DSP w `../SynthEngine/`.
