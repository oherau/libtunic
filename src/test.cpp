#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <queue>
#include <filesystem>
#include <string>
#include "dictionary.h"
#include "arpeggiodetector.h"
#include "rune.h"
#include "runedetector.h"
#include "word.h"
#include "color_print.h"
#include "note.h"
#include "yin.h"
#include <cstdint>


///////////////////////////////////////////////////////
//  TOOLS
///////////////////////////////////////////////////////

void PRINT_TEST_HEADER(const std::string& title) {
    const std::string sep = std::string(80, '=');
    int offset = (sep.size() - std::string(title).size()) / 2;
    std::cout << std::endl << std::endl << sep << std::endl << std::string(offset, ' ') << title << std::endl << sep << std::endl << std::endl;
}

int get_index_sequence_diff(const std::vector<int>& s1, const std::vector<int>& s2) {

    int result = -1;
    for (int i = 0; i < s1.size() && i < s2.size(); i++) {
        result += std::abs(s1[i] - s2[i]);
    }
    return result;
}

// Platform-specific headers
#ifdef _WIN32
#include <windows.h> // For GetModuleFileNameW
#elif defined(__linux__)
#include <unistd.h>  // For readlink
#include <limits.h>  // For PATH_MAX
#elif defined(__APPLE__)
#include <mach-o/dyld.h> // For _NSGetExecutablePath
#include <limits.h>  // For PATH_MAX
#endif
#include <toolbox.h>
#include <wavgenerator.h>
#include "toolbox.cpp"

namespace fs = std::filesystem;

fs::path getExecutablePath() {
#ifdef _WIN32
    // On Windows, use GetModuleFileNameW for wide characters
    wchar_t path[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (length == 0 || length == MAX_PATH) {
        // Handle error or buffer too small
        return fs::path(); // Return an empty path
    }
    return fs::path(path);
#elif defined(__linux__)
    // On Linux, use /proc/self/exe symlink
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count == -1) {
        // Handle error
        return fs::path(); // Return an empty path
    }
    return fs::path(std::string(path, count));
#elif defined(__APPLE__)
    // On macOS, use _NSGetExecutablePath
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) {
        // Handle buffer too small or other error
        // You might need to reallocate and try again if _NSGetExecutablePath returns non-zero
        return fs::path(); // Return an empty path
    }
    return fs::path(path);
#else
    // Fallback for other/unknown platforms or if you don't need this functionality everywhere
    std::cerr << "Warning: getExecutablePath() not implemented for this platform." << std::endl;
    return fs::current_path(); // Return current working directory as a fallback
#endif
}

void deleteFilesInFolder(const fs::path& folderPath) {
    if (!fs::exists(folderPath)) {
        std::cerr << "Error: Folder '" << folderPath << "' does not exist." << std::endl;
        return;
    }

    if (!fs::is_directory(folderPath)) {
        std::cerr << "Error: Path '" << folderPath << "' is not a directory." << std::endl;
        return;
    }

    std::cout << "Attempting to delete files in: " << folderPath << std::endl;

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        try {
            if (fs::is_regular_file(entry.status())) { // Check if it's a regular file
                fs::remove(entry.path());
                std::cout << "Deleted: " << entry.path().filename() << std::endl;
            }
            else if (fs::is_directory(entry.status())) {
                std::cout << "Skipping directory: " << entry.path().filename() << std::endl;
            }
            else {
                std::cout << "Skipping non-file/non-directory item: " << entry.path().filename() << std::endl;
            }
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "Error deleting " << entry.path().filename() << ": " << e.what() << std::endl;
        }
    }

    std::cout << "Finished attempting to delete files in: " << folderPath << std::endl;
}

///////////////////////////////////////////////////////
//  CONSTANTS
///////////////////////////////////////////////////////

const auto RUNES_FOLDER = fs::path("../../../data/runes");
const auto DICTIONARY_EN = fs::path("../../../lang/dictionary.txt");


///////////////////////////////////////////////////////
//  TEST CASES
///////////////////////////////////////////////////////

TEST_CASE("unify_rune_hash", "[translate]") {

    PRINT_TEST_HEADER("unify_rune_hash");

    Dictionary dictionary(DICTIONARY_EN);
    RuneDetector rune_detector(&dictionary);
    rune_detector.load_rune_folder(RUNES_FOLDER);



    const std::vector< std::pair<std::string, std::string> > test_set_001 = {
        { "9317" , "9317"},
        { "ABCD" , "abcd"},
        { "abcd" , "abcd"},
    };

    for (const auto& test : test_set_001) {
        auto rune_str_original = test.first;
        auto rune_str_expected = test.second;
        Rune r;
        r.from_hexa(rune_str_original);
        std::cout << "Rune: " << r.to_pseudophonetic() << std::endl;
        auto rune_str_back = r.to_hexa();
        std::cout << "Rune: " << rune_str_back << std::endl;

        CHECK(rune_str_expected == rune_str_back);

    }
}

TEST_CASE("unify_word_hash", "[translate]") {

    PRINT_TEST_HEADER("unify_word_hash");

    Dictionary dictionary(DICTIONARY_EN);
    RuneDetector rune_detector(&dictionary);
    rune_detector.load_rune_folder(RUNES_FOLDER);

    const std::vector< std::pair<std::string, std::string> > test_set_001 = {
        { "9317-1234" , "9317-1234"},
        { "ABCD-8142" , "abcd-8142"},
    };

    int nb_fails = 0;
    for (const auto& test : test_set_001) {
        auto word_str_original = test.first;
        auto word_str_expected = test.second;
        Word w(word_str_original);
        std::cout << "Rune: " << w.get_hash() << std::endl;
        auto word_str_back = w.get_hash();
        std::cout << "Rune: " << word_str_back << std::endl;

        CHECK(word_str_expected == word_str_back);

    }
}

TEST_CASE("translate_rune_sequence", "[translate]") {

    PRINT_TEST_HEADER("translate_rune_sequence");

    Dictionary dictionary(DICTIONARY_EN);

    const std::vector< std::pair<std::vector<Rune>, std::string> > test_set_001 = {
        { { Rune(RUNE_VOWEL_A) } , "A"  },
        { { Rune(RUNE_CONSONANT_R) } , "R"  },
        { { Rune(RUNE_VOWEL_OO) } , "OO"  },
        { { Rune(RUNE_CONSONANT_N) } , "N"  },
        { { Rune(RUNE_VOWEL_A) } , "A"  },
        { { Rune(RUNE_VOWEL_A) } , "A"  },
        { { Rune(RUNE_CONSONANT_R | RUNE_VOWEL_OO), Rune(RUNE_VOWEL_OU | RUNE_CONSONANT_N | RUNE_REVERSE) } , "ruin"  },
        { { Rune(RUNE_CONSONANT_S | RUNE_VOWEL_I), Rune(RUNE_CONSONANT_K | RUNE_VOWEL_AR) } , "seeker"  },
        { { Rune(RUNE_CONSONANT_R | RUNE_VOWEL_OO), Rune(RUNE_VOWEL_OU | RUNE_CONSONANT_N | RUNE_REVERSE) , Rune(RUNE_NULL), Rune(RUNE_CONSONANT_S | RUNE_VOWEL_I), Rune(RUNE_CONSONANT_K | RUNE_VOWEL_AR) } , "ruin seeker"  },
    };

    for (const auto& test : test_set_001) {
        auto rune_sequence = test.first;
        auto expected = test.second;

        auto translated = dictionary.translate(rune_sequence);
        CHECK(expected == translated);

    }
}


TEST_CASE("image_detect_words", "[image]") {

    PRINT_TEST_HEADER("image_detect_words");

    Dictionary dictionary(DICTIONARY_EN);
    RuneDetector rune_detector(&dictionary);
    rune_detector.load_rune_folder(RUNES_FOLDER);

    const std::vector< std::pair<std::string, std::string> > test_set_001 = {
       { "../../../data/screenshots/found_an_item.jpg" , "found an item"},
       //{ "../../../data/screenshots/found_bracelet.png" , "found an item"},
       //{ "../../../data/screenshots/manual_page_10.jpg" , "lots of runes !!!"},
        { "../../../data/screenshots/manual_page_10_inverted.jpg" , "lots of runes !!!"},
    };

    int nb_fails = 0;
    for (const auto& test : test_set_001) {
        auto image_path = test.first;
        auto expected = test.second;

        std::vector<Word> detected_words;
        rune_detector.detect_words(fs::path(image_path), detected_words, true, true);

        auto translated = dictionary.translate(detected_words);
        CHECK(expected == translated);

    }
}


TEST_CASE("image_detect_single_word", "[image]") {
    PRINT_TEST_HEADER("image_detect_single_word");

    const auto TEMP_FOLDER = fs::path("tmp");
    fs::create_directory(TEMP_FOLDER);

    const std::vector<std::pair<std::string, std::string>> test_set_001 = {
        { "../../../data/screenshots/found_an_item.jpg" , "2988-0304-03a0_found"},
        { "../../../data/screenshots/found_an_item.jpg" , "9317_an"},
        { "../../../data/screenshots/found_an_item.jpg" , "a08e-9f10_item"},
        //{ "../../../data/screenshots/found_bracelet.png" , "found an item"},
        { "../../../data/screenshots/manual_page_10.jpg" , "23a3_the"},
    };

    int nb_fails = 0;
    for (const auto& test : test_set_001) {
        auto& image_path = test.first;
        auto& word = test.second;
        auto word_hash = word.substr(0, word.find("_"));
        auto word_transl = word.substr(word.find("_") + 1);

        // create an empty dictionary
        Dictionary dictionary;
        dictionary.add_word(word_hash, word_transl);
        deleteFilesInFolder(TEMP_FOLDER);
        dictionary.generate_images(TEMP_FOLDER);

        RuneDetector rune_detector(&dictionary);
        rune_detector.load_rune_folder(TEMP_FOLDER);

        std::vector<Word> detected_words;
        rune_detector.detect_words(fs::path(image_path), detected_words, true);

        auto translated = dictionary.translate(detected_words);

        int detected_occurences = std::count(detected_words.begin(), detected_words.end(), Word(word_hash));
        CHECK(detected_occurences > 0);

    }
}


TEST_CASE("rune_image_generation", "[image]") {

    PRINT_TEST_HEADER("rune_image_generation");

    Dictionary dictionary(DICTIONARY_EN);
    std::vector<std::string> hash_list;
    dictionary.get_hash_list(hash_list);
    int nb_fails = 0;
    for (const auto& word_hash : hash_list) {
        Word word(word_hash);
        std::string word_str;
        dictionary.get_translation(word_hash, word_str);

        cv::Mat result;
        auto rune_size = RUNE_DEFAULT_SIZE; // Size of the rune image
        auto tickness = RUNE_SEGMENT_DRAW_DEFAULT_TICKNESS * rune_size.height; // Thickness of the lines in the image 
        word.generate_image(rune_size, tickness, result);


        cv::imshow(word_str + " " + word_hash, result);
        cv::waitKey(1000); // Wait for a key press to close the window
        cv::destroyAllWindows();

        nb_fails += 0; // Placeholder for actual checks
    }
    CHECK(nb_fails == 0);
}

TEST_CASE("dictionary_image_gen", "[image]") {

    PRINT_TEST_HEADER("dictionary_image_gen");

    Dictionary dictionary(DICTIONARY_EN);
    dictionary.save(DICTIONARY_EN);

    fs::path current_path = fs::path("../../../data/runes");
    dictionary.generate_images(current_path, ".png");

}

TEST_CASE("decode_word_image", "[image]") {


    PRINT_TEST_HEADER("decode_word_image");

    Dictionary dictionary(DICTIONARY_EN);
    RuneDetector rune_detector(&dictionary);

    fs::path unknown_words_folder = fs::path("../../../data/unknown_words");

    // Check if the folder exists
    if (!fs::exists(unknown_words_folder) || !fs::is_directory(unknown_words_folder)) {
        std::cerr << "Error: Rune folder does not exist or is not a directory: " << unknown_words_folder << std::endl;
        CHECK(false);
    }

    for (const auto& entry : fs::directory_iterator(unknown_words_folder)) {
        Word decoded_word;
        if (!rune_detector.decode_word_image(entry.path(), decoded_word)) {
            std::cerr << "Error: Could not decode word image: " << entry.path() << std::endl;
            continue; // Skip to the next file if decoding fails
        }


        cv::Mat detected_word_img;
        cv::Size rune_size = RUNE_DEFAULT_SIZE;
        auto tickness = RUNE_SEGMENT_DRAW_DEFAULT_TICKNESS * rune_size.height; // Thickness of the lines in the image

        if (decoded_word.generate_image(rune_size, tickness, detected_word_img)) {

            auto decoded_word_str = decoded_word.to_pseudophonetic();
            auto decoded_word_hash = decoded_word.get_hash();

            auto filename = entry.path().stem().string();
            auto expected_word_hash = filename.substr(0, filename.find_first_of('_'));
            auto expected_word_str = Word(expected_word_hash).to_pseudophonetic();

            std::string original_path = entry.path().string();
            auto original_img = cv::imread(original_path.c_str(), cv::IMREAD_COLOR_BGR);

            CHECK(decoded_word_hash == expected_word_hash);
            CHECK(decoded_word_str == expected_word_str);

            if (decoded_word_hash != expected_word_hash || decoded_word_str != expected_word_str) {
                std::cerr << "Decoded word does not match expected: " << decoded_word_hash << " != " << expected_word_hash << std::endl
                    << " or " << std::endl
                    << decoded_word_str << " != " << expected_word_str << std::endl;

                cv::imshow("Original: ", original_img);
                cv::imshow("Decoded: " + decoded_word_hash + " " + decoded_word_str, detected_word_img);

                cv::waitKey(1000); // Wait for a key press to close the window
                cv::destroyAllWindows();
            }
        }
        else {
            std::cerr << "Error: Could not decode word image: " << entry.path() << std::endl;
        }

    }
}


TEST_CASE("dictionary_load_save", "[translate]") {


    PRINT_TEST_HEADER("dictionary_load_save");

    fs::path temp_file = "test_dictionary.txt";

    Dictionary dictionary(DICTIONARY_EN);
    dictionary.save(temp_file);

    auto original_dictionary_lines = loadLinesFromFile(DICTIONARY_EN);
    auto saved_dictionary_lines = loadLinesFromFile(temp_file);

    int nb_fails = 0;

    for (const auto& o_line : original_dictionary_lines) {
        if (o_line.starts_with("#") || o_line.empty()) {
            continue; // Skip comments and empty lines
        }
        if (std::find(saved_dictionary_lines.begin(), saved_dictionary_lines.end(), o_line) == saved_dictionary_lines.end()) {
            std::cerr << "Line not found in saved dictionary: " << o_line << std::endl;
            nb_fails++;
        }
    }
    CHECK(nb_fails == 0);
    CHECK(nb_fails == 0);
}

TEST_CASE("dictionarize", "[image][translate]") {

    PRINT_TEST_HEADER("dictionarize");

    int nb_fails = 0;
    fs::path temp_file = "test_dictionarize.txt";

    Dictionary dictionary;
    RuneDetector rune_detector(&dictionary);

    rune_detector.dictionarize(fs::path("../../../data/screenshots/manual_page_10.jpg"), true);

    std::cout << "Detected words in the image:" << std::endl;
    std::vector<std::string> word_list;
    dictionary.get_word_list(word_list);
    for (const auto& key : word_list) {
        std::string translation;
        dictionary.get_translation(key, translation);
        std::cout << "Word: " << key << " - Translation: " << translation << std::endl;
    }

    dictionary.save(temp_file);

    CHECK(nb_fails == 0);
}

TEST_CASE("arpeggio_sequence", "[audio]") {

    PRINT_TEST_HEADER("arpeggio_sequence");

    Dictionary dictionary(DICTIONARY_EN);
    dictionary.save(DICTIONARY_EN);

    const std::vector< std::pair<std::vector<int>, std::string> > test_set_001 = {
        { {13, 12, 9, 6, 2, 1} , "own"},
        { {13, 12, 6, 2, 1, 3, 6, 10, 12, 13} , "enter"},
    };

    int nb_fails = 0;

    RuneDetector rune_detector(&dictionary);
    ArpeggioDetector arpeggio_detector;
    arpeggio_detector.load_dictionary(dictionary);

    for (const auto& test : test_set_001) {
        auto arpeggio_sequence = test.first;
        Arpeggio arpeggio(arpeggio_sequence);
        auto expected = test.second;

        std::vector<Rune> detected_runes;
        //auto runes = arpeggio.to_runes();
        //Word word(runes);
        auto word = arpeggio_detector.find(arpeggio);

        CHECK(word.is_valid());
        if (word.is_valid()) {
            std::string translation = dictionary.translate(word);
            CHECK(translation == expected);
            nb_fails += (translation == expected ? 0 : 1);
        }

    }

    CHECK(nb_fails == 0);
    CHECK(nb_fails == 0);
}


TEST_CASE("audio_get_indexed_note_sequence", "[audio]") {

    PRINT_TEST_HEADER("audio_get_indexed_note_sequence");

    Dictionary dictionary(DICTIONARY_EN);

    ArpeggioDetector arpeggio_detector;
    arpeggio_detector.load_dictionary(dictionary);

    // test set from the documentation
    const std::vector< std::pair<std::vector<Note>, std::vector<int>> > test_set_001 = {
        { {Note::F5, Note::G5, Note::D6, Note::G6, Note::C7, Note::D7 } , {1, 2, 6, 9, 12, 13}},
        { {Note::C5, Note::D5, Note::A5, Note::D6, Note::G6, Note::A6 } , {1, 2, 6, 9, 12, 13}},
        { {Note::Eb5, Note::F5, Note::C6, Note::F6, Note::Bb6, Note::C7 } , {1, 2, 6, 9, 12, 13}},
        { {Note::Bb4, Note::C5, Note::G5, Note::C6, Note::F6, Note::G6 } , {1, 2, 6, 9, 12, 13}},

    };

    int nb_fails = 0;
    for (const auto& test : test_set_001) {
        auto note_sequence = test.first;
        auto expected = test.second;

        int scale = 0;
        std::vector<int> indexed_sequence, best_seq;
        int min_diff = INT_MAX;
        int best_scale = -1;
        for (scale = static_cast<int>(ScaleType::Major); scale <= static_cast<int>(ScaleType::Chromatic); ++scale) {
            indexed_sequence = ArpeggioDetector::get_indexed_note_sequence(note_sequence, (ScaleType)scale);

            auto diff = get_index_sequence_diff(indexed_sequence, expected);
            if (diff < min_diff) {
                min_diff = diff;
                best_scale = scale;
                best_seq = indexed_sequence;
            }

            if (indexed_sequence == expected)
                break;
        }
        CHECK(best_seq == expected);

        std::cout << "Scale used:" << (int)best_scale << std::endl;
        std::cout << "expected:         ";
        for (auto x : expected) std::cout << x << " ";
        std::cout << std::endl;
        std::cout << "indexed_sequence: ";
        for (auto x : indexed_sequence) std::cout << x << " ";
        std::cout << std::endl;

        if (indexed_sequence != expected) {
            nb_fails++;
        }
        std::cout << std::endl;
    }

    CHECK(nb_fails == 0);
}

TEST_CASE("audio_detect_words", "[audio][translate]") {

    PRINT_TEST_HEADER("audio_detect_words");

    Dictionary dictionary(DICTIONARY_EN);

    ArpeggioDetector arpeggio_detector;
    arpeggio_detector.load_dictionary(dictionary);

    // test set from the documentation
    const std::vector< std::pair<std::vector<Note>, std::vector<std::string>> > test_set_001 = {
        {  {Note::F5, Note::G5, Note::D6, Note::G6, Note::C7, Note::D7 } , {"no"}},
        {  {Note::C5, Note::D5, Note::A5, Note::D6, Note::G6, Note::A6 } , {"no"}},
        {  {Note::Eb5, Note::F5, Note::C6, Note::F6, Note::Bb6, Note::C7 } , {"no"}},
        {  {Note::Bb4, Note::C5, Note::G5, Note::C6, Note::F6, Note::G6 } , {"no"}},
        {  {Note::F5, Note::G5, Note::D6, Note::G6, Note::C7, Note::D7, Note::C5, Note::D5, Note::A5, Note::D6, Note::G6, Note::A6, Note::Eb5, Note::F5, Note::C6, Note::F6, Note::Bb6, Note::C7, Note::Bb4, Note::C5, Note::G5, Note::C6, Note::F6, Note::G6 } , {"no", "no", "no", "no"}},
    };

    int nb_fails = 0;
    for (const auto& test : test_set_001) {
        auto note_sequence = test.first;
        auto expected = test.second;

        std::vector<Word> words;
        arpeggio_detector.detect_words(note_sequence, words);

        std::vector<std::string> result;
        for (const auto& word : words) {
            std::string translation = "";
            dictionary.get_translation(word.get_hash(), translation);
            result.push_back(translation);
        }

        CHECK(result == expected);
        nb_fails += (result == expected ? 0 : 1);
    }

    CHECK(nb_fails == 0);
}

TEST_CASE("audio_detection_generated_wav_detect_notes", "[audio]") {

    PRINT_TEST_HEADER("audio_detection_generated_wav_detect_notes");

    Dictionary dictionary(DICTIONARY_EN);

    const double STD_NOTE_DURATION = 0.150;
    const std::vector<std::vector<Note>> test_set_001 = {
        {Note::C3, Note::D3, Note::E3},
        {Note::F5, Note::G5, Note::D6, Note::G6, Note::C7, Note::D7 },
    };

    int index = 0;
    int nb_fails = 0;
    for (const auto& test : test_set_001) {
        index++;
        fs::path wav_file = fs::path("gen_wav" + std::to_string(index) + ".wav");
        std::vector<Note> expected_note_seq = test;

        std::vector<std::pair<Note, double>> notes_with_durations;
        for (const auto& note : expected_note_seq) {
            notes_with_durations.push_back(std::pair(note, STD_NOTE_DURATION));
        }

        generate_wav(notes_with_durations, wav_file);

        ArpeggioDetector detector;
        auto detected_sequence = detector.detect_note_sequence(wav_file, 0, true);
        auto clean_detected_sequence = detector.get_clean_sequence(detected_sequence);
        std::cout << std::endl;
        CHECK(clean_detected_sequence == expected_note_seq);
    }

    CHECK(nb_fails == 0);
}

TEST_CASE("audio_detection", "[audio][translate]") {

    PRINT_TEST_HEADER("audio_detection");

    Dictionary dictionary(DICTIONARY_EN);
    ArpeggioDetector detector;

    const std::vector< std::pair<std::string, std::string> > test_set_001 = {
        { "../../../data/audio_captures/open_chest.wav" , "found an item"},
    };

    int nb_fails = 0;
    for (const auto& test : test_set_001) {
        auto audio_file = test.first;
        auto expected = test.second;

        auto file = fs::path(audio_file);
        std::string result;
        detector.audio_detection(DICTIONARY_EN, file, 25, true, result);
        CHECK(result == expected);
        nb_fails += (expected != result);
    }

    CHECK(nb_fails == 0);
}
