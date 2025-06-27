#ifndef __ARPEGGIO_H__
#define __ARPEGGIO_H__

#include <string>
#include <rune.h>
#include <word.h>






// bits describing arpeggio sequence
// C	D	Eb	G	Ab2	C2	D2	Eb2	G2	Ab3	C3

const auto NOTE_C   = 0b10000000000;
const auto NOTE_D   = 0b01000000000;
const auto NOTE_Eb  = 0b00100000000;
const auto NOTE_G   = 0b00010000000;
const auto NOTE_Ab2 = 0b00001000000;
const auto NOTE_C2  = 0b00000100000;
const auto NOTE_D2  = 0b00000010000;
const auto NOTE_Eb2 = 0b00000001000;
const auto NOTE_G2  = 0b00000000100;
const auto NOTE_Ab3 = 0b00000000010;
const auto NOTE_C3  = 0b00000000001;


// CONSONNANTS - same octave as the fundamental

const auto ARPEGGIO_CONSONANT_S = { 2 };
const auto ARPEGGIO_CONSONANT_NG = { 2, 3 };
const auto ARPEGGIO_CONSONANT_D = { 2, 5 };
const auto ARPEGGIO_CONSONANT_N = { 2, 6 };
const auto ARPEGGIO_CONSONANT_SH = { 2, 8 };
const auto ARPEGGIO_CONSONANT_THS = { 3 };
const auto ARPEGGIO_CONSONANT_K = { 3, 5 };
const auto ARPEGGIO_CONSONANT_T = { 3, 6 };
const auto ARPEGGIO_CONSONANT_R = { 3, 8 };
const auto ARPEGGIO_CONSONANT_M = { 5 };
const auto ARPEGGIO_CONSONANT_L = { 5, 6 };
const auto ARPEGGIO_CONSONANT_W = { 5, 8 };
const auto ARPEGGIO_CONSONANT_F = { 6 };
const auto ARPEGGIO_CONSONANT_P = { 6, 8 };
const auto ARPEGGIO_CONSONANT_B = { 8 };
const auto ARPEGGIO_CONSONANT_J = { 2, 3, 5 };
const auto ARPEGGIO_CONSONANT_Y = { 2, 3, 6 };
const auto ARPEGGIO_CONSONANT_CH = { 2, 3, 8 };
const auto ARPEGGIO_CONSONANT_H = { 2, 5, 6 };
const auto ARPEGGIO_CONSONANT_THZ = { 2, 5, 8 };
const auto ARPEGGIO_CONSONANT_V = { 2, 6, 8 };
const auto ARPEGGIO_CONSONANT_Z = { 3, 5, 6 };
const auto ARPEGGIO_CONSONANT_G = { 3, 5, 8 };
const auto ARPEGGIO_CONSONANT_ZH = { 3, 6, 8 };

// VOWELS - one octave up to the fundamental

const auto ARPEGGIO_VOWEL_OU = { 2, 3 };
const auto ARPEGGIO_VOWEL_OW = { 2, 5 };
const auto ARPEGGIO_VOWEL_UH = { 2, 6 };
const auto ARPEGGIO_VOWEL_OI = { 2, 8 };
const auto ARPEGGIO_VOWEL_IE = { 3, 5 };
const auto ARPEGGIO_VOWEL_AR = { 3, 6 };
const auto ARPEGGIO_VOWEL_I = { 3, 8 };
const auto ARPEGGIO_VOWEL_EH = { 5, 6 };
const auto ARPEGGIO_VOWEL_EER = { 5, 8 };
const auto ARPEGGIO_VOWEL_A = { 6, 8 };
const auto ARPEGGIO_VOWEL_ORE = { 2, 3, 5 };
const auto ARPEGGIO_VOWEL_EE = { 2, 3, 6 };
const auto ARPEGGIO_VOWEL_OH = { 2, 5, 6 };
const auto ARPEGGIO_VOWEL_AY = { 2, 5, 8 };
const auto ARPEGGIO_VOWEL_OO = { 2, 6, 8 };
const auto ARPEGGIO_VOWEL_IR = { 3, 5, 6 };
const auto ARPEGGIO_VOWEL_AH = { 3, 5, 8 };
const auto ARPEGGIO_VOWEL_ERE = { 5, 6, 8 };

class Arpeggio {
public:
    Arpeggio() = default;
	Arpeggio(const std::vector<Rune>& runes);
	Arpeggio(const std::vector<int>& sequence);
	Arpeggio(const Word& word);
    //Arpeggio(const std::initializer_list<int>& consonnant, const std::initializer_list<int>& vowel, bool reverse);
	bool append_rune(const Rune& rune);
	std::vector<Rune> to_runes() const;
	int get_length() const;
	static std::vector<int> get_rune_arpeggio_sequence(unsigned long rune);
	bool operator==(const Arpeggio& other) const;
private:
    std::vector<int> m_sequence;
};

#endif // __ARPEGGIO_H__