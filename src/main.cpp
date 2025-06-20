// Sample code
#include <iostream>
#include <vector>
#include <queue>
#include <filesystem>
#include "dictionary.h"
#include "notedetector.h"
#include "rune.h"
#include <runedetector.h>
#include <word.h>
//#include "libtuneic.h"
namespace fs = std::filesystem;

const auto RUNES_FOLDER = fs::path("data/runes");
const auto DICTIONARY_EN = fs::path("lang/dictionary.txt");

// prototypes
int test();
int test_audio();
int test_image();
int test_rune();
int test_word();

int audio_detection(const fs::path& dictionary_file, const fs::path& audio_file, double note_length);
int image_detection(const fs::path& dictionary_file, const fs::path& image_file);

int main(int argc, char* argv[]) {

    std::cerr << "argc: " << argc << "   ";
    // TEST MODE
    if (argc == 1) {
        return test();
    }

    if (argc != 2 && argc != 3) {
        std::cerr << "Usage: "
            << argv[0] << " <file.wav> [note_length_millisec=75]>" << std::endl
            << argv[0] << " (test mode)" << std::endl;
        return 1;
    }

    std::string filename = argv[1];

    int detection_result = -1;

    if (fs::path(filename).extension() == ".wav") {
        double note_length = 75;
        if (argc >= 3) {
            note_length = std::stod(argv[2]);
        }

        int detection_result = audio_detection(DICTIONARY_EN, fs::path(filename), note_length);
    }
    if (fs::path(filename).extension() == ".jpg") {
        int detection_result = image_detection(RUNES_FOLDER, fs::path(filename));
    }

    return detection_result;
}

int image_detection(const fs::path& dictionary_file, const fs::path& image_file) {

    // Load dictionary
    Dictionary dictionary(dictionary_file);
    RuneDetector rune_detector(&dictionary);
	rune_detector.load_rune_folder(RUNES_FOLDER);

    std::vector<Word> detected_words;
    rune_detector.detect_words(image_file, detected_words);
    // Afficher la séquence de runes detectees
    std::cout << "==== DETECTED RUNES ====" << std::endl;
    for(const auto& word : detected_words) {
        std::cout << word.get_hash() << " ";
    }
    std::cout << std::endl << std::endl;

    
    std::cout << "==== TRANSLATION ==== " << std::endl;
    for(const auto& word : detected_words) {
        std::cout << word.get_hash() << " -> " << dictionary.translate(word) << std::endl;
	}

	return -1; // Placeholder for image detection logic
}

int audio_detection(const fs::path& dictionary_file, const fs::path& audio_file, double note_length) {

    // DETECTION MODE
    Dictionary dictionary(dictionary_file);

    std::vector<float> audioData;

    NoteDetector noteDetector;
    std::vector<Note> detected_sequence;
    noteDetector.processFile(audio_file, detected_sequence, note_length);

    // detected sequence
    std::cout << "==== detected_sequence ====" << std::endl;
    for(size_t i = 0; i < detected_sequence.size(); ++i) {
        std::cout << note_to_string(detected_sequence[i]) << " ";
    }
    std::cout << std::endl << std::endl;

    auto note_sequence = noteDetector.get_clean_sequence(detected_sequence);

    // Afficher la séquence nettoyee
    std::cout << "==== NOTE SEQUENCE ====" << std::endl;
    for(size_t i = 0; i < note_sequence.size(); ++i) {
        std::cout << note_to_string(note_sequence[i]) << " ";
    }
    std::cout << std::endl << std::endl;


    std::string translated = dictionary.translate(note_sequence);
    std::cout << std::endl ;
    std::cout << "==== TRANSLATION ==== " << std::endl << translated << std::endl << std::endl;

    return 0;
}

template <typename T>
int test_check(const T& expected, const T& result) {
    bool testOK = (result == expected);
    auto resultStr = testOK ? "[ OK ] " : "[ KO ] ";
    std::cout << resultStr << "expected: [" << expected << "]  result: [" << result << "]" << std::endl;
    return testOK ? 0 : 1;
}

int test() {
    int result = 0;
    result += test_rune();
    result += test_word();
	result += test_audio();
    result += test_image();

    if (result == 0) {
        std::cout << "All tests passed!" << std::endl;
    } else {
        std::cout << result << " tests failed." << std::endl;
    }
	return result;
}

int test_rune() {
    std::cout << std::endl << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << "==========     RUNE - TEST MODE    ===========" << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << std::endl;

    Dictionary dictionary(DICTIONARY_EN);
    RuneDetector rune_detector(&dictionary);
    rune_detector.load_rune_folder(RUNES_FOLDER);



    const std::vector< std::pair<std::string, std::string> > test_set_001 = {
        { "9317" , "9317"},
        { "ABCD" , "abcd"},
        { "abcd" , "abcd"},
    };

    int nb_fails = 0;
    for (const auto& test : test_set_001) {
        auto rune_str_original = test.first;
        auto rune_str_expected = test.second;
        Rune r;
		r.from_hexa(rune_str_original);
		std::cout << "Rune: " << r.to_pseudophonetic() << std::endl;
		auto rune_str_back = r.to_hexa();
        std::cout << "Rune: " << rune_str_back << std::endl;

        nb_fails += test_check(rune_str_expected, rune_str_back);

    }

    return nb_fails;
}

int test_word() {
    std::cout << std::endl << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << "==========     WORD - TEST MODE     ===========" << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << std::endl;

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

        nb_fails += test_check(word_str_expected, word_str_back);

    }

    return nb_fails;
}

int test_audio() {
    std::cout << std::endl << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << "==========     AUDIO - TEST MODE    ===========" << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << std::endl;

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

    int nb_fails = 0;
    for(const auto& test : test_set_001) {
        auto rune_sequence = test.first;
        auto expected = test.second;

        auto translated = dictionary.translate(rune_sequence);
        nb_fails += test_check(expected, translated);

    }

    return nb_fails;
}

int test_image() {
    std::cout << std::endl << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << "==========     IMAGE - TEST MODE    ===========" << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << std::endl;

    Dictionary dictionary(DICTIONARY_EN);
	RuneDetector rune_detector(&dictionary);
	rune_detector.load_rune_folder(RUNES_FOLDER);

    const std::vector< std::pair<std::string, std::string> > test_set_001 = {
        { "data/screenshots/found_an_item.jpg" , "found an item"},
        { "data/screenshots/found_bracelet.png" , "found an item"},
    };

    int nb_fails = 0;
    for (const auto& test : test_set_001) {
        auto image_path = test.first;
        auto expected = test.second;

        std::vector<Word> detected_words;
        rune_detector.detect_words(fs::path(image_path), detected_words);

        auto translated = dictionary.translate(detected_words);
        nb_fails += test_check(expected, translated);

    }

    return nb_fails;
}
