#include "richtext.h"
#include <string>
#include <unordered_map>
#include <functional>
#include <cmath>

RichText::RichText() :
	m_font(nullptr),
	m_characterSize(20),
	m_charVertices(sf::Triangles),
	m_charOutlineVertices(sf::Triangles),
	m_lineVertices(sf::Triangles),
	m_lineOutlineVertices(sf::Triangles),
	m_shouldUpdateVertices(false)
{
}

RichText::RichText(sf::Font const& font, sf::String const& string, uint characterSize) :
	m_font(&font),
	m_characterSize(characterSize),
	m_charVertices(sf::Triangles),
	m_charOutlineVertices(sf::Triangles),
	m_lineVertices(sf::Triangles),
	m_lineOutlineVertices(sf::Triangles),
	m_shouldUpdateVertices(true)
{
	initializeLineStarts();
	parseString(string);
}

RichText::~RichText() {
	for (auto it = m_stylizers.begin(); it != m_stylizers.end(); it++)
		delete it->second;
}

void RichText::parseString(sf::String const& s, bool append) {
	const std::unordered_map<std::string, Stylizer::StyleProperty> tagMap {
		{"b", Stylizer::Bold},
		{"i", Stylizer::Italic},
		{"u", Stylizer::Underlined},
		{"s", Stylizer::StrikeThrough},
		{"c", Stylizer::FillColor},
		{"ot", Stylizer::OutlineThickness},
		{"oc", Stylizer::OutlineColor},
		{"lts", Stylizer::LetterSpacing},
		{"lns", Stylizer::LineSpacing}
	};
	
	const std::unordered_map<std::string, sf::Color> colorMap {
		{"black", sf::Color::Black},
		{"white", sf::Color::White},
		{"red", sf::Color::Red},
		{"green", sf::Color::Green},
		{"blue", sf::Color::Blue},
		{"yellow", sf::Color::Yellow},
		{"magenta", sf::Color::Magenta},
		{"cyan", sf::Color::Cyan},
		{"transparent", sf::Color::Transparent}
	};
	
	m_shouldUpdateVertices = true;
	
	if (!append) {
		m_string.clear();
		for (auto it = m_stylizers.begin(); it != m_stylizers.end(); it++)
			delete it->second;
		m_stylizers.clear();
		m_modifiableStylizers.clear();
		
		m_charVertices.clear();
		m_lineVertices.clear();
		m_charOutlineVertices.clear();
		m_lineOutlineVertices.clear();

		m_totalDisplayableCharacters = 0;
		
		m_updateStartLine = 0;
	}
	else {
		m_updateStartLine = std::min(m_lineStart_i.size()-1, m_updateStartLine);
	}
	
	size_t i = 0;
	size_t true_i = m_string.getSize();
	size_t i_displayOnly = m_totalDisplayableCharacters;
	size_t len = s.getSize();
	while (i < len) {
		if (s[i] == '<') {
			std::vector<Stylizer*> stylizers;
			bool modifiable = false;
			int ID;
			
			size_t tags_end = s.find('>', i+1);
			sf::String tags = s.substring(i+1, tags_end-i-1);
			size_t spacePos = tags.find(' ');
			while (spacePos != sf::String::InvalidPos) {
				tags.erase(spacePos, 1);
				spacePos = tags.find(' ');
			}
			
			sf::String fullTag;
			size_t start = 0;
			size_t end;
			do {
				end = tags.find(',', start);
				fullTag = tags.substring(start, end-start);
				size_t argPos = fullTag.find('=');
				sf::String tag = fullTag.substring(0, argPos);
				bool ender = (tag.getSize() > 1 && tag[0] == '/');
				bool inactive = (tag.getSize() > 1 && tag[0] == '!');
				sf::String arg = (argPos >= fullTag.getSize()-1) ? "" : fullTag.substring(argPos+1);
				
				auto it = tagMap.find((ender || inactive) ? tag.substring(1) : tag);
				if (it != tagMap.end()) {
					switch (it->second) {
					case Stylizer::Bold:
					case Stylizer::Italic:
					case Stylizer::Underlined:
					case Stylizer::StrikeThrough:
						if (ender)
							stylizers.push_back(new EnderStylizer<bool>(it->second));
						else if (inactive)
							stylizers.push_back(new StarterStylizer<bool>(it->second));
						else
							stylizers.push_back(new StarterStylizer<bool>(it->second, arg == "0" ? false : true));
						break;
					case Stylizer::FillColor:
					case Stylizer::OutlineColor: {
						if (ender)
							stylizers.push_back(new EnderStylizer<sf::Color>(it->second));
						else if (inactive)
							stylizers.push_back(new StarterStylizer<sf::Color>(it->second));
						else {
							if (arg.getSize() == 0)
								break;
							if (arg[0] == '#') { //Hex color
								long fullCode;
								size_t j;
								try {
									fullCode = std::stol(arg.substring(1).toAnsiString(), &j, 16);
								} catch (...) {
									break;
								}
								if (j == 6) { //No transparency value
									stylizers.push_back(new StarterStylizer<sf::Color>(it->second, sf::Color((fullCode >> 16) & 0xFF, (fullCode >> 8) & 0xFF, fullCode & 0xFF)));
								}
								else if (j == 8) {
									stylizers.push_back(new StarterStylizer<sf::Color>(it->second, sf::Color((fullCode >> 24) & 0xFF, (fullCode >> 16) & 0xFF, (fullCode >> 8) & 0xFF, fullCode & 0xFF)));
								}
							}
							else {
								auto colorIt = colorMap.find(arg);
								if (colorIt != colorMap.end())
									stylizers.push_back(new StarterStylizer<sf::Color>(it->second, colorIt->second));
							}							
						}
						break;
					}
					case Stylizer::OutlineThickness:
					case Stylizer::LetterSpacing:
					case Stylizer::LineSpacing:
						if (ender)
							stylizers.push_back(new EnderStylizer<float>(it->second));
						else if (inactive)
							stylizers.push_back(new StarterStylizer<float>(it->second));
						else {
							if (arg.getSize() == 0)
								break;
							float f;
							size_t j;
							try {
								f = std::stof(arg.toAnsiString(), &j);
							} catch (...) {
								break;
							}
							if (j == arg.getSize()) {
								stylizers.push_back(new StarterStylizer<float>(it->second, f));
							}	
						}
						break;
					default:
						break;
					}	
				}
				else if (tag == "id") {
					if (arg.getSize() == 0)
						break;
					size_t j;
					int tempID;
					try {
						tempID = std::stoi(arg.toAnsiString(), &j);
					} catch (...) {
						break;
					}
					if (j == arg.getSize()) {
						modifiable = true;
						ID = tempID;
					}
				}
				
				start = end+1;
			} while (end != sf::String::InvalidPos);
			
			for (auto it = stylizers.begin(); it != stylizers.end(); it++) {
				m_stylizers.emplace(true_i, *it);
			}
			
			if (modifiable) {
				for (auto it = stylizers.begin(); it != stylizers.end(); it++) {
					m_modifiableStylizers.emplace(ID, *it);
				}
			}
			
			i = tags_end;
		}
		else if (s[i] != '\r') {
			if (s[i] == '\\' && i+1 < len)
				i++;
			m_string += s[i];
			true_i++;
			if (s[i] != ' ' && s[i] != '\n' && s[i] != '\t' && s[i] != '\r')
				i_displayOnly++;
		}
		i++;
	}
	
	m_totalDisplayableCharacters = i_displayOnly;
}

sf::String const& RichText::getParsedString() const { return m_string; }

void RichText::setFont(const sf::Font &font) {
	m_font = &font;
	m_shouldUpdateVertices = true;
	m_updateStartLine = 0;
	initializeLineStarts();
}

void RichText::setCharacterSize(uint size) { 
	m_characterSize = size;
	m_shouldUpdateVertices = true;
	m_updateStartLine = 0;
	initializeLineStarts();
}

void RichText::setStyle(sf::Uint32 style) {
	m_style.bolds.front() = style & sf::Text::Bold;
	m_style.italics.front() = style & sf::Text::Italic;
	m_style.underlineds.front() = style & sf::Text::Underlined;
	m_style.strikeThroughs.front() = style & sf::Text::StrikeThrough;
	m_updateStartLine = 0;
	m_shouldUpdateVertices = true;
}

void RichText::setStyle(int ID, sf::Uint32 style) {
	for (auto it = m_modifiableStylizers.find(ID); it != m_modifiableStylizers.end() && it->first == ID; it++) {
		switch (it->second->getType()) {
		case Stylizer::Bold:
			static_cast<StarterStylizer<bool>*>(it->second)->setValue(style & sf::Text::Bold);
			m_updateStartLine = std::min(m_updateStartLine, it->second->line);
			m_shouldUpdateVertices = true;
			break;
		case Stylizer::Italic:
			static_cast<StarterStylizer<bool>*>(it->second)->setValue(style & sf::Text::Italic);
			m_updateStartLine = std::min(m_updateStartLine, it->second->line);
			m_shouldUpdateVertices = true;
			break;
		case Stylizer::Underlined:
			static_cast<StarterStylizer<bool>*>(it->second)->setValue(style & sf::Text::Underlined);
			m_updateStartLine = std::min(m_updateStartLine, it->second->line);
			m_shouldUpdateVertices = true;
			break;
		case Stylizer::StrikeThrough:
			static_cast<StarterStylizer<bool>*>(it->second)->setValue(style & sf::Text::StrikeThrough);
			m_updateStartLine = std::min(m_updateStartLine, it->second->line);
			m_shouldUpdateVertices = true;
			break;
		default:
			break;
		}
	}
	m_shouldUpdateVertices = true;
}

void RichText::setStyle(int ID, sf::Uint32 style, bool activated) {
	for (auto it = m_modifiableStylizers.find(ID); it != m_modifiableStylizers.end() && it->first == ID; it++) {
		switch (it->second->getType()) {
		case Stylizer::Bold:
			if (style & sf::Text::Bold) {
				static_cast<StarterStylizer<bool>*>(it->second)->activated = activated;
				m_updateStartLine = std::min(m_updateStartLine, it->second->line);
				m_shouldUpdateVertices = true;
			}
			break;
		case Stylizer::Italic:
			if (style & sf::Text::Italic) {
				static_cast<StarterStylizer<bool>*>(it->second)->activated = activated;
				m_updateStartLine = std::min(m_updateStartLine, it->second->line);
				m_shouldUpdateVertices = true;
			}
			break;
		case Stylizer::Underlined:
			if (style & sf::Text::Underlined) {
				static_cast<StarterStylizer<bool>*>(it->second)->activated = activated;
				m_updateStartLine = std::min(m_updateStartLine, it->second->line);
				m_shouldUpdateVertices = true;
			}
			break;
		case Stylizer::StrikeThrough:
			if (style & sf::Text::StrikeThrough) {
				static_cast<StarterStylizer<bool>*>(it->second)->activated = activated;
				m_updateStartLine = std::min(m_updateStartLine, it->second->line);
				m_shouldUpdateVertices = true;
			}
			break;
		default:
			break;
		}
	}
}

void RichText::setFillColor(sf::Color color) {
	m_style.fillColors.front() = color;
	m_updateStartLine = 0;
	m_shouldUpdateVertices = true;
}

void RichText::setFillColor(int ID, sf::Color color) {
	for (auto it = m_modifiableStylizers.find(ID); it != m_modifiableStylizers.end() && it->first == ID; it++) {
		if (it->second->getType() == Stylizer::FillColor) {
			static_cast<StarterStylizer<sf::Color>*>(it->second)->setValue(color);
			m_updateStartLine = std::min(m_updateStartLine, it->second->line);
			m_shouldUpdateVertices = true;
		}
	}
}

void RichText::setFillColor(int ID, bool activated) {
	for (auto it = m_modifiableStylizers.find(ID); it != m_modifiableStylizers.end() && it->first == ID; it++) {
		if (it->second->getType() == Stylizer::FillColor) {
			static_cast<StarterStylizer<sf::Color>*>(it->second)->activated = activated;
			m_updateStartLine = std::min(m_updateStartLine, it->second->line);
			m_shouldUpdateVertices = true;
		}
	}
}

void RichText::setOutlineThickness(float thickness) {
	m_style.outlineThicknesses.front() = thickness;
	m_updateStartLine = 0;
	m_shouldUpdateVertices = true;
	
}

void RichText::setOutlineThickness(int ID, float thickness) {
	for (auto it = m_modifiableStylizers.find(ID); it != m_modifiableStylizers.end() && it->first == ID; it++) {
		if (it->second->getType() == Stylizer::OutlineThickness) {
			static_cast<StarterStylizer<float>*>(it->second)->setValue(thickness);
			m_updateStartLine = std::min(m_updateStartLine, it->second->line);
			m_shouldUpdateVertices = true;
		}
	}
}

void RichText::setOutlineThickness(int ID, bool activated) {
	for (auto it = m_modifiableStylizers.find(ID); it != m_modifiableStylizers.end() && it->first == ID; it++) {
		if (it->second->getType() == Stylizer::OutlineThickness) {
			static_cast<StarterStylizer<float>*>(it->second)->activated = activated;
			m_updateStartLine = std::min(m_updateStartLine, it->second->line);
			m_shouldUpdateVertices = true;
		}
	}
}

void RichText::setOutlineColor(sf::Color color) {
	m_style.outlineColors.front() = color;
	m_updateStartLine = 0;
	m_shouldUpdateVertices = true;
}

void RichText::setOutlineColor(int ID, sf::Color color) {
	for (auto it = m_modifiableStylizers.find(ID); it != m_modifiableStylizers.end() && it->first == ID; it++) {
		if (it->second->getType() == Stylizer::OutlineColor) {
			static_cast<StarterStylizer<sf::Color>*>(it->second)->setValue(color);
			m_updateStartLine = std::min(m_updateStartLine, it->second->line);
			m_shouldUpdateVertices = true;
		}
	}
}

void RichText::setOutlineColor(int ID, bool activated) {
	for (auto it = m_modifiableStylizers.find(ID); it != m_modifiableStylizers.end() && it->first == ID; it++) {
		if (it->second->getType() == Stylizer::OutlineColor) {
			static_cast<StarterStylizer<sf::Color>*>(it->second)->activated = activated;
			m_updateStartLine = std::min(m_updateStartLine, it->second->line);
			m_shouldUpdateVertices = true;
		}
	}
}


void RichText::setLetterSpacingFactor(float factor) {
	m_style.letterSpacingFactors.front() = factor;
	m_updateStartLine = 0;
	m_shouldUpdateVertices = true;
}

void RichText::setLetterSpacingFactor(int ID, float factor) {
	for (auto it = m_modifiableStylizers.find(ID); it != m_modifiableStylizers.end() && it->first == ID; it++) {
		if (it->second->getType() == Stylizer::LetterSpacing) {
			static_cast<StarterStylizer<float>*>(it->second)->setValue(factor);
			m_updateStartLine = std::min(m_updateStartLine, it->second->line);
			m_shouldUpdateVertices = true;	
		}
	}
}

void RichText::setLetterSpacingFactor(int ID, bool activated) {
	for (auto it = m_modifiableStylizers.find(ID); it != m_modifiableStylizers.end() && it->first == ID; it++) {
		if (it->second->getType() == Stylizer::LetterSpacing) {
			static_cast<StarterStylizer<float>*>(it->second)->activated = activated;
			m_updateStartLine = std::min(m_updateStartLine, it->second->line);
			m_shouldUpdateVertices = true;	
		}
	}
}

void RichText::setLineSpacingFactor(float factor) {
	m_style.lineSpacingFactors.front() = factor;
	m_updateStartLine = 0;
	m_shouldUpdateVertices = true;
}

void RichText::setLineSpacingFactor(int ID, float factor) {
	for (auto it = m_modifiableStylizers.find(ID); it != m_modifiableStylizers.end() && it->first == ID; it++) {
		if (it->second->getType() == Stylizer::LineSpacing) {
			static_cast<StarterStylizer<float>*>(it->second)->setValue(factor);
			m_updateStartLine = std::min(m_updateStartLine, it->second->line);
			m_shouldUpdateVertices = true;	
		}
	}
}

void RichText::setLineSpacingFactor(int ID, bool activated) {
	for (auto it = m_modifiableStylizers.find(ID); it != m_modifiableStylizers.end() && it->first == ID; it++) {
		if (it->second->getType() == Stylizer::LineSpacing) {
			static_cast<StarterStylizer<float>*>(it->second)->activated = activated;
			m_updateStartLine = std::min(m_updateStartLine, it->second->line);
			m_shouldUpdateVertices = true;	
		}
	}
}

uint RichText::getCharacterSize() const { return m_characterSize; }
sf::Uint32 RichText::getStyle() const {
	return (m_style.bolds.front() ? sf::Text::Bold : 0)
			+ (m_style.italics.front() ? sf::Text::Italic : 0)
			+ (m_style.underlineds.front() ? sf::Text::Underlined : 0)
			+ (m_style.strikeThroughs.front() ? sf::Text::StrikeThrough : 0);
}

sf::Color RichText::getFillColor() const { return m_style.fillColors.front(); }
float RichText::getOutlineThickness() const { return m_style.outlineThicknesses.front(); }
sf::Color RichText::getOutlineColor() const { return m_style.outlineColors.front(); }
float RichText::getLetterSpacingFactor() const { return m_style.letterSpacingFactors.front(); }
float RichText::getLineSpacingFactor() const { return m_style.lineSpacingFactors.front(); }

void RichText::setHorizontalLimit(float limit) {
	m_horizontalLimit = limit;
	m_shouldUpdateVertices = true;
	m_updateStartLine = 0;
}

float RichText::getHorizontalLimit() const { return m_horizontalLimit; }

void RichText::setCharacterLimit(size_t limit) {
	if (m_characterLimit == limit)
		return;
	
	if (!(m_characterLimit >= m_totalDisplayableCharacters && limit >= m_totalDisplayableCharacters)) {
		size_t startLine = 0;
		while (startLine < m_lineStart_i.size() && m_lineStart_char[startLine] / 6 < limit)
			startLine++;
		
		m_shouldUpdateVertices = true;
		m_updateStartLine = std::min(m_updateStartLine, (startLine == 0) ? 0 : startLine-1);
	}
	
	m_characterLimit = limit;
}

size_t RichText::getCharacterLimit() const { return m_characterLimit; }

size_t RichText::getMaxEffectiveCharacterLimit() const { return m_totalDisplayableCharacters; }

sf::FloatRect RichText::findCharacterBounds(size_t index) const {
	if (!m_font || m_string.getSize() == 0 || index >= m_string.getSize())
		return sf::FloatRect();
	
	float whitespaceWidth = m_font->getGlyph(L' ', m_characterSize, false).advance;
	float letterSpacing = (whitespaceWidth / 3.f) * (m_style.letterSpacingFactors.front() - 1.f);
	float lineSpacing = m_font->getLineSpacing(m_characterSize) * m_style.lineSpacingFactors.front();
	
	sf::Vector2f pos(0, lineSpacing - m_characterSize);
	bool inWord = false;
	float currentLineWidth = 0;
	bool intentionalLineBreak = true;
	float whiteSpaceWidthAtWordStart = 0;
	
	bool passedTarget = false;
	bool shouldStop = false;
	float extraWidth = 0;
	
	float characterWidth = 0;
	
	auto it = m_stylizers.begin();
	sf::Uint32 previousChar = 0;
	size_t i = 0;
	while (i < m_string.getSize() && !shouldStop) {
		if (i >= index)
			passedTarget = true;
		
		while (it != m_stylizers.end() && it->first <= i) {
			switch (it->second->stylize(m_style)) {
			case Stylizer::LetterSpacing:
				whitespaceWidth = m_font->getGlyph(L' ', m_characterSize, false).advance;
				letterSpacing = (whitespaceWidth / 3.f) * (m_style.letterSpacingFactors.back() - 1.f);
				whitespaceWidth += letterSpacing;
				break;
			case Stylizer::LineSpacing:
				lineSpacing = m_font->getLineSpacing(m_characterSize) * m_style.lineSpacingFactors.back();
				break;
			default:
				break;
			}
			it++;
		}
		
		switch (m_string[i]) {
		case ' ':
			if (i == index)
				characterWidth = whitespaceWidth;
			if (passedTarget) {
				shouldStop = true;
				break;
			}
			if (inWord) {
				inWord = false;
				whiteSpaceWidthAtWordStart = 0;
				currentLineWidth = pos.x;
				intentionalLineBreak = false;
			}
			
			pos.x += whitespaceWidth;
			if (intentionalLineBreak) {
				currentLineWidth = pos.x;
			}
			else {
				whiteSpaceWidthAtWordStart += whitespaceWidth;
			}
			break;
		case '\n':
			if (i == index)
				characterWidth = whitespaceWidth;
			if (passedTarget) {
				shouldStop = true;
				break;
			}
			inWord = false;
			whiteSpaceWidthAtWordStart = 0;
			pos.x = 0;
			pos.y += lineSpacing;
			currentLineWidth = pos.x;
			intentionalLineBreak = true;
			break;
		case '\t': {
			float added = whitespaceWidth*8;
			added -= fmodf(pos.x + added, whitespaceWidth*8);
			if (i == index)
				characterWidth = added;
			if (passedTarget) {
				shouldStop = true;
				break;
			}
			if (inWord) {
				inWord = false;
				whiteSpaceWidthAtWordStart = 0;
				currentLineWidth = pos.x;
				intentionalLineBreak = false;
			}
			
			pos.x += added;
			if (intentionalLineBreak) {
				currentLineWidth = pos.x;
			}
			else {
				whiteSpaceWidthAtWordStart += added;
			}
			break;
		}
		default:
			float added = m_font->getKerning(previousChar, m_string[i], m_characterSize) + m_font->getGlyph(m_string[i], m_characterSize, m_style.bolds.back()).advance + letterSpacing;
			pos.x += added;
			if (i == index)
				characterWidth = added;
			if (passedTarget)
				extraWidth += added;
			
			if (currentLineWidth > 0 && pos.x > m_horizontalLimit) {
				pos.x -= currentLineWidth + whiteSpaceWidthAtWordStart;
				pos.y += lineSpacing;
				currentLineWidth = 0;
			}
			break;
		}
		
		previousChar = m_string[i];
		i++;
	}
	
	m_style.rewind();
	return sf::FloatRect(pos.x - extraWidth, pos.y, characterWidth, lineSpacing);
}

sf::FloatRect RichText::getLocalBounds() const {
	updateVertices();
	return m_bounds;
}

sf::FloatRect RichText::getGlobalBounds() const {
	return getTransform().transformRect(getLocalBounds());
}

void RichText::initializeLineStarts() {
	if (!m_font)
		return;
	m_lineStart_i.clear();
	m_lineStart_i.push_back(0);
	m_lineStart_verticalPos.clear();
	m_lineStart_verticalPos.push_back(m_font->getLineSpacing(m_characterSize) * m_style.lineSpacingFactors.front());
	m_lineStart_char.clear();
	m_lineStart_char.push_back(0);
	m_lineStart_charOutline.clear();
	m_lineStart_charOutline.emplace(0, 0);
	m_lineStart_line.clear();
	m_lineStart_line.emplace(0, 0);
	m_lineStart_lineOutline.clear();
	m_lineStart_lineOutline.emplace(0, 0);
}

void addGlyphQuad(sf::VertexArray& vertices, sf::Vector2f position, sf::Color const& color, sf::Glyph const& glyph, float italicShear, float outlineThickness = 0) {
	float padding = 1.0;

	float left   = glyph.bounds.left - padding;
	float top    = glyph.bounds.top - padding;
	float right  = glyph.bounds.left + glyph.bounds.width + padding;
	float bottom = glyph.bounds.top  + glyph.bounds.height + padding;

	float u1 = static_cast<float>(glyph.textureRect.left) - padding;
	float v1 = static_cast<float>(glyph.textureRect.top) - padding;
	float u2 = static_cast<float>(glyph.textureRect.left + glyph.textureRect.width) + padding;
	float v2 = static_cast<float>(glyph.textureRect.top  + glyph.textureRect.height) + padding;

	sf::Vertex bottomLeft(sf::Vector2f(position.x + left  - italicShear * bottom - outlineThickness, position.y + bottom - outlineThickness), color, sf::Vector2f(u1, v2));
	sf::Vertex topRight(sf::Vertex(sf::Vector2f(position.x + right - italicShear * top - outlineThickness, position.y + top - outlineThickness), color, sf::Vector2f(u2, v1)));
			
	vertices.append(sf::Vertex(sf::Vector2f(position.x + left  - italicShear * top - outlineThickness, position.y + top - outlineThickness), color, sf::Vector2f(u1, v1)));
	vertices.append(topRight);
	vertices.append(bottomLeft);
	vertices.append(bottomLeft);
	vertices.append(topRight);
	vertices.append(sf::Vertex(sf::Vector2f(position.x + right - italicShear * bottom - outlineThickness, position.y + bottom - outlineThickness), color, sf::Vector2f(u2, v2)));
}

void addLine(sf::VertexArray& vertices, sf::Vector2f origin, float lineLength, const sf::Color& color, float thickness, float outlineThickness = 0)
{
	float top = std::floor(origin.y - (thickness / 2) + 0.5f);
	float bottom = top + std::floor(thickness + 0.5f);

	sf::Vertex bottomLeft(sf::Vector2f(origin.x - outlineThickness, bottom + outlineThickness), color, sf::Vector2f(1, 1));
	sf::Vertex topRight(sf::Vector2f(origin.x + lineLength + outlineThickness, top - outlineThickness), color, sf::Vector2f(1, 1));
	
	vertices.append(sf::Vertex(sf::Vector2f(origin.x - outlineThickness, top - outlineThickness), color, sf::Vector2f(1, 1)));
	vertices.append(topRight);
	vertices.append(bottomLeft);
	vertices.append(bottomLeft);
	vertices.append(topRight);
	vertices.append(sf::Vertex(sf::Vector2f(origin.x + lineLength + outlineThickness, bottom + outlineThickness), color, sf::Vector2f(1, 1)));
}

void resizeLineStartMap(std::map<size_t, size_t>& map, size_t const& firstLineToErase) {
	auto it = map.begin();
	while (it != map.end() && it->first < firstLineToErase) {
		it++;
	}
	map.erase(it, map.end());
}

size_t const& getLastVerticesIndexBeforeLine(std::map<size_t, size_t> const& map, size_t const& startLine) {
	auto it = map.lower_bound(startLine);
	if (it == map.end()) {
		return (--it)->second;
	}
	return it->second;
}

void roundNewVertices(sf::VertexArray& va, size_t newVerticesStart) {
	size_t len = va.getVertexCount();
	for (size_t i = newVerticesStart; i < len; i++) {
		va[i].position.x = roundf(va[i].position.x);
		va[i].position.y = roundf(va[i].position.y);
	}
}

void RichText::updateVertices() const {
	if (!m_font)
		return;
	
	if (!m_shouldUpdateVertices)
		return;
	
	//Updating from a certain line:	
	//Don't update if the starting line is after all explored lines
	if (m_updateStartLine >= m_lineStart_i.size()) {
		m_updateStartLine = std::numeric_limits<size_t>::max();
		return;
	}
	
	//First, make all the lines after the starting line unexplored:
	m_lineStart_i.resize(m_updateStartLine+1);
	m_lineStart_verticalPos.resize(m_updateStartLine+1);
	m_lineStart_char.resize(m_updateStartLine+1);
	resizeLineStartMap(m_lineStart_line, m_updateStartLine+1);
	resizeLineStartMap(m_lineStart_charOutline, m_updateStartLine+1);
	resizeLineStartMap(m_lineStart_lineOutline, m_updateStartLine+1);
	
	//Discard the vertex arrays' information starting from the starting line.
	//We keep the indices so that they can be used at the end of the program for pixel alignment of all new vertices
	size_t startOfNewCharVertices = m_lineStart_char[m_updateStartLine];
	m_charVertices.resize(startOfNewCharVertices);
	size_t startOfNewCharOutlineVertices = getLastVerticesIndexBeforeLine(m_lineStart_charOutline, m_updateStartLine);
	m_charOutlineVertices.resize(startOfNewCharOutlineVertices);
	size_t startOfNewLineVertices = getLastVerticesIndexBeforeLine(m_lineStart_line, m_updateStartLine);
	m_lineVertices.resize(startOfNewLineVertices);
	size_t startOfNewLineOutlineVertices = getLastVerticesIndexBeforeLine(m_lineStart_lineOutline, m_updateStartLine);
	m_lineOutlineVertices.resize(startOfNewLineOutlineVertices);
	
	//Populate the starting variables with the line start info
	size_t i = m_lineStart_i[m_updateStartLine];
	sf::Vector2f pos(0, m_lineStart_verticalPos[m_updateStartLine]);
	size_t i_displayOnly = m_charVertices.getVertexCount() / 6;
	
	//Populate the complex variables with the default style values
	m_style.rewind();
	float whitespaceWidth = m_font->getGlyph(L' ', m_characterSize, false).advance;
	float letterSpacing = (whitespaceWidth / 3.f) * (m_style.letterSpacingFactors.front() - 1.f);
	whitespaceWidth += letterSpacing;
	float lineSpacing = m_font->getLineSpacing(m_characterSize) * m_style.lineSpacingFactors.front();
	float lineThickness = m_font->getUnderlineThickness(m_characterSize);
	
	sf::Vector2f underlineStart(pos.x, pos.y + m_font->getUnderlinePosition(m_characterSize));
	sf::Vector2f underlineOutlineStart = underlineStart;
	sf::FloatRect xBounds = m_font->getGlyph(L'x', m_characterSize, false).bounds;
	sf::Vector2f strikeThroughStart(pos.x, pos.y + xBounds.top + xBounds.height * 0.4f);
	sf::Vector2f strikeThroughOutlineStart = strikeThroughStart;
	float italicShear = m_style.italics.back() ? 0.209f : 0.f;
	bool hasOutline = m_style.outlineThicknesses.front() != 0.f;
	
	//Now, update the style (and complex variables) by iterating through the stylizers up to the starting line
	auto it = m_stylizers.begin();
	while (it != m_stylizers.end() && it->first <= i) {
		switch (it->second->stylize(m_style)) {
		case Stylizer::Italic:
			italicShear = m_style.italics.back() ? 0.209f : 0.f;
			break;
		case Stylizer::OutlineThickness:
			hasOutline = m_style.outlineThicknesses.back() != 0.f;
			break;
		case Stylizer::LetterSpacing:
			whitespaceWidth = m_font->getGlyph(L' ', m_characterSize, false).advance;
			letterSpacing = (whitespaceWidth / 3.f) * (m_style.letterSpacingFactors.back() - 1.f);
			whitespaceWidth += letterSpacing;
			break;
		case Stylizer::LineSpacing:
			lineSpacing = m_font->getLineSpacing(m_characterSize) * m_style.lineSpacingFactors.back();
			break;
		default:
			break;
		}
		it++;
	}
	
	sf::VertexArray wordCharVertices = sf::VertexArray(sf::Triangles);
	sf::VertexArray wordLineVertices  = sf::VertexArray(sf::Triangles);
	sf::VertexArray wordCharOutlineVertices  = sf::VertexArray(sf::Triangles);
	sf::VertexArray wordLineOutlineVertices  = sf::VertexArray(sf::Triangles);
	
	float lineSpacingAtWordStart = lineSpacing;
	float outlineThicknessAtWordStart = m_style.outlineThicknesses.back();
	float whitespaceWidthAtWordStart = 0;
	size_t whitespacesAtWordStart = 0;
	size_t i_atWordStart = 0;	
	
	float currentLineWidth = 0.f;
	bool intentionalLineBreak = true; //When true, whitespace before the first word of a line can push it to line wrapping; becomes false after a line wrap (the whitespace "disappears").
	
	size_t currentLine = m_updateStartLine;
	m_updateStartLine = std::numeric_limits<size_t>::max();
	
	bool reachedCharacterLimit = false;
	
	const std::function<void()> addWordToText = [&]() { 
		for (size_t i = 0; i < wordCharVertices.getVertexCount(); i++)
			m_charVertices.append(wordCharVertices[i]);
		for (size_t i = 0; i < wordLineVertices.getVertexCount(); i+=6) {
			for (size_t j = i; j < i+6; j++)
				m_lineVertices.append(wordLineVertices[j]);
		}
		for (size_t i = 0; i < wordCharOutlineVertices.getVertexCount(); i++)
			m_charOutlineVertices.append(wordCharOutlineVertices[i]);
		for (size_t i = 0; i < wordLineOutlineVertices.getVertexCount(); i+=6) {
			for (size_t j = i; j < i+6; j++)
				m_lineOutlineVertices.append(wordLineOutlineVertices[j]);
		}
	};
	
	const std::function<void()> resetWord = [&]() {
		wordCharVertices.clear();
		wordLineVertices.clear();
		wordCharOutlineVertices.clear();
		wordLineOutlineVertices.clear();
		
		currentLineWidth = pos.x;
		lineSpacingAtWordStart = lineSpacing;
		outlineThicknessAtWordStart = m_style.outlineThicknesses.back();
		whitespaceWidthAtWordStart = 0;
		whitespacesAtWordStart = 0;
		i_atWordStart = i;
	};
	
	const std::function<void()> setLineStarts = [&]() {
		m_lineStart_i.push_back(i_atWordStart + whitespacesAtWordStart);
		m_lineStart_verticalPos.push_back(pos.y);
		
		m_lineStart_char.push_back(m_charVertices.getVertexCount());
		if (m_lineStart_charOutline.rbegin()->second != m_charOutlineVertices.getVertexCount()) //If any char outlines were added since the last line logged in the map
			m_lineStart_charOutline.emplace(currentLine, m_charOutlineVertices.getVertexCount());
		if (m_lineStart_line.rbegin()->second != m_lineVertices.getVertexCount())
			m_lineStart_line.emplace(currentLine, m_lineVertices.getVertexCount());
		if (m_lineStart_lineOutline.rbegin()->second != m_lineOutlineVertices.getVertexCount())
			m_lineStart_lineOutline.emplace(currentLine, m_lineOutlineVertices.getVertexCount());
	};
	
	sf::Uint32 previousChar = 0;
	size_t len = m_string.getSize();
	
	while (i < len) {
		if (i_displayOnly == m_characterLimit) {			
			if (m_style.underlineds.back()) {
				addLine(wordLineVertices, underlineStart, pos.x - underlineStart.x, m_style.fillColors.back(), lineThickness);
				if (hasOutline) {
					addLine(wordLineOutlineVertices, underlineOutlineStart, pos.x - underlineOutlineStart.x, m_style.outlineColors.back(), lineThickness, m_style.outlineThicknesses.back());
				}
			}
			if (m_style.strikeThroughs.back()) {
				addLine(wordLineVertices, strikeThroughStart, pos.x - strikeThroughStart.x, m_style.fillColors.back(), lineThickness);
				if (hasOutline) {
					addLine(wordLineOutlineVertices, strikeThroughOutlineStart, pos.x - strikeThroughOutlineStart.x, m_style.outlineColors.back(), lineThickness, m_style.outlineThicknesses.back());
				}
			}
			
			reachedCharacterLimit = true;
		}
		
		if (it != m_stylizers.end() && it->first == i) { //If stylizers exist at i
			bool shouldUpdateUnderline = false, shouldUpdateUnderlineOutline = false, shouldUpdateStrikeThrough = false, shouldUpdateStrikeThroughOutline = false;
			bool wasUnderlined = m_style.underlineds.back();
			bool wasStrikeThrough = m_style.strikeThroughs.back();
			bool hadOutline = hasOutline;
			float oldLineThickness = lineThickness;
			sf::Color oldFillColor = m_style.fillColors.back();
			float oldOutlineThickness = m_style.outlineThicknesses.back();
			sf::Color oldOutlineColor = m_style.outlineColors.back();
			
			while (it != m_stylizers.end() && it->first == i) { //Modify the style; the return type gives info on whether or not the modification changed the style visually
				it->second->line = currentLine;
				switch (it->second->stylize(m_style)) {
				case Stylizer::Italic:
					italicShear = m_style.italics.back() ? 0.209f : 0.f;
					break;
				case Stylizer::Underlined:
					shouldUpdateUnderline = true;
					shouldUpdateUnderlineOutline = true;
					break;
				case Stylizer::StrikeThrough:
					shouldUpdateStrikeThrough = true;
					shouldUpdateStrikeThroughOutline = true;
					break;
				case Stylizer::FillColor:
					shouldUpdateUnderline = true;
					shouldUpdateStrikeThrough = true;
					break;
				case Stylizer::OutlineThickness:
					shouldUpdateUnderlineOutline = true;
					shouldUpdateStrikeThroughOutline = true;
					hasOutline = m_style.outlineThicknesses.back() != 0.f;
					break;
				case Stylizer::OutlineColor:
					shouldUpdateUnderlineOutline = true;
					shouldUpdateStrikeThroughOutline = true;
					break;
				case Stylizer::LetterSpacing:
					whitespaceWidth = m_font->getGlyph(L' ', m_characterSize, false).advance;
					letterSpacing = (whitespaceWidth / 3.f) * (m_style.letterSpacingFactors.back() - 1.f);
					whitespaceWidth += letterSpacing;
					break;
				case Stylizer::LineSpacing:
					lineSpacing = m_font->getLineSpacing(m_characterSize) * m_style.lineSpacingFactors.back();
					break;
				default:
					break;
				}
				it++;
			}
				
			if (!reachedCharacterLimit) {
				if (shouldUpdateUnderline) {
					if (wasUnderlined)
						addLine(wordLineVertices, underlineStart, pos.x - underlineStart.x, oldFillColor, oldLineThickness);
					if (m_style.underlineds.back())
						underlineStart.x = pos.x;
				}
				if (shouldUpdateStrikeThrough) {
					if (wasStrikeThrough)
						addLine(wordLineVertices, strikeThroughStart, pos.x - strikeThroughStart.x, oldFillColor, oldLineThickness);
					if (m_style.strikeThroughs.back())
						strikeThroughStart.x = pos.x;
				}
				if (shouldUpdateUnderlineOutline) {
					if (hadOutline && wasUnderlined)
						addLine(wordLineOutlineVertices, underlineOutlineStart, pos.x - underlineOutlineStart.x, oldOutlineColor, oldLineThickness, oldOutlineThickness);
					if (hasOutline && m_style.underlineds.back())
						underlineOutlineStart.x = pos.x;
				}
				if (shouldUpdateStrikeThroughOutline) {
					if (hadOutline && wasStrikeThrough)
						addLine(wordLineOutlineVertices, strikeThroughOutlineStart, pos.x - strikeThroughOutlineStart.x, oldOutlineColor, oldLineThickness, oldOutlineThickness);
					if (hasOutline && m_style.strikeThroughs.back())
						strikeThroughOutlineStart.x = pos.x;
				}
			}
			
		}
		
		sf::Uint32 currentChar = m_string[i];
		
		bool shouldStop = false;
		
		switch (currentChar) {
		case ' ': {
			if (reachedCharacterLimit) {
				shouldStop = true;
				break;
			}
			if (wordCharVertices.getVertexCount() > 0) {
				addWordToText();
				resetWord();
				intentionalLineBreak = false;
			}
			
			pos.x += whitespaceWidth;
			if (intentionalLineBreak) {
				currentLineWidth = pos.x;
			}
			else {
				whitespaceWidthAtWordStart += whitespaceWidth;
				whitespacesAtWordStart++;
			}
			break;
		}
		case '\t': {
			if (reachedCharacterLimit) {
				shouldStop = true;
				break;
			}
			float added = whitespaceWidth*8;
			added -= fmodf(pos.x + added, whitespaceWidth*8);
			if (wordCharVertices.getVertexCount() > 0) {
				addWordToText();
				resetWord();
				intentionalLineBreak = false;
			}
			
			pos.x += added;
			if (intentionalLineBreak) {
				currentLineWidth = pos.x;
			}
			else {
				whitespaceWidthAtWordStart += added;
				whitespacesAtWordStart++;
			}
			break;
		}
		case '\n': {
			if (reachedCharacterLimit) {
				shouldStop = true;
				break;
			}
			
			addWordToText();
			resetWord();
			
			if (m_style.underlineds.back()) {
				addLine(m_lineVertices, underlineStart, pos.x - underlineStart.x, m_style.fillColors.back(), lineThickness);
				if (hasOutline) {
					addLine(m_lineOutlineVertices, underlineOutlineStart, pos.x - underlineOutlineStart.x, m_style.outlineColors.back(), lineThickness, m_style.outlineThicknesses.back());
				}
			}
			if (m_style.strikeThroughs.back()) {
				addLine(m_lineVertices, strikeThroughStart, pos.x - strikeThroughStart.x, m_style.fillColors.back(), lineThickness);
				if (hasOutline) {
					addLine(m_lineOutlineVertices, strikeThroughOutlineStart, pos.x - strikeThroughOutlineStart.x, m_style.outlineColors.back(), lineThickness, m_style.outlineThicknesses.back());
				}
			}
			pos.x = 0;
			pos.y += lineSpacing;
			
			underlineStart.x = 0;
			underlineStart.y += lineSpacing;
			underlineOutlineStart = underlineStart;
			strikeThroughStart.x = 0;
			strikeThroughStart.y += lineSpacing;
			strikeThroughOutlineStart = strikeThroughStart;
			
			currentLineWidth = 0;
			currentLine++;
			whitespacesAtWordStart++;
			
			setLineStarts();
			
			intentionalLineBreak = true;
			break;
		}
		default: {			
			pos.x += m_font->getKerning(previousChar, m_string[i], m_characterSize);
			
			sf::Glyph const& g = m_font->getGlyph(m_string[i], m_characterSize, m_style.bolds.back());
			if (!reachedCharacterLimit) {
				addGlyphQuad(wordCharVertices, pos, m_style.fillColors.back(), g, italicShear);
				if (hasOutline)
					addGlyphQuad(wordCharOutlineVertices, pos, m_style.outlineColors.back(), m_font->getGlyph(m_string[i], m_characterSize, m_style.bolds.back(), m_style.outlineThicknesses.back()), italicShear, m_style.outlineThicknesses.back());
			}
			pos.x += g.advance + letterSpacing;
			i_displayOnly++;
			
			//Move the word down a line if it became too long
			if (currentLineWidth != 0.f && pos.x > m_horizontalLimit) {
				float extendedLineWidth = currentLineWidth + whitespaceWidthAtWordStart;
				sf::Vector2f wordMovement(-extendedLineWidth, lineSpacingAtWordStart);
								
				//If a line was in progress and started before the word, finish it before moving on
				if (!reachedCharacterLimit) {
					if (m_style.underlineds.back()) {
						if (underlineStart.x < currentLineWidth) {
							addLine(m_lineVertices, underlineStart, currentLineWidth - underlineStart.x, m_style.fillColors.back(), lineThickness);
							underlineStart.x = extendedLineWidth;
						}
						if (hasOutline && underlineOutlineStart.x < currentLineWidth) {
							addLine(m_lineOutlineVertices, underlineOutlineStart, currentLineWidth - underlineOutlineStart.x, m_style.outlineColors.back(), lineThickness, m_style.outlineThicknesses.back());
							underlineOutlineStart.x = extendedLineWidth;
						}	
					}
					if (m_style.strikeThroughs.back()) {
						if (strikeThroughStart.x < currentLineWidth) {
							addLine(m_lineVertices, strikeThroughStart, currentLineWidth - strikeThroughStart.x, m_style.fillColors.back(), lineThickness);
							strikeThroughStart.x = extendedLineWidth;
						}
						if (hasOutline && strikeThroughOutlineStart.x < currentLineWidth) {
							addLine(m_lineOutlineVertices, strikeThroughOutlineStart, currentLineWidth - strikeThroughOutlineStart.x, m_style.outlineColors.back(), lineThickness, m_style.outlineThicknesses.back());
							strikeThroughOutlineStart.x = extendedLineWidth;
						}
					}
				}
				
				//If any finished line in the word stemmed from before it, cut it in half at the start of the word (one half will stay, the other will move with the word)
				for (size_t i = 0; i < wordLineVertices.getVertexCount(); i += 6) {
					if (wordLineVertices[i].position.x <= currentLineWidth) {
						for (size_t j = 0; j < 6; j++) {
							sf::Vertex v = wordLineVertices[i+j];
							if (j == 1 || j == 4 || j == 5) //Shorten the end of the first half, which will stay on the line
								v.position.x = roundf(currentLineWidth);
							else
								wordLineVertices[i+j].position.x = extendedLineWidth; //Push the beginning of the second, which will go down with the word afterwards
							m_lineVertices.append(v);
						}
					}
					
					for (size_t j = i; j < i+6; j++) {
						wordLineVertices[j].position += wordMovement;
					}
				}
				for (size_t i = 0; i < wordLineOutlineVertices.getVertexCount(); i += 6) {
					if (wordLineOutlineVertices[i].position.x + outlineThicknessAtWordStart <= currentLineWidth) {
						for (size_t j = 0; j < 6; j++) {
							sf::Vertex v = wordLineOutlineVertices[i+j];
							
							if (j == 1 || j == 4 || j == 5)
								v.position.x = roundf(currentLineWidth + outlineThicknessAtWordStart);
							else
								wordLineOutlineVertices[i+j].position.x = extendedLineWidth - outlineThicknessAtWordStart;
							m_lineOutlineVertices.append(v);
						}
					}
					
					for (size_t j = i; j < i+6; j++) {
						wordLineOutlineVertices[j].position += wordMovement;
					}
				}
				
				for (size_t i = 0; i < wordCharVertices.getVertexCount(); i++) {
					wordCharVertices[i].position += wordMovement;
				}
				for (size_t i = 0; i < wordCharOutlineVertices.getVertexCount(); i++) {
					wordCharOutlineVertices[i].position += wordMovement;
				}
				
				pos += wordMovement;
				underlineStart += wordMovement;
				underlineOutlineStart += wordMovement;
				strikeThroughStart += wordMovement;
				strikeThroughOutlineStart += wordMovement;
				
				currentLineWidth = 0;
				currentLine++;
				
				setLineStarts();
				
				if (reachedCharacterLimit)
					shouldStop = true;
			}
			
			break;
		}
		}
		
		if (shouldStop)
			break;
		
		previousChar = m_string[i];
		i++;
	}
	
	if (!reachedCharacterLimit) {
		float excessWhiteSpace = wordCharVertices.getVertexCount() == 0 ? whitespaceWidthAtWordStart : 0;
		if (m_style.underlineds.back()) {
			addLine(wordLineVertices, underlineStart, pos.x - underlineStart.x - excessWhiteSpace, m_style.fillColors.back(), lineThickness);
			if (hasOutline)
				addLine(wordLineOutlineVertices, underlineOutlineStart, pos.x - underlineOutlineStart.x - excessWhiteSpace, m_style.outlineColors.back(), lineThickness, m_style.outlineThicknesses.back());
		}
		if (m_style.strikeThroughs.back()) {
			addLine(wordLineVertices, strikeThroughStart, pos.x - strikeThroughStart.x - excessWhiteSpace, m_style.fillColors.back(), lineThickness);
			if (hasOutline)
				addLine(wordLineOutlineVertices, strikeThroughOutlineStart, pos.x - strikeThroughOutlineStart.x - excessWhiteSpace, m_style.outlineColors.back(), lineThickness, m_style.outlineThicknesses.back());
		}
	}
	
	addWordToText();
	
	roundNewVertices(m_charVertices, startOfNewCharVertices);
	roundNewVertices(m_charOutlineVertices, startOfNewCharOutlineVertices);
	roundNewVertices(m_lineVertices, startOfNewLineVertices);
	roundNewVertices(m_lineOutlineVertices, startOfNewLineOutlineVertices);
	
	//Compute bounds; in a square of 6 vertices, the first one is the upper left and the last one the bottom right
	float minX = std::numeric_limits<float>::infinity(), minY = std::numeric_limits<float>::infinity(),
		  maxX = std::numeric_limits<float>::lowest(),	 maxY = std::numeric_limits<float>::lowest();

	sf::VertexArray const* arrays[4] { &m_charVertices, &m_lineVertices, &m_charOutlineVertices, &m_lineOutlineVertices };
	for (size_t i = 0; i < 4; i++) {
		sf::VertexArray const& a = *arrays[i];
		for (size_t j = 0; j < a.getVertexCount(); j+=6) {
			minX = fminf(minX, a[j].position.x);
			minY = fminf(minY, a[j].position.y);
			maxX = fmaxf(maxX, a[j+5].position.x);
			maxY = fmaxf(maxY, a[j+5].position.y);
		}
	}
	
	m_bounds.top = minY;
	m_bounds.left = minX;
	m_bounds.width = maxX - minX;
	m_bounds.height = maxY - minY;
	
	m_style.rewind();
	
	m_shouldUpdateVertices = false;
}

void RichText::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!m_font)
		return;
	
	states.transform *= getTransform();
	states.texture = &m_font->getTexture(m_characterSize);
	
	if (m_shouldUpdateVertices)
		updateVertices();
	
	if (m_charOutlineVertices.getVertexCount() > 0)
		target.draw(m_charOutlineVertices, states);
	if (m_lineOutlineVertices.getVertexCount() > 0)
		target.draw(m_lineOutlineVertices, states);
	if (m_charVertices.getVertexCount() > 0)
		target.draw(m_charVertices, states);
	if (m_lineVertices.getVertexCount() > 0)
		target.draw(m_lineVertices, states);
}

RichText::VariableStyle::VariableStyle() {
	bolds.push_back(false);
	italics.push_back(false);
	underlineds.push_back(false);
	strikeThroughs.push_back(false);
	fillColors.push_back(sf::Color::Black);
	outlineThicknesses.push_back(0.f);
	outlineColors.push_back(sf::Color::White);
	letterSpacingFactors.push_back(1.f);
	lineSpacingFactors.push_back(1.f);
}

void RichText::VariableStyle::rewind() {
	while (bolds.size() > 1)
		bolds.pop_back();
	while (italics.size() > 1)
		italics.pop_back();
	while (underlineds.size() > 1)
		underlineds.pop_back();
	while (strikeThroughs.size() > 1)
		strikeThroughs.pop_back();
	while (fillColors.size() > 1)
		fillColors.pop_back();
	while (outlineThicknesses.size() > 1)
		outlineThicknesses.pop_back();
	while (outlineColors.size() > 1)
		outlineColors.pop_back();
	while (letterSpacingFactors.size() > 1)
		letterSpacingFactors.pop_back();
	while (lineSpacingFactors.size() > 1)
		lineSpacingFactors.pop_back();
}

template <>
RichText::SpecializedStylizer<bool>::SpecializedStylizer(Stylizer::StyleProperty const& type) : Stylizer (type) {
	switch (type) {
	case Bold:
		m_styleMember = &VariableStyle::bolds; break;
	case Italic:
		m_styleMember = &VariableStyle::italics; break;
	case Underlined:
		m_styleMember = &VariableStyle::underlineds; break;
	case StrikeThrough:
		m_styleMember = &VariableStyle::strikeThroughs; break;
	default:
		break;
	}
}

template <>
RichText::SpecializedStylizer<float>::SpecializedStylizer(Stylizer::StyleProperty const& type) : Stylizer (type) {
	switch (type) {
	case OutlineThickness:
		m_styleMember = &VariableStyle::outlineThicknesses; break;
	case LetterSpacing:
		m_styleMember = &VariableStyle::letterSpacingFactors; break;
	case LineSpacing:
		m_styleMember = &VariableStyle::lineSpacingFactors; break;
	default:
		break;
	}
}

template <>
RichText::SpecializedStylizer<sf::Color>::SpecializedStylizer(Stylizer::StyleProperty const& type) : Stylizer (type) {
	switch (type) {
	case FillColor:
		m_styleMember = &VariableStyle::fillColors; break;
	case OutlineColor:
		m_styleMember = &VariableStyle::outlineColors; break;
	default: break;
	}
}

template<class T>
RichText::EnderStylizer<T>::EnderStylizer(Stylizer::StyleProperty type) : SpecializedStylizer<T>(type) {}

template<class T>
RichText::Stylizer::StyleProperty RichText::EnderStylizer<T>::stylize(VariableStyle& vs) const {
	if ((vs.*this->m_styleMember).size() > 1) {
		T popped = (vs.*this->m_styleMember).back();
		(vs.*this->m_styleMember).pop_back();
		if (popped == (vs.*this->m_styleMember).back())
			return Stylizer::None;
		return this->m_type;
	}
	else
		return Stylizer::None;
}

template<class T>
RichText::StarterStylizer<T>::StarterStylizer(Stylizer::StyleProperty type) : SpecializedStylizer<T>(type), activated(false) {}

template<class T>
RichText::StarterStylizer<T>::StarterStylizer(Stylizer::StyleProperty type, T value) : SpecializedStylizer<T>(type), activated(true), m_value(value) {}

template<class T>
RichText::Stylizer::StyleProperty RichText::StarterStylizer<T>::stylize(VariableStyle& vs) const {
	if (!activated) {
		(vs.*this->m_styleMember).push_back((vs.*this->m_styleMember).back());
		return Stylizer::None;
	}
	else if ((vs.*this->m_styleMember).back() == m_value) {
		(vs.*this->m_styleMember).push_back(m_value);
		return Stylizer::None;
	}
	else {
		(vs.*this->m_styleMember).push_back(m_value);
		return this->m_type;
	}	
}

template<class T>
void RichText::StarterStylizer<T>::setValue(T value) {
	m_value = value;
	activated = true;
}
