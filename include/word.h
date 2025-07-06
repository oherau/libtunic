#ifndef __WORD_H__
#define __WORD_H__

#include <vector>
#include <string>
#include <rune.h>

class Word {
public:
	Word() = default;
	Word(const std::string& str);
	Word(const std::vector<Rune>& runes);
	bool operator==(const Word& other) const { return m_runes == other.m_runes; }
	std::string get_hash() const;
	std::string to_pseudophonetic() const;
	bool parse_runes(const std::string& str, std::vector<Rune>& runes);
	bool decode_image(const cv::Mat& word_image);
	size_t size() const { return m_runes.size(); }
	bool generate_image(cv::Size2i rune_size, int tickness, cv::Mat& output_image) const;
	std::vector<Rune> get_runes() const { return m_runes; }
	bool is_valid() const { return !m_runes.empty(); }
private:
	std::vector<Rune> m_runes;
};


#endif __WORD_H__
