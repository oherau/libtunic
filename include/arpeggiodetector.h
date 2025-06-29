#ifndef __ARPEGGIODETECTOR_H__
#define __ARPEGGIODETECTOR_H__

#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <string>
#include <cmath>
#include <complex>
#include <cstdint>
#include <map>
#include <algorithm>
#include <filesystem>
#include <numbers> // for Pi
#include "note.h"
#include <dictionary.h>
#include <arpeggio.h>

const auto DEFAULT_NOTE_LENGTH = 75.0f; // in millisec
const auto SILENCE_TONE_THRESHOLD = 10.0f;

// --- Structures WavHeader ---
// (Simplified for standard PCM files)
struct WavHeader {
    char riff[4]; // "RIFF"
    uint32_t overall_size;
    char wave[4]; // "WAVE"
    char fmt_chunk_marker[4]; // "fmt "
    uint32_t length_of_fmt;
    uint16_t format_type;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t byterate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data_chunk_header[4]; // "data"
    uint32_t data_size;
};

namespace FFT {
    using Complex = std::complex<double>;
    void fft(std::vector<Complex>& a) ;
}

// Hash table (std::map) to store note frequencies
const std::map<Note, double> target_notes = {
    // Octave 0
    {Note::C0, 16.35}, {Note::Db0, 17.32}, {Note::D0, 18.35}, {Note::Eb0, 19.45},
    {Note::E0, 20.60}, {Note::F0, 21.83}, {Note::Gb0, 23.12}, {Note::G0, 24.50},
    {Note::Ab0, 25.96}, {Note::A0, 27.50}, {Note::Bb0, 29.14}, {Note::B0, 30.87},
    // Octave 1
    {Note::C1, 32.70}, {Note::Db1, 34.65}, {Note::D1, 36.71}, {Note::Eb1, 38.89},
    {Note::E1, 41.20}, {Note::F1, 43.65}, {Note::Gb1, 46.25}, {Note::G1, 49.00},
    {Note::Ab1, 51.91}, {Note::A1, 55.00}, {Note::Bb1, 58.27}, {Note::B1, 61.74},
    // Octave 2
    {Note::C2, 65.41}, {Note::Db2, 69.30}, {Note::D2, 73.42}, {Note::Eb2, 77.78},
    {Note::E2, 82.41}, {Note::F2, 87.31}, {Note::Gb2, 92.50}, {Note::G2, 98.00},
    {Note::Ab2, 103.83}, {Note::A2, 110.00}, {Note::Bb2, 116.54}, {Note::B2, 123.47},
    // Octave 3
    {Note::C3, 130.81}, {Note::Db3, 138.59}, {Note::D3, 146.83}, {Note::Eb3, 155.56},
    {Note::E3, 164.81}, {Note::F3, 174.61}, {Note::Gb3, 185.00}, {Note::G3, 196.00},
    {Note::Ab3, 207.65}, {Note::A3, 220.00}, {Note::Bb3, 233.08}, {Note::B3, 246.94},
    // Octave 4
    {Note::C4, 261.63}, {Note::Db4, 277.18}, {Note::D4, 293.66}, {Note::Eb4, 311.13},
    {Note::E4, 329.63}, {Note::F4, 349.23}, {Note::Gb4, 369.99}, {Note::G4, 392.00},
    {Note::Ab4, 415.30}, {Note::A4, 440.00}, {Note::Bb4, 466.16}, {Note::B4, 493.88},
    // Octave 5
    {Note::C5, 523.25}, {Note::Db5, 554.37}, {Note::D5, 587.33}, {Note::Eb5, 622.25},
    {Note::E5, 659.26}, {Note::F5, 698.46}, {Note::Gb5, 739.99}, {Note::G5, 783.99},
    {Note::Ab5, 830.61}, {Note::A5, 880.00}, {Note::Bb5, 932.33}, {Note::B5, 987.77},
    // Octave 6
    { Note::C6, 1046.50 }, {Note::Db6, 1108.73}, {Note::D6, 1174.66}, {Note::Eb6, 1244.51},
    {Note::E6, 1318.51}, {Note::F6, 1396.91}, {Note::Gb6, 1479.98}, {Note::G6, 1567.98},
    {Note::Ab6, 1661.22}, {Note::A6, 1760.00}, {Note::Bb6, 1864.66}, {Note::B6, 1975.53},
    // Octave 7
    {Note::C7, 2093.00}, {Note::Db7, 2217.46}, {Note::D7, 2349.32}, {Note::Eb7, 2489.02},
    {Note::E7, 2637.02}, {Note::F7, 2793.83}, {Note::Gb7, 2959.96}, {Note::G7, 3135.96},
    {Note::Ab7, 3322.44}, {Note::A7, 3520.00}, {Note::Bb7, 3729.31}, {Note::B7, 3951.07},
    // Octave 8
    {Note::C8, 4186.01}, {Note::Db8, 4434.92}, {Note::D8, 4698.64}, {Note::Eb8, 4978.03},
    {Note::E8, 5274.04}, {Note::F8, 5587.65}, {Note::Gb8, 5919.91}, {Note::G8, 6271.93},
    {Note::Ab8, 6644.88}, {Note::A8, 7040.00}, {Note::Bb8, 7458.62}, {Note::B8, 7902.13}
};

// "Enum to string" function
std::string note_to_string(Note note, bool no_octave = false);
std::string notes_to_string(const std::deque<Note>& notes, bool no_octave = false);
std::string remove_octave(const std::string& str);

// Detector main class
class ArpeggioDetector {
public:
    ArpeggioDetector() =  default;
    bool load_dictionary(const Dictionary& dictionary);
    void processFile(const std::filesystem::path& filePath, std::vector<Note>& detected_sequence, double note_length = DEFAULT_NOTE_LENGTH);
    std::vector<Note> get_clean_sequence(const std::vector<Note>& sequence);
    Word find(const Arpeggio& arpeggio);
	//std::vector<Rune> get_runes_from_arpeggio(const Arpeggio& arpeggio);
    static std::vector<int> get_indexed_note_sequence(std::vector<Note>& note_sequence, ScaleType scale = ScaleType::Mixolydian);
    bool detect_words(const std::vector<Note>& notes, std::vector<Word>& words);
private:
    Note findClosestNote(double frequency);
    void detectNoteSequence(const std::vector<float>& samples, uint32_t sampleRate, std::vector<Note>& detected_sequence, double note_length = DEFAULT_NOTE_LENGTH);
    std::map<std::string, Arpeggio> m_arpeggio_dictionary;
    int m_arpeggio_min_length = INT_MAX;
	int m_arpeggio_max_length = 0;
};

#endif // __NOTEDETECTOR_H__