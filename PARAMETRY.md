# Tabela parametrów SynthEngine

Wartości numeryczne są wartościami rzeczywistymi; `Waveform` to wartość całkowita zapisana jako float.

| ID | Nazwa | Minimum | Maksimum | Domyślnie | Status |
|---|---|---:|---:|---:|---|
| `master.gain` | Master Gain | 0.0 | 1.0 | 0.70 | aktywny |
| `osc.frequency` | Frequency | 20 Hz | 20000 Hz | 440 Hz | aktywny; używany bez nuty MIDI |
| `osc.waveform` | Waveform | 0 | 3 | 0 | aktywny: 0 sine, 1 saw, 2 square, 3 triangle |
| `filter.cutoff` | Filter Cutoff | 20 Hz | 20000 Hz | 12000 Hz | aktywny; filtr LP jednobiegunowy |
| `filter.resonance` | Filter Resonance | 0.1 | 10.0 | 0.707 | zarezerwowany, nie wpływa jeszcze na DSP |
| `amp.attack` | Attack | 0.001 s | 5 s | 0.01 s | aktywny |
| `amp.decay` | Decay | 0.001 s | 5 s | 0.15 s | uproszczony etap opadania do poziomu sustain 0.75 |
| `amp.release` | Release | 0.005 s | 10 s | 0.25 s | aktywny |

## Konwencja dla parametrów UI
Pole `parameter` elementu SynthEditor powinno używać stabilnego identyfikatora, np.
`filter.cutoff`. Zakresy `min`, `max` i `value` zapisane w dokumencie opisują kontrolkę.
Aby sterować silnikiem, identyfikator musi odpowiadać ID rozpoznawanemu przez `SynthEngine::setParameter`.
Nieznane ID są ignorowane.
