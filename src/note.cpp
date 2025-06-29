#include "note.h"

// Helper to get the base chromatic index for a given note name (e.g., C=0, C#=1, D=2, ..., B=11)
// This is used for scale root calculation.
int get_chromatic_index(char note_char, bool is_sharp_or_flat) {
    switch (note_char) {
    case 'C': return is_sharp_or_flat ? 1 : 0;
    case 'D': return is_sharp_or_flat ? 3 : 2;
    case 'E': return is_sharp_or_flat ? -1 : 4; // E# is F, Fb is E; not simple for flat/sharp
    case 'F': return is_sharp_or_flat ? 6 : 5;
    case 'G': return is_sharp_or_flat ? 8 : 7;
    case 'A': return is_sharp_or_flat ? 10 : 9;
    case 'B': return is_sharp_or_flat ? -1 : 11; // B# is C, Bb is A#
    default: return -1; // Error
    }
}



// --- 3. The Conversion Function ---

/**
 * @brief Converts a sequence of absolute note indices (MIDI numbers) to indices relative to a given scale,
 * considering octave shifts. The fundamental note is represented by index 1.
 * Octave shifts are represented as +(n*m), where n is the number of notes in the scale
 * and m is the octave difference from the fundamental.
 *
 * @param absolute_note_sequence A vector of absolute MIDI note numbers (0-127).
 * @param scale_type The type of scale to use (e.g., ScaleType::Major, ScaleType::Mixolydian).
 * @param root_note_midi_number The MIDI number of the root note of the scale (e.g., 60 for C4, 62 for D4).
 * @return A std::vector<ScaleRelativeIndex> where each element is the index of the corresponding
 * absolute note within the scale, considering octave. Notes not in the scale are -1.
 */
std::vector<ScaleRelativeIndex> convert_to_scale_relative_indices(
    const std::vector<AbsoluteNoteIndex>& absolute_note_sequence,
    ScaleType scale_type,
    AbsoluteNoteIndex root_note_midi_number)
{
    std::vector<ScaleRelativeIndex> relative_indices;

    auto it = scale_intervals.find(scale_type);
    if (it == scale_intervals.end()) {
        std::cerr << "Error: Unknown scale type provided." << std::endl;
        return {};
    }
    const std::vector<int>& intervals = it->second;
    const int num_notes_in_scale = static_cast<int>(intervals.size()); // 'n' in your formula

    // Determine the actual notes present in the scale (chromatic values 0-11)
    std::vector<AbsoluteNoteIndex> scale_notes_chromatic; // Stores 0-11 values
    int root_chromatic_base = root_note_midi_number % 12;

    for (int interval : intervals) {
        int scale_note_chromatic_index = (root_chromatic_base + interval) % 12;
        scale_notes_chromatic.push_back(scale_note_chromatic_index);
    }

    for (AbsoluteNoteIndex abs_note : absolute_note_sequence) {

        // Calculate the octave of the root note relative to C0
        int root_note_octave = root_note_midi_number / 12;
        auto rel_note = (abs_note - root_note_midi_number);

        // Calculate the chromatic value of the current note (0-11)
        int current_note_chromatic_index = abs_note % 12;
        int rel_current_note_chromatic_index = rel_note % 12;

        // Calculate the octave of the current note relative to root note
        int rel_current_note_octave = rel_note / 12;

        std::optional<ScaleRelativeIndex> found_relative_index;
        for (int i = 0; i < num_notes_in_scale; ++i) {
            if (scale_notes_chromatic[i] == current_note_chromatic_index) {
                // Base scale degree (1-based)
                ScaleRelativeIndex base_degree = static_cast<ScaleRelativeIndex>(i + 1);

                // Add octave offset: +(n * m)
                found_relative_index = base_degree + (rel_current_note_octave * num_notes_in_scale);
                break;
            }
        }

        if (found_relative_index.has_value()) {
            relative_indices.push_back(found_relative_index.value());
        }
        else {
            relative_indices.push_back(-1); // Note not in the scale
        }
    }

    return relative_indices;
}


// --- Utility for printing (for demonstration) ---
template <typename T>
void print_vector(const std::vector<T>& vec, const std::string& label) {
    if (!label.empty()) {
        std::cout << label;
    }
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}

/**
 * @brief Modifies a vector of Note enums, converting flat notes to their natural equivalents.
 *
 * This function iterates through the input vector and for each Note:
 * - If it's Db (any octave), it's changed to D.
 * - If it's Eb (any octave), it's changed to E.
 * - If it's Gb (any octave), it's changed to G.
 * - If it's Ab (any octave), it's changed to A.
 * - If it's Bb (any octave), it's changed to B.
 * - Other notes (natural notes, UNKNOWN, SILENCE) remain unchanged.
 *
 * @param notes A reference to the std::vector<Note> to be modified.
 */
void convertFlatsToNaturals(std::vector<Note>& notes) {
    for (Note& note : notes) {
        switch (note) {
        case Note::Db0: note = Note::D0; break;
        case Note::Eb0: note = Note::E0; break;
        case Note::Gb0: note = Note::G0; break;
        case Note::Ab0: note = Note::A0; break;
        case Note::Bb0: note = Note::B0; break;

        case Note::Db1: note = Note::D1; break;
        case Note::Eb1: note = Note::E1; break;
        case Note::Gb1: note = Note::G1; break;
        case Note::Ab1: note = Note::A1; break;
        case Note::Bb1: note = Note::B1; break;

        case Note::Db2: note = Note::D2; break;
        case Note::Eb2: note = Note::E2; break;
        case Note::Gb2: note = Note::G2; break;
        case Note::Ab2: note = Note::A2; break;
        case Note::Bb2: note = Note::B2; break;

        case Note::Db3: note = Note::D3; break;
        case Note::Eb3: note = Note::E3; break;
        case Note::Gb3: note = Note::G3; break;
        case Note::Ab3: note = Note::A3; break;
        case Note::Bb3: note = Note::B3; break;

        case Note::Db4: note = Note::D4; break;
        case Note::Eb4: note = Note::E4; break;
        case Note::Gb4: note = Note::G4; break;
        case Note::Ab4: note = Note::A4; break;
        case Note::Bb4: note = Note::B4; break;

        case Note::Db5: note = Note::D5; break;
        case Note::Eb5: note = Note::E5; break;
        case Note::Gb5: note = Note::G5; break;
        case Note::Ab5: note = Note::A5; break;
        case Note::Bb5: note = Note::B5; break;

        case Note::Db6: note = Note::D6; break;
        case Note::Eb6: note = Note::E6; break;
        case Note::Gb6: note = Note::G6; break;
        case Note::Ab6: note = Note::A6; break;
        case Note::Bb6: note = Note::B6; break;

        case Note::Db7: note = Note::D7; break;
        case Note::Eb7: note = Note::E7; break;
        case Note::Gb7: note = Note::G7; break;
        case Note::Ab7: note = Note::A7; break;
        case Note::Bb7: note = Note::B7; break;

        case Note::Db8: note = Note::D8; break;
        case Note::Eb8: note = Note::E8; break;
        case Note::Gb8: note = Note::G8; break;
        case Note::Ab8: note = Note::A8; break;
        case Note::Bb8: note = Note::B8; break;

            // No default case needed, as other notes remain unchanged
        }
    }
}
