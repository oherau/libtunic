#include "arpeggio.h"
#include <ranges>

Arpeggio::Arpeggio(const std::vector<Rune>& runes)
{
	for(const auto& rune : runes) {
		append_rune(rune);
	}
}

Arpeggio::Arpeggio(const Word& word)
{
	for (const auto& rune : word.get_runes()) {
		append_rune(rune);
	}
}

bool Arpeggio::append_rune(const Rune& rune)
{
	auto vowel = rune.vowel_part().get_value();
	auto consonnant = rune.consonant_part().get_value();
	bool reverse = rune.is_reverse();

	auto vowel_seq = get_rune_arpeggio_sequence(vowel);
	auto consonnant_seq = get_rune_arpeggio_sequence(consonnant);

	std::vector<int> arpegio_seq;

	if (reverse) {
		for (auto note_height : std::views::reverse(vowel_seq)) {
			arpegio_seq.push_back(note_height + 7);
		}
		for (auto note_height : std::views::reverse(consonnant_seq)) {
			arpegio_seq.push_back(note_height);
		}
		arpegio_seq.push_back(1);
	}
	else {
		arpegio_seq.push_back(1); // Always start with the fundamental note = 1
		for (auto note_height : consonnant_seq) {
			arpegio_seq.push_back(note_height);
		}
		for (auto note_height : vowel_seq) {
			arpegio_seq.push_back(note_height + 7);
		}
	}

	if(!m_sequence.empty()) {
		if(m_sequence.back() == arpegio_seq.front()) {
			// If the last note of the current sequence is the same as the first note of the new sequence, we can skip it
			arpegio_seq.erase(arpegio_seq.begin());
		}
	}
	m_sequence.insert(m_sequence.end(), arpegio_seq.begin(), arpegio_seq.end());

	return true;
}

Rune Arpeggio::to_rune() const
{
	return Rune();
}

int Arpeggio::get_length() const
{
	return m_sequence.size();
}

std::vector<int> Arpeggio::get_rune_arpeggio_sequence(unsigned long rune)
{
	switch (rune) {
		case RUNE_CONSONANT_S:
			return ARPEGGIO_CONSONANT_S;
		case RUNE_CONSONANT_NG:
			return ARPEGGIO_CONSONANT_NG;
		case RUNE_CONSONANT_D:
			return ARPEGGIO_CONSONANT_D;
		case RUNE_CONSONANT_N:
			return ARPEGGIO_CONSONANT_N;
		case RUNE_CONSONANT_SH:
			return ARPEGGIO_CONSONANT_SH;
		case RUNE_CONSONANT_THS:
			return ARPEGGIO_CONSONANT_THS;
		case RUNE_CONSONANT_K:
			return ARPEGGIO_CONSONANT_K;
		case RUNE_CONSONANT_T:
			return ARPEGGIO_CONSONANT_T;
		case RUNE_CONSONANT_R:
			return ARPEGGIO_CONSONANT_R;
		case RUNE_CONSONANT_M:
			return ARPEGGIO_CONSONANT_M;
		case RUNE_CONSONANT_L:
			return ARPEGGIO_CONSONANT_L;
		case RUNE_CONSONANT_W:
			return ARPEGGIO_CONSONANT_W;
		case RUNE_CONSONANT_F:
			return ARPEGGIO_CONSONANT_F;
		case RUNE_CONSONANT_P:
			return ARPEGGIO_CONSONANT_P;
		case RUNE_CONSONANT_B:
			return ARPEGGIO_CONSONANT_B;
		case RUNE_CONSONANT_J:
			return ARPEGGIO_CONSONANT_J;
		case RUNE_CONSONANT_Y:
			return ARPEGGIO_CONSONANT_Y;
		case RUNE_CONSONANT_CH:
			return ARPEGGIO_CONSONANT_CH;
		case RUNE_CONSONANT_H:
			return ARPEGGIO_CONSONANT_H;
		case RUNE_CONSONANT_THZ:
			return ARPEGGIO_CONSONANT_THZ;
		case RUNE_CONSONANT_V:
			return ARPEGGIO_CONSONANT_V;
		case RUNE_CONSONANT_Z:
			return ARPEGGIO_CONSONANT_Z;
		case RUNE_CONSONANT_G:
			return ARPEGGIO_CONSONANT_G;
		case RUNE_CONSONANT_ZH:
			return ARPEGGIO_CONSONANT_ZH;
		case RUNE_VOWEL_OU:
			return ARPEGGIO_VOWEL_OU;
		case RUNE_VOWEL_OW:
			return ARPEGGIO_VOWEL_OW;
		case RUNE_VOWEL_UH:
			return ARPEGGIO_VOWEL_UH;
		case RUNE_VOWEL_OI:
			return ARPEGGIO_VOWEL_OI;
		case RUNE_VOWEL_IE:
			return ARPEGGIO_VOWEL_IE;
		case RUNE_VOWEL_A:
			return ARPEGGIO_VOWEL_A;
		case RUNE_VOWEL_AR:
			return ARPEGGIO_VOWEL_AR;
		case RUNE_VOWEL_I:
			return ARPEGGIO_VOWEL_I;
		case RUNE_VOWEL_EH:
			return ARPEGGIO_VOWEL_EH;
		case RUNE_VOWEL_EER:
			return ARPEGGIO_VOWEL_EER;
		case RUNE_VOWEL_OH:
			return ARPEGGIO_VOWEL_OH;
		case RUNE_VOWEL_ORE:
			return ARPEGGIO_VOWEL_ORE;
		case RUNE_VOWEL_EE:
			return ARPEGGIO_VOWEL_EE;
		case RUNE_VOWEL_OO:
			return ARPEGGIO_VOWEL_OO;
		case RUNE_VOWEL_AY:
			return ARPEGGIO_VOWEL_AY;
		case RUNE_NULL:
		default:
			return std::vector<int>();
	}
	return std::vector<int>();
}
