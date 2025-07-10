#ifndef __RUNEDICTIONARY_H__
#define __RUNEDICTIONARY_H__

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
const auto DICT_RUNE_SEPARATOR_2 = ' ';
const auto DICT_COMMENT = '#';

class RuneDictionary {
public:
    RuneDictionary() = default;
    RuneDictionary(const fs::path& filePath);
    bool load(const fs::path& filePath);
    bool save(const fs::path& filePath);
    bool has_hash(const std::string& hash) const;
    bool add_word(const std::string& word, const std::string& translation);
    bool get_hash_list(std::vector<std::string>& hash_list) const;
    bool get_value_list(std::vector<std::string>& value_list) const;
    bool get_translation(const std::string& word, std::string& translation) const;
    std::string translate(const std::string& str);
    std::string translate(const std::vector<Note>& notes);
    std::string translate(const std::vector<Rune>& runes);
    std::string translate(const Word& word);
    std::string translate(const std::vector<Word>& words);
    bool generate_images(const fs::path& image_dir, std::string extension = ".png") const;
    std::string& operator[](std::string key) {
        return m_hashtable[key];
    }

private:
    bool m_learning = false;
    std::map<std::string, std::string> m_hashtable;
    std::vector<std::pair<std::string, std::string>> m_ordered_entries;
    size_t notes_min_length = std::numeric_limits<size_t>::max();
    size_t notes_max_length = 0;
};

#endif // __RUNEDICTIONARY_H__