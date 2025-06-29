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
    UNKNOWN,
    SILENCE,
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