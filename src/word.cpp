#include "word.h"
#include "dictionary.h"

Word::Word(const std::string& str)
{
	parse_runes(str, m_runes);
}

std::string Word::get_hash() const
{
	std::stringstream ss;
	bool firstRune = true;
	for(const auto& rune : m_runes) {
		if(!firstRune) {
			ss << DICT_RUNE_SEPARATOR;
		} else {
			firstRune = false;
		}
		ss << rune.to_hexa();
	}
	return std::string(ss.str());
}

std::string Word::to_pseudophonetic() const
{
	std::stringstream ss;
	bool firstRune = true;
	for(const auto& rune : m_runes) {
		if(!firstRune) {
			ss << DICT_RUNE_SEPARATOR;
		} else {
			firstRune = false;
		}
		ss << rune.to_pseudophonetic();
	}
	return std::string(ss.str());
}


bool Word::parse_runes(const std::string& str, std::vector<Rune>& runes)
{
	std::string strUpper(str);
	std::stringstream ss;
	auto counter = 0;
	for (const char c : strUpper) {
		if (c == DICT_RUNE_SEPARATOR) {
			continue;
		}
		ss << c;
		counter++;
		if (counter >= 4) {
			// 4 characters max per rune
			std::string ssStr = ss.str();
			Rune rune;
			rune.from_hexa(ssStr);
			runes.push_back(rune);
			ss.str(""); // clear the stringstream
			counter = 0;
		}
	}

	return true;
}
