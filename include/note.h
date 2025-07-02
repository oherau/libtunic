#include <vector>
#ifndef __NOTE_H__
#define __NOTE_H__

#include <deque>
#include <iostream>
#include <vector>
#include <numeric> // For std::iota (C++11+)
#include <algorithm> // For std::find, std::transform
#include <map>
#include <optional> // C++17 for std::optional

// Enum definition for all musical notes (MIDI values)
enum class Note {
    SILENCE = 0,
    UNKNOWN,
    C0=12, Db0, D0, Eb0, E0, F0, Gb0, G0, Ab0, A0, Bb0, B0,
    C1, Db1, D1, Eb1, E1, F1, Gb1, G1, Ab1, A1, Bb1, B1,
    C2, Db2, D2, Eb2, E2, F2, Gb2, G2, Ab2, A2, Bb2, B2,
    C3, Db3, D3, Eb3, E3, F3, Gb3, G3, Ab3, A3, Bb3, B3,
    C4, Db4, D4, Eb4, E4, F4, Gb4, G4, Ab4, A4, Bb4, B4,
    C5, Db5, D5, Eb5, E5, F5, Gb5, G5, Ab5, A5, Bb5, B5,
    C6, Db6, D6, Eb6, E6, F6, Gb6, G6, Ab6, A6, Bb6, B6,
    C7, Db7, D7, Eb7, E7, F7, Gb7, G7, Ab7, A7, Bb7, B7,
    C8, Db8, D8, Eb8, E8, F8, Gb8, G8, Ab8, A8, Bb8, B8,
};

// Global constant map for note frequencies
const std::map<Note, double> note_frequencies = {
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
    {Note::Ab8, 6644.88}, {Note::A8, 7040.00}, {Note::Bb8, 7458.62}, {Note::B8, 7902.13},
    // Special: Rest
    {Note::SILENCE, 0.0} // Frequency for a rest is 0
};

// --- 1. Define Note and Scale Types ---

// Represents an absolute MIDI note number (0-127)
// C0 = 12, C4 = 60 (Middle C)
using AbsoluteNoteIndex = int;

// Represents the index within a specific scale (0 to 6 for heptatonic scales, etc.)
using ScaleRelativeIndex = int;

enum class ScaleType {
    Major,
    MinorNatural,
    MinorHarmonic,
    MinorMelodicAscending, // Different ascending vs descending often
    Dorian,
    Phrygian,
    Lydian,
    Mixolydian,
    Aeolian, // Same as Natural Minor
    Locrian,
    PentatonicMajor,
    PentatonicMinor,
    Blues,
    Chromatic // All 12 notes, relative index will just be modulo 12
};

// --- 2. Scale Definitions (Intervals from Root in Semitones) ---
// These are standard Western 12-tone intervals.
// 0 means the root itself.
static const std::map<ScaleType, std::vector<int>> scale_intervals = {
    {ScaleType::Major, {0, 2, 4, 5, 7, 9, 11}}, // W W H W W W H
    {ScaleType::MinorNatural, {0, 2, 3, 5, 7, 8, 10}}, // W H W W H W W
    {ScaleType::MinorHarmonic, {0, 2, 3, 5, 7, 8, 11}}, // W H W W H Aug2 H
    {ScaleType::MinorMelodicAscending, {0, 2, 3, 5, 7, 9, 11}}, // W H W W W W H
    {ScaleType::Dorian, {0, 2, 3, 5, 7, 9, 10}},
    {ScaleType::Phrygian, {0, 1, 3, 5, 7, 8, 10}},
    {ScaleType::Lydian, {0, 2, 4, 6, 7, 9, 11}},
    {ScaleType::Mixolydian, {0, 2, 4, 5, 7, 9, 10}},
    {ScaleType::Aeolian, {0, 2, 3, 5, 7, 8, 10}}, // Same as MinorNatural
    {ScaleType::Locrian, {0, 1, 3, 5, 6, 8, 10}},
    {ScaleType::PentatonicMajor, {0, 2, 4, 7, 9}}, // C D E G A
    {ScaleType::PentatonicMinor, {0, 3, 5, 7, 10}}, // C Eb F G Bb
    {ScaleType::Blues, {0, 3, 5, 6, 7, 10}}, // C Eb F Gb G Bb
    {ScaleType::Chromatic, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}} // All 12 notes
};

// Helper to get the base chromatic index for a given note name (e.g., C=0, C#=1, D=2, ..., B=11)
// This is used for scale root calculation.
int get_chromatic_index(char note_char, bool is_sharp_or_flat);

/**
 * @brief Converts a sequence of absolute note indices (MIDI numbers) to indices relative to a given scale.
 *
 * @param absolute_note_sequence A vector of absolute MIDI note numbers (0-127).
 * @param scale_type The type of scale to use (e.g., ScaleType::Major, ScaleType::Mixolydian).
 * @param root_note_midi_number The MIDI number of the root note of the scale (e.g., 60 for C4, 62 for D4).
 * This determines the starting point and specific notes of the scale.
 * @return A std::vector<ScaleRelativeIndex> where each element is the index of the corresponding
 * absolute note within the scale (0 for root, 1 for 2nd degree, etc.).
 * Notes not in the scale will be represented as -1.
 */
std::vector<ScaleRelativeIndex> convert_to_scale_relative_indices(
    const std::vector<AbsoluteNoteIndex>& absolute_note_sequence,
    ScaleType scale_type,
    AbsoluteNoteIndex root_note_midi_number);


// --- Utility for printing (for demonstration) ---
template <typename T>
void print_vector(const std::vector<T>& vec, const std::string& label = "");

void convertFlatsToNaturals(std::vector<Note>& notes);


#endif // __NOTE_H__