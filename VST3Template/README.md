# VST3Template

Plugin build expects `GeneratedParameters.h` at `generated/GeneratedParameters.h`.
Generate it from a SynthEditor document with:

```bat
SynthCodeGen.exe instrument.dat VST3Template/generated/GeneratedParameters.h
```

Alternatively configure CMake with `-DGENERATED_PARAMETERS_HEADER=<absolute path>`.

The plugin registers the generated parameters as VST3 automatable parameters.
VST3 host values are normalized to 0..1; the processor converts them back to each
parameter's actual `min`/`max` range.

The template currently has no graphical `IPlugView` implementation and no integrated
DSP engine. VSTGUI is the intended route for an embedded VST3 editor; Qt is used by
the standalone SynthEditor application, not by the plugin.
