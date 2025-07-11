#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <queue>
#include <filesystem>
#include <string>
#include <cstdint>
#include <chrono>
#include <cmath>
#include "runedictionary.h"
#include "arpeggiodetector.h"
#include "rune.h"
#include "runedetector.h"
#include "word.h"
#include "color_print.h"
#include "note.h"
#include "yin.h"



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
const auto DICTIONARY_ENG = fs::path("../../../lang/dictionary.eng.txt");


///////////////////////////////////////////////////////
//  TEST CASES
///////////////////////////////////////////////////////

TEST_CASE("unify_rune_hash", "[translate]") {

    PRINT_TEST_HEADER("unify_rune_hash");

    RuneDictionary dictionary(DICTIONARY_ENG);
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

    RuneDictionary dictionary(DICTIONARY_ENG);
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

    RuneDictionary dictionary(DICTIONARY_ENG);

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

    RuneDictionary dictionary(DICTIONARY_ENG);
    RuneDetector rune_detector(&dictionary);
    rune_detector.load_rune_folder(RUNES_FOLDER);

    const std::vector< std::pair<std::string, std::string> > test_set_001 = {
       //{ "../../../data/screenshots/found_an_item.jpg" , "found an item"},
       //{ "../../../data/screenshots/found_bracelet.png" , "found an item"},
       //{ "../../../data/screenshots/manual_page_10.jpg" , "lots of runes !!!"},
       //{ "../../../data/screenshots/manual_page_10_inverted.jpg" , "lots of runes !!!"},
         { "../../../data/screenshots/manual_page_10.jpg" , "lots of runes !!!"},
    };

    int nb_fails = 0;
    for (const auto& test : test_set_001) {
        auto image_path = test.first;
        auto expected = test.second;

        std::vector<Word> detected_words;
        auto original_img = cv::imread(image_path, cv::IMREAD_COLOR_BGR);
        if (original_img.empty()) {
        	std::cerr << "Error: Could not load rune image from " << image_path << std::endl;
            continue;
        }
        rune_detector.detect_words(original_img, detected_words, true, true);

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
        RuneDictionary dictionary;
        dictionary.add_word(word_hash, word_transl);
        deleteFilesInFolder(TEMP_FOLDER);
        dictionary.generate_images(TEMP_FOLDER);

        RuneDetector rune_detector(&dictionary);
        rune_detector.load_rune_folder(TEMP_FOLDER);

        std::vector<Word> detected_words;
        auto original_img = cv::imread(image_path, cv::IMREAD_COLOR_BGR);
        if (original_img.empty()) {
            std::cerr << "Error: Could not load rune image from " << image_path << std::endl;
            continue;
        }
        rune_detector.detect_words(original_img, detected_words, true);

        auto translated = dictionary.translate(detected_words);

        int detected_occurences = std::count(detected_words.begin(), detected_words.end(), Word(word_hash));
        CHECK(detected_occurences > 0);

    }
}


TEST_CASE("rune_image_generation", "[image]") {

    PRINT_TEST_HEADER("rune_image_generation");

    RuneDictionary dictionary(DICTIONARY_ENG);
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

    RuneDictionary dictionary(DICTIONARY_ENG);
    dictionary.save(DICTIONARY_ENG);

    fs::path current_path = fs::path("../../../data/runes");
    dictionary.generate_images(current_path, ".png");

}

TEST_CASE("decode_word_image", "[image]") {


    PRINT_TEST_HEADER("decode_word_image");

    RuneDictionary dictionary(DICTIONARY_ENG);
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

    RuneDictionary dictionary(DICTIONARY_ENG);
    dictionary.save(temp_file);

    auto original_dictionary_lines = loadLinesFromFile(DICTIONARY_ENG);
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

    RuneDictionary dictionary;
    RuneDetector rune_detector(&dictionary);

    rune_detector.dictionarize(fs::path("../../../data/screenshots/manual_page_10.jpg"), true);

    std::cout << "Detected words in the image:" << std::endl;
    std::vector<std::string> word_list;
    dictionary.get_hash_list(word_list);
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

    RuneDictionary dictionary(DICTIONARY_ENG);
    dictionary.save(DICTIONARY_ENG);

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

    RuneDictionary dictionary(DICTIONARY_ENG);

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

    RuneDictionary dictionary(DICTIONARY_ENG);

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

    RuneDictionary dictionary(DICTIONARY_ENG);

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

    RuneDictionary dictionary(DICTIONARY_ENG);
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
        detector.audio_detection(DICTIONARY_ENG, file, 25, true, result);
        CHECK(result == expected);
        nb_fails += (expected != result);
    }

    CHECK(nb_fails == 0);
}



//TEST_CASE("translate_dictionary", "[audio][translate]") {
//
//    PRINT_TEST_HEADER("translate_dictionary");
//
//    const auto LANG_DICT_FOLDER = fs::path("../../../dict");
//    const auto RUNE_DICT_FILE = fs::path("../../../lang/dictionary.eng.txt");
//
//    for (const auto& entry : std::filesystem::directory_iterator(LANG_DICT_FOLDER)) {
//        if (entry.is_regular_file() && fs::path(entry).extension() == ".tei" && fs::path(entry).stem().string().find("eng-") == 0) {
//            translate_dictionary(RUNE_DICT_FILE, entry);
//        }
//    }
//}



TEST_CASE("detect_runes_brightness", "[image]") {

    PRINT_TEST_HEADER("detect_runes_brightness");

    bool debug_mode = true;

    const std::vector<std::pair<std::string, Brightness>> test_set = {
                {"../../../data/screenshots/manual_page_10.jpg", BRIGHTNESS_DARK},
         {"../../../data/screenshots/manual_page_10_inverted.jpg", BRIGHTNESS_BRIGHT},
        {"../../../data/screenshots/manual_page_3.jpg", BRIGHTNESS_DARK},
        {"../../../data/screenshots/manual_page_3_inverted.jpg", BRIGHTNESS_BRIGHT},
    };

    for (const auto& item : test_set) {

        const auto& img = item.first;
        const auto& expected = item.second;

        // --- Load an image ---
        // Replace "path/to/your/image.jpg" with the actual path to your image file.
        // If you don't have an image, you can create a dummy one for testing.
        cv::Mat originalImage = cv::imread(img);

        // Check if the image was loaded successfully
        if (originalImage.empty()) {
            std::cerr << "Error: Could not open or find the image." << std::endl;
            std::cerr << "Please replace 'path/to/your/image.jpg' with a valid image path." << std::endl;
            // Create a dummy image for demonstration if loading fails
            std::cout << "Creating a dummy image for demonstration..." << std::endl;
            originalImage = cv::Mat(400, 600, CV_8UC3, cv::Scalar(0, 0, 0)); // Black background
            // Draw some lines on the dummy image
            cv::line(originalImage, cv::Point(50, 50), cv::Point(550, 50), cv::Scalar(0, 255, 0), 5); // Green horizontal (bright on black)
            cv::line(originalImage, cv::Point(100, 100), cv::Point(100, 300), cv::Scalar(255, 0, 0), 5); // Blue vertical (bright on black)
            cv::line(originalImage, cv::Point(200, 100), cv::Point(400, 300), cv::Scalar(0, 0, 255), 5); // Red diagonal (bright on black)
            cv::line(originalImage, cv::Point(450, 50), cv::Point(500, 350), cv::Scalar(255, 255, 0), 3); // Yellow diagonal (bright on black)

            // Add some "darker" lines if the background was lighter, or on a bright background
            // For demonstration purposes, let's make a white background and draw dark lines
            cv::Mat brightBackground = cv::Mat(400, 600, CV_8UC3, cv::Scalar(255, 255, 255)); // White background
            cv::line(brightBackground, cv::Point(50, 350), cv::Point(550, 350), cv::Scalar(50, 50, 50), 5); // Dark gray horizontal
            cv::line(brightBackground, cv::Point(500, 100), cv::Point(500, 300), cv::Scalar(100, 0, 0), 5); // Dark red vertical
            cv::addWeighted(originalImage, 0.5, brightBackground, 0.5, 0, originalImage); // Blend for mixed lines
        }

        // Define line detection parameters
        // TODO: change hardcoded values to parameters or relative sizes
        double minLineLength = 50; // Minimum length of a line to be detected
        double maxLineGap = 10;    // Maximum allowed gap between points on the same line to link them

        // Vector to store detected lines, passed by reference to detectAndMaskStraightLines
        std::vector<cv::Vec4i> detectedLines;

        // --- Process the image ---
        cv::Mat resultImage = detectAndMaskStraightLines(originalImage, minLineLength, maxLineGap, detectedLines);

        // --- Analyze line brightness ---
        // Define thresholds for dark and bright lines (0-255 scale)
        double darkThreshold = 80;  // Pixels with average intensity below this are considered dark
        double brightThreshold = 180; // Pixels with average intensity above this are considered bright
        auto brightness = analyzeLineBrightness(originalImage, detectedLines, darkThreshold, brightThreshold, debug_mode);
        CHECK(brightness == expected);

        if (debug_mode) {
            // --- Display the images ---
            cv::imshow("Original Image", originalImage);
            cv::imshow("Lines Masked Image", resultImage);


            // To see the generated mask explicitly:
            // The mask generation logic is already within detectAndMaskStraightLines,
            // but we can recreate it for explicit display if needed.
            cv::Mat explicitMask = cv::Mat::zeros(originalImage.size(), originalImage.type());
            for (const auto& line : detectedLines) { // Use the same detectedLines from the function call
                cv::Point pt1(line[0], line[1]);
                cv::Point pt2(line[2], line[3]);
                cv::line(explicitMask, pt1, pt2, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
            }
            cv::imshow("Explicit Line Mask", explicitMask);


            // Wait for a key press indefinitely
            cv::waitKey(0);

            // Destroy all OpenCV windows
            cv::destroyAllWindows();
        }

    }
}



///////////////////////////////////////////////////
//   BENCH
///////////////////////////////////////////////////

TEST_CASE("bench_detectct_words_load_resize_word_vs_dynamic_draw", "[image][bench]")
{
    PRINT_TEST_HEADER("bench_detectct_words_load_resize_word_vs_dynamic_draw");

    const auto TEST_IMG = "../../../data/screenshots/manual_page_3_inverted.jpg";
    const auto scale_factors = { 0.13, 0.23, 0.33, 0.43, 0.53, 1.13, 1.23, 1.33, 1.43, 1.53 };

    RuneDictionary dictionary(DICTIONARY_ENG);
    dictionary.save(DICTIONARY_ENG);
    dictionary.load(DICTIONARY_ENG);
    dictionary.generate_images("../../../data/runes");
    RuneDetector rune_detector(&dictionary);
    rune_detector.load_rune_folder(RUNES_FOLDER);

    std::vector<double> adapt_scale_factors_confirmed;
    int adapt_detections = 0;
    cv::Mat original_img = cv::imread(TEST_IMG);
    cv::Mat image;
    cv::cvtColor(original_img, image, cv::COLOR_BGR2GRAY);
    image.convertTo(image, CV_8U);
    std::vector<RuneZone> detected_runes_zones;

    // load and resize bench
    auto start_loadandresize = std::chrono::high_resolution_clock::now();
    for (const auto& [key, pattern_image_original] : rune_detector.m_rune_images) {
        auto word = Word(key);
        for (const auto& scale_factor : scale_factors) {
            // Resize the rune image to the current scale factor
            cv::Mat pattern_image;
            cv::resize(pattern_image_original, pattern_image, cv::Size(), scale_factor, scale_factor, (scale_factor > 1.0f ? cv::INTER_LINEAR : cv::INTER_AREA));
            //cv::imshow("load and resize", pattern_image);
            //cv::waitKey(0);
        }
    }
    auto end_loadandresize = std::chrono::high_resolution_clock::now();
    auto duration_loadandresize = end_loadandresize - start_loadandresize;
    long long duration_loadandresize_ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration_loadandresize).count();

    // generate image bench
    auto start_genimg = std::chrono::high_resolution_clock::now();
    for (const auto& [key, pattern_image_original] : rune_detector.m_rune_images) {
        auto word = Word(key);
        for (const auto& scale_factor : scale_factors) {
            // Resize the rune image to the current scale factor
            cv::Mat pattern_image;
            auto size = RUNE_DEFAULT_SIZE * scale_factor;
            double thickness = (std::max)((double)1.0f, RUNE_SEGMENT_DRAW_DEFAULT_TICKNESS * RUNE_DEFAULT_SIZE.height * scale_factor);
            word.generate_image(size, thickness, pattern_image);
            //cv::imshow("draw dynmically", pattern_image);
            //cv::waitKey(0);
        }
    }
    auto end_genimg = std::chrono::high_resolution_clock::now();
    auto duration_genimg = end_genimg - start_genimg;
    long long duration_genimg_ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration_genimg).count();

    printf("============ BENCH RESULTS ============\n");
    printf("duration_loadandresize_ms: %lld\n", duration_loadandresize_ms);
    printf("duration_genimg_ms: %lld\n", duration_genimg_ms);
    printf("\n");
}

TEST_CASE("bench_image_detect_load_resize_word_vs_dynamic_draw", "[image][bench]")
{
    PRINT_TEST_HEADER("bench_image_detect_load_resize_word_vs_dynamic_draw");

    const auto SRC_IMG = "../../../data/screenshots/manual_page_3_inverted.jpg";
    const auto DST_IMG_1 = "../../../data/screenshots/manual_page_3_inverted_decoded_1.jpg";
    const auto DST_IMG_2 = "../../../data/screenshots/manual_page_3_inverted_decoded_2.jpg";

    RuneDictionary dictionary(DICTIONARY_ENG);
    dictionary.save(DICTIONARY_ENG);
    dictionary.load(DICTIONARY_ENG);
    dictionary.generate_images("../../../data/runes");
    RuneDetector rune_detector(&dictionary);
    rune_detector.load_rune_folder(RUNES_FOLDER);

    bool debug_mode = true;

    // load and resize bench
    auto start_loadandresize = std::chrono::high_resolution_clock::now();
    rune_detector.image_detection(SRC_IMG, DST_IMG_1, 7, false, debug_mode);
    auto end_loadandresize = std::chrono::high_resolution_clock::now();
    auto duration_loadandresize = end_loadandresize - start_loadandresize;
    long long duration_loadandresize_ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration_loadandresize).count();

    // generate image bench
    auto start_genimg = std::chrono::high_resolution_clock::now();
    rune_detector.image_detection(SRC_IMG, DST_IMG_2, 7, true, debug_mode);
    auto end_genimg = std::chrono::high_resolution_clock::now();
    auto duration_genimg = end_genimg - start_genimg;
    long long duration_genimg_ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration_genimg).count();

    printf("============ BENCH RESULTS ============\n");
    printf("duration_loadandresize_ms: %lld\n", duration_loadandresize_ms);
    printf("duration_genimg_ms: %lld\n", duration_genimg_ms);
    printf("\n");
}