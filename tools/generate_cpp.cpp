#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>
#include <algorithm>
#include <string>

using json = nlohmann::json;

// Usage: SynthCodeGen instrument.dat GeneratedInstrument.h
// Accepts SynthEditor format ("elements") and legacy VST3Template format ("widgets").
int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: SynthCodeGen <instrument.dat> <output.h>\n";
        return 2;
    }
    std::ifstream in(argv[1]);
    if (!in) { std::cerr << "Cannot open input: " << argv[1] << "\n"; return 1; }
    json doc;
    try { in >> doc; } catch (const std::exception& e) {
        std::cerr << "Invalid JSON: " << e.what() << "\n"; return 1;
    }

    const json* items = nullptr;
    if (doc.contains("elements") && doc["elements"].is_array()) items = &doc["elements"];
    else if (doc.contains("widgets") && doc["widgets"].is_array()) items = &doc["widgets"];
    if (!items) { std::cerr << "Expected an 'elements' or 'widgets' array.\n"; return 1; }

    struct Param { std::string id; double min, max, value; };
    std::vector<Param> params;
    std::set<std::string> seen;
    for (const auto& item : *items) {
        if (!item.is_object() || !item.contains("parameter") || !item["parameter"].is_string()) continue;
        const std::string id = item["parameter"].get<std::string>();
        if (id.empty() || !seen.insert(id).second) continue;
        const double lo = item.value("min", 0.0);
        const double hi = item.value("max", 1.0);
        const double val = item.value("value", 0.5);
        if (!(lo < hi)) { std::cerr << "Invalid range for parameter: " << id << "\n"; return 1; }
        params.push_back({id, lo, hi, std::max(lo, std::min(hi, val))});
    }

    std::ofstream out(argv[2]);
    if (!out) { std::cerr << "Cannot write output: " << argv[2] << "\n"; return 1; }
    out << "#pragma once\n#include <array>\n#include <string_view>\n\n"
           "struct GeneratedParameter { std::string_view id; double minimum; double maximum; double defaultValue; };\n"
        << "inline constexpr std::array<GeneratedParameter, " << params.size()
        << "> kGeneratedParameters{{\n";
    for (const auto& p : params)
        out << "    {\"" << p.id << "\", " << p.min << ", " << p.max << ", " << p.value << "},\n";
    out << "}};\n";
    std::cout << "Generated " << params.size() << " parameter(s) in " << argv[2] << "\n";
    return 0;
}
