#include "runedictionary.h"
#include "arpeggiodetector.h"
#include "word.h"
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <sstream>
#include <algorithm> // For std::count_if
#include <cctype>    // For std::isalnum
#include <cmath> // For std::max and std::min
#include <opencv2/opencv.hpp>
#include "dictionary.h"

RuneDictionary::RuneDictionary(const fs::path& filePath) {
    load(filePath);
}

long count_notes(const std::string& str) {
    long unsigned int count = 0;
    for (const char c : str) {
        if (c >= 'A' && c <= 'G')
            count++;
    }
    return count;
}

bool RuneDictionary::load(const fs::path& filePath) {

    printf("Loading dict file: %s\n", filePath.string().c_str());
    std::ifstream file(filePath);
    if (!file.is_open()) {
        printf("Error loading dict file: %s\n", filePath.string().c_str());
        return false;
    }


    std::string line;
    while (std::getline(file, line)) {
        //printf("%s\n", line.c_str());

        auto pos = line.find(DICT_ENTRY_SEPARATOR);
        if (pos != std::string::npos) {
            auto wordRunes = std::string(line.substr(0, pos));
            auto wordTranslation = std::string(line.substr(pos + 1));
            printf("[%s] = [%s]\n", wordRunes.c_str(), wordTranslation.c_str());

            std::vector<Rune> runes;
            Word word(wordRunes);

            if (word.size() > 0) {
                auto hash = word.get_hash();
                if (m_hashtable.contains(hash)) {
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

    // sort elements for detection
    // 1. Copy map elements to a vector of pairs
    m_ordered_entries = std::vector<std::pair<std::string, std::string>>(m_hashtable.begin(), m_hashtable.end());

    // 2. Sort the vector using a custom lambda comparator
    std::sort(m_ordered_entries.begin(), m_ordered_entries.end(),
        [](const std::pair<std::string, std::string>& a,
            const std::pair<std::string, std::string>& b) {
                // Primary sort criterion: length of the key (a.first)
                if (a.first.length() != b.first.length()) {
                    return a.first.length() > b.first.length();
                }
                // Secondary sort criterion: lexicographical order of the value (a.second)
                return a.second < b.second;
        });

    std::cout << "Vector sorted by key length, then by value lexicographically:\n";
    for (const auto& pair : m_ordered_entries) {
        std::cout << "  Key: \"" << pair.first << "\", Value: \"" << pair.second << "\"\n";
    }
    std::cout << "\n";

    printf("\n");
    return true;
}

bool RuneDictionary::save(const fs::path& filePath) {
    std::ofstream file(filePath);
    if (!file.is_open())
        return false;

    //std::string line;
    //for(auto const& [wordPhonetic, wordTranslation] : m_hashtable) {
    //    file << wordPhonetic << DICT_ENTRY_SEPARATOR << wordTranslation << std::endl;
    //}
    //file.close();

    // 1. Copy map elements to a vector of pairs
    std::vector<std::pair<std::string, std::string>> vec(m_hashtable.begin(), m_hashtable.end());

    // 2. Sort the vector by values alphanumerically (case-insensitive)
    std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b) {
        // Compare values case-insensitively
        std::string s1 = a.second;
        std::string s2 = b.second;

        // Using std::lexicographical_compare and std::tolower for robust case-insensitive comparison
        return std::lexicographical_compare(s1.begin(), s1.end(),
            s2.begin(), s2.end(),
            [](char c1, char c2) {
                return std::tolower(static_cast<unsigned char>(c1)) <
                    std::tolower(static_cast<unsigned char>(c2));
            });
        });

    // 3. Display sorted elements with headers
    char currentHeaderChar = '\0'; // Initialize with a character that won't match A-Z

    for (const auto& pair : vec) {
        if (!pair.second.empty()) { // Ensure the string is not empty
            char firstChar = std::toupper(static_cast<unsigned char>(pair.second[0]));

            if (firstChar != currentHeaderChar) {
                // New group, print header

                file << "###############################" << std::endl
                    << "######         " << firstChar << "         ######" << std::endl
                    << "###############################" << std::endl;
                currentHeaderChar = firstChar;
            }
        }

        file << pair.first << DICT_ENTRY_SEPARATOR << pair.second << std::endl;
    }

    std::cout << "\n--- End of List ---" << std::endl;
    return true;
}

bool RuneDictionary::has_hash(const std::string& word_hash) const
{
    return m_hashtable.find(word_hash) != m_hashtable.end();
}

bool RuneDictionary::add_word(const std::string& word_hash, const std::string& translation)
{
    if (word_hash.empty() || translation.empty()) {
        return false;
    }

    if (m_hashtable.contains(word_hash)) {
        if (m_hashtable[word_hash] == translation) {
            printf("INFO: word [%s] already exists in the dictionary with the same translation\n", word_hash.c_str());
            return true;
        }
        printf("WARNING: word [%s] already exists in the dictionary and translation mismatch ! existing: [%s] new: [%s]\n", word_hash.c_str(), m_hashtable[word_hash].c_str(), translation.c_str());
        return false;
    }
    m_hashtable[word_hash] = translation;

    return true;
}

bool RuneDictionary::get_hash_list(std::vector<std::string>& hash_list) const
{
    //for (const auto& [key, value] : m_hashtable) {
    //    hash_list.push_back(key);
    //}
    // get keys from ordered list
    for (const auto& [key, value] : m_ordered_entries) {
        hash_list.push_back(key);
    }
    return true;
}

bool RuneDictionary::get_value_list(std::vector<std::string>& value_list) const
{
    for (const auto& [key, value] : m_hashtable) {
        value_list.push_back(value);
    }
    return true;
}

bool RuneDictionary::get_translation(const std::string& word, std::string& translation) const
{
    if (m_hashtable.contains(word)) {
        translation = m_hashtable.at(word);
        return true;
    }
    return false;
}

std::string RuneDictionary::translate(const std::string& str) {

    if (m_hashtable.contains(str))
        return m_hashtable[str];

    // word not found but added to be saved in file
    if (m_learning)
        m_hashtable[str] = str;

    return "";
}

std::string RuneDictionary::translate(const Word& word) {

    std::string hash = word.get_hash();
    if (m_hashtable.contains(hash))
        return m_hashtable[hash];

    // word not found but we can still translate rune by rune
    std::string str = word.to_pseudophonetic();

    if (m_learning)
        m_hashtable[hash] = str;

    return std::string(str);
}

std::string RuneDictionary::translate(const std::vector<Rune>& runes) {

    std::stringstream ss;
    for (const auto& rune : runes) {
        ss << rune.to_pseudophonetic();
    }

    return std::string(ss.str());
}

std::string RuneDictionary::translate(const std::vector<Word>& words) {

    std::stringstream ss;
    bool first = true;
    for (const auto& word : words) {
        if (!first) {
            ss << " ";
        }
        else {
            first = false;
        }
        ss << translate(word);
    }

    return std::string(ss.str());
}

bool RuneDictionary::generate_images(const fs::path& image_dir, std::string extension) const
{
    if (image_dir.empty() || !fs::exists(image_dir) || !fs::is_directory(image_dir)) {
        std::cerr << "Error: Invalid image directory path." << std::endl;
        return false;
    }

    for (const auto& [word_hash, word_str] : m_hashtable) {
        Word word(word_hash);

        cv::Mat image;
        auto rune_size = RUNE_DEFAULT_SIZE;
        auto tickness = RUNE_SEGMENT_DRAW_DEFAULT_TICKNESS * rune_size.height;
        word.generate_image(rune_size, tickness, image);

        // for debugging purposes, display the image
        //cv::imshow(word_str + " " + word_hash, result);
        //cv::waitKey(1000); // Wait for a key press to close the window
        //cv::destroyAllWindows();

        std::string filename = word_hash + "_" + word_str + extension;
        fs::path filepath = image_dir / filename;

        bool success = false;
        if (extension == ".png") {
            success = cv::imwrite(filepath.string(), image);
        }
        else if (extension == ".jpg" || extension == ".jpeg") {
            std::vector<int> compression_params;
            compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
            compression_params.push_back(95); // Set quality to 95 (out of 100)
            success = cv::imwrite(filepath.string(), image, compression_params);
        }
        else {
            std::cerr << "Error: Unsupported image format. Use .png or .jpg." << std::endl;
            return false;
        }

        if (success) {
            std::cout << "Image saved successfully as: " << filepath << std::endl;
        }
        else {
            std::cerr << "Error: Could not save image as " << filepath << std::endl;
        }
    }

    return true;
}
