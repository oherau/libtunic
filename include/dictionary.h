#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include <map>
#include <vector>
#include <string>
#include <limits>
#include <filesystem>
#include "note.h"
#include "rune.h"
#include "word.h"
namespace fs = std::filesystem;

const auto DICT_ENTRY_SEPARATOR = '=';
const auto DICT_RUNE_SEPARATOR = '-';
const auto DICT_COMMENT = '#';

class Dictionary {
public:
    Dictionary() = default;
    Dictionary(const fs::path& filePath);
    bool load(const fs::path& filePath);
    bool save(const fs::path& filePath);
	bool add_word(const std::string& word, const std::string& translation);
    std::string translate(const std::string& str);
    std::string translate(const std::vector<Note>& notes);
    std::string translate(const std::vector<Rune>& runes);
    std::string translate(const Word& word);
    std::string translate(const std::vector<Word>& words);
private:
    bool m_learning = false;
    std::map<std::string, std::string> m_hashtable;
    size_t notes_min_length = std::numeric_limits<size_t>::max();
    size_t notes_max_length = 0;
};

#endif // __DICTIONARY_H__