#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include <map>
#include <vector>
#include <string>
#include <limits>
#include <filesystem>
#include <tinyxml2/tinyxml2.h>

namespace fs = std::filesystem;


#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <filesystem> // C++17 for file path handling

// Define structures to hold dictionary data
struct GrammaticalGroup {
    std::string partOfSpeech; // e.g., "noun", "verb"
    // Add more if your TEI includes gender, number, etc.
};

struct Example {
    std::string text;
    // Potentially add more fields like source, translation, etc.
};

struct Cit {
    std::string quote;
    // Potentially add more fields like source, translation, etc.
};

class Sense {
public:
    int number;
    GrammaticalGroup gramGrp;
    std::string definition;
    std::vector<Example> examples; // An entry can have multiple examples per sense
    std::vector<Cit> cits;

    Sense() : number(0) {} // Default constructor

    std::string to_string() const {
        std::stringstream ss;
        bool first = true;
        for (const auto& cit : cits) {
            if (first) { first = false; }
            else { ss << ","; }
            ss << cit.quote;
        }
        return ss.str();
    }
};

class DictionaryEntry {
public:
    void print() const;
    std::string id;        // xml:id attribute
    std::string lemma;     // <orth> content
    std::vector<Sense> senses; // An entry can have multiple senses
    std::string to_string() const {
        std::stringstream ss;
        bool first = true;
        for (const auto& sense : senses) {
            if (first) { first = false; }
            else { ss << "/"; }
            ss << sense.to_string();
        }
        return ss.str();
    }
};



class Dictionary {
public:
    Dictionary() = default;
    Dictionary(const fs::path& filePath);
    std::optional<DictionaryEntry> find(const std::string& searchTerm);
private:
    fs::path m_filepath;
};

#endif // __DICTIONARY_H__