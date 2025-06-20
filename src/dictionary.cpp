#include "dictionary.h"
#include "notedetector.h"
#include "word.h"
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <sstream>
#include <algorithm> // For std::count_if
#include <cctype>    // For std::isalnum
#include <cmath> // For std::max and std::min

Dictionary::Dictionary(const fs::path& filePath) {
    load(filePath);
}

long count_notes(const std::string& str) {
    long unsigned int count = 0;
    for(const char c : str) {
        if(c >= 'A' && c <= 'G')
            count++;
    }
    return count;
}

bool Dictionary::load(const fs::path& filePath) {

    printf("Loading dict file: %s\n", filePath.string().c_str());
    std::ifstream file(filePath);
    if(!file.is_open()) {
        printf("Error loading dict file: %s\n", filePath.string().c_str());
        return false;
    }

        
    std::string line;
    while (std::getline(file, line)) {
        //printf("%s\n", line.c_str());

        auto pos = line.find(DICT_ENTRY_SEPARATOR);
        if(pos != std::string::npos) {
            auto wordRunes = std::string(line.substr(0, pos));
            auto wordTranslation = std::string(line.substr(pos+1));
            printf("[%s] = [%s]\n", wordRunes.c_str(), wordTranslation.c_str());

            std::vector<Rune> runes;
            Word word(wordRunes);

            if(word.size() > 0) {
				auto hash = word.get_hash();
                if(m_hashtable.contains(hash)) {
                    printf("WARNING : duplicate entry [%s]\n", hash.c_str());
                } 
                else {
                    m_hashtable[hash] = wordTranslation;
                    
                    //printf(" nbNotes = %lu\n", nbNotes);
                    notes_max_length = std::max(notes_max_length, word.size());
                    notes_min_length = std::min(notes_min_length, word.size());
                }
            }
        }

    }
    file.close();

    // printf("%s\n\nm_hashtable:\n", line.c_str());

    // for(const auto& [key, value] : m_hashtable) {
    //     printf("[%s] = [%s]\n", key.c_str(), value.c_str());
    // }

    printf("\n");
    return true;
}

bool Dictionary::save(const fs::path& filePath) {
    std::ofstream file(filePath);
    if(!file.is_open())
        return false;

    std::string line;
    for(auto const& [wordPhonetic, wordTranslation] : m_hashtable) {
        file << wordPhonetic << DICT_ENTRY_SEPARATOR << wordTranslation << std::endl;
    }
    file.close();

    return true;
}

bool Dictionary::add_word(const std::string& word, const std::string& translation)
{
    if(word.empty() || translation.empty()) {
        return false;
	}

    if(m_hashtable.contains(word)) {
        if(m_hashtable[word] == translation) {
            printf("INFO: word [%s] already exists in the dictionary with the same translation\n", word.c_str());
            return true;
		}
        printf("WARNING: word [%s] already exists in the dictionary and translation mismatch ! existing: [%s] new: [%s]\n", word.c_str(), m_hashtable[word], translation);
        return false;
	}
	m_hashtable[word] = translation;

    return true;
}

std::string Dictionary::translate(const std::string& str) {

    if(m_hashtable.contains(str))
        return m_hashtable[str];
    
    // word not found but added to be saved in file
    if(m_learning)
        m_hashtable[str] = str;

    return "";
}

std::string Dictionary::translate(const Word& word) {

	std::string hash = word.get_hash();
    if (m_hashtable.contains(hash))
        return m_hashtable[hash];

    // word not found but we can still translate rune by rune
	std::string str = word.to_pseudophonetic();

    if (m_learning)
        m_hashtable[hash] = str;

    return std::string(str);
}

std::string Dictionary::translate(const std::vector<Note>& notes) {
    std::stringstream ssout;
    std::deque<Note> noteBuffer;

    // std::cout << "--- START TRANSLATION ---" << std::endl;
    // std::cout << "  notes_min_length: " << notes_min_length << std::endl;
    // std::cout << "  notes_max_length: " << notes_max_length << std::endl;
    for(const auto& note : notes) {
        noteBuffer.push_back(note);
        if(noteBuffer.size() > notes_max_length) {
            noteBuffer.pop_front();
        }

        std::deque<Note> wordBuffer(noteBuffer);
        while(wordBuffer.size() >= notes_min_length ) {

            std::string str = notes_to_string(wordBuffer, true);
            // std::cout << "  try to translate: " <<  str << std::endl;
            std::string translated = translate(str);
            // std::cout << "  translated: " <<  translated << std::endl;

            if(translated.size() > 0) {
                std::cout << str << " => " <<  translated << std::endl;
                ssout << translated << " ";
                noteBuffer.clear();
                wordBuffer.clear();
                break; // while
            }
            wordBuffer.pop_front();
        }
    }

    return std::string(ssout.str());
}

std::string Dictionary::translate(const std::vector<Rune>& runes) {

    std::stringstream ss;
    for(const auto& rune : runes) {
        ss << rune.to_pseudophonetic();
    }

    return std::string(ss.str());
}

std::string Dictionary::translate(const std::vector<Word>& words) {

    std::stringstream ss;
	bool first = true;
    for (const auto& word : words) {
        if(!first) {
            ss << " ";
        } else {
            first = false;
		}
        ss << translate(word);
    }

    return std::string(ss.str());
}