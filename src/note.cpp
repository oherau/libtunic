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
        // Calculate the chromatic value of the current note (0-11)
        int current_note_chromatic_index = abs_note % 12;

        // Calculate the octave of the current note relative to C0 (MIDI note 0)
        int current_note_octave = abs_note / 12; // Integer division gives octave number (MIDI convention)

        // Calculate the octave of the root note relative to C0
        int root_note_octave = root_note_midi_number / 12;

        // Determine the octave difference 'm'
        // m = 0 if in the same octave as root
        // m = 1 if one octave higher than root's octave
        // m = -1 if one octave lower than root's octave
        int octave_difference_m = current_note_octave - root_note_octave;

        std::optional<ScaleRelativeIndex> found_relative_index;
        for (int i = 0; i < num_notes_in_scale; ++i) { // Iterate using int i for index
            if (scale_notes_chromatic[i] == current_note_chromatic_index) {
                // Base scale degree (1-based)
                ScaleRelativeIndex base_degree = static_cast<ScaleRelativeIndex>(i + 1);

                // Add octave offset: +(n * m)
                found_relative_index = base_degree + (octave_difference_m * num_notes_in_scale);
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
