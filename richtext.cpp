#include "richtext.h"
#include <stack>
#include <cmath>

RichText::RichText() :
	m_vertices				(sf::Triangles),
	m_lineVertices			(sf::Triangles),
	m_outlineVertices		(sf::Triangles),
	m_lineOutlineVertices	(sf::Triangles),
	m_geometryNeedsUpdate	(false),
	m_partialDisplayVertices(sf::Triangles)
{
}

RichText::RichText(sf::String const& string, sf::Font const& font, unsigned int characterSize) :
	m_string				(string),
	m_font					(&font),
	m_characterSize			(characterSize),
	m_vertices				(sf::Triangles),
	m_lineVertices			(sf::Triangles),
	m_outlineVertices		(sf::Triangles),
	m_lineOutlineVertices	(sf::Triangles),
	m_geometryNeedsUpdate	(true),
	m_partialDisplayVertices(sf::Triangles)
{
}

void RichText::setString(sf::String const& string) {
	m_string = string;
	m_geometryNeedsUpdate = true;
}

void RichText::setFont(sf::Font const& font) {
	if (m_font != &font) {
		m_font = &font;
		m_geometryNeedsUpdate = true;
	}
}

void RichText::setCharacterSize(unsigned int size) {
	m_characterSize = size;
	m_geometryNeedsUpdate = true;
}

void RichText::setDefaultLineSpacing(float spacingFactor) {
	m_dLineSpacingFactor = spacingFactor;
	m_geometryNeedsUpdate = true;
}

void RichText::setDefaultLetterSpacing(float spacingFactor) {
	m_dLetterSpacingFactor = spacingFactor;
	m_geometryNeedsUpdate = true;
}

void RichText::setDefaultFillColor(sf::Color const& color) {
	m_dFillColor = color;
	m_geometryNeedsUpdate = true;
}

void RichText::setDefaultOutlineColor(sf::Color const& color) {
	m_dOutlineColor = color;
	m_geometryNeedsUpdate = true;
}

void RichText::setDefaultOutlineThickness(float thickness) {
	m_dOutlineThickness = thickness;
	m_geometryNeedsUpdate = true;
}

void RichText::setDefaultStyle(sf::Uint32 style) {
	m_dBold = style & sf::Text::Bold;
	m_dItalic = style & sf::Text::Italic;
	m_dUnderlined = style & sf::Text::Underlined;
	m_dStrikeThrough = style & sf::Text::StrikeThrough;
	m_geometryNeedsUpdate = true;
}

void RichText::setHorizontalLimit(float limit) {
	m_horizontalLimit = limit;
	m_geometryNeedsUpdate = true;
}

void RichText::setCharacterLimit(size_t limit) {
	m_characterLimit = limit;
	m_pDVNeedsUpdate = true;
}

sf::String const& RichText::getString() const { return m_string; }

sf::Font const* RichText::getFont() const { return m_font; }

unsigned int RichText::getCharacterSize() const { return m_characterSize; }

float RichText::getBaseLetterSpacing() const { return m_dLetterSpacingFactor; }

float RichText::getBaseLineSpacing() const { return m_dLineSpacingFactor; }

sf::Color const& RichText::getBaseFillColor() const { return m_dFillColor; }

sf::Color const& RichText::getBaseOutlineColor() const { return m_dOutlineColor; }

float RichText::getBaseOutlineThickness() const { return m_dOutlineThickness; }

sf::Uint32 RichText::getDefaultStyle() const {
	sf::Uint32 style = sf::Text::Regular;
	if (m_dBold)
		style &= sf::Text::Bold;
	if (m_dItalic)
		style &= sf::Text::Italic;
	if (m_dUnderlined)
		style &= sf::Text::Underlined;
	if (m_dStrikeThrough)
		style &= sf::Text::StrikeThrough;
	return style;
}

float RichText::getHorizontalLimit() const { return m_horizontalLimit; }

size_t RichText::getCharacterLimit() const { return m_characterLimit; }

sf::FloatRect RichText::findCharacterBounds(size_t index) const {
	size_t i = 6*index;
	if (i >= m_displayableCharacters)
		throw (std::out_of_range("Character doesn't exist"));
	
	return sf::FloatRect(m_vertices[i].position.x,
						 m_vertices[i].position.y,
						 m_vertices[i+5].position.x - m_vertices[i].position.x,
						 m_vertices[i+5].position.y - m_vertices[i].position.y);
}

sf::FloatRect RichText::getLocalBounds() const {
	ensureGeometryUpdate();
	return m_bounds;
}

sf::FloatRect RichText::getGlobalBounds() const {
	return getTransform().transformRect(getLocalBounds());
}

inline float round(float const& f) { return std::floor(f + 0.5f); }

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

	vertices.append(sf::Vertex(sf::Vector2f(round(position.x + left  - italicShear * top    - outlineThickness), round(position.y + top    - outlineThickness)), color, sf::Vector2f(u1, v1)));
	vertices.append(sf::Vertex(sf::Vector2f(round(position.x + right - italicShear * top    - outlineThickness), round(position.y + top    - outlineThickness)), color, sf::Vector2f(u2, v1)));
	vertices.append(sf::Vertex(sf::Vector2f(round(position.x + left  - italicShear * bottom - outlineThickness), round(position.y + bottom - outlineThickness)), color, sf::Vector2f(u1, v2)));
	vertices.append(sf::Vertex(sf::Vector2f(round(position.x + left  - italicShear * bottom - outlineThickness), round(position.y + bottom - outlineThickness)), color, sf::Vector2f(u1, v2)));
	vertices.append(sf::Vertex(sf::Vector2f(round(position.x + right - italicShear * top    - outlineThickness), round(position.y + top    - outlineThickness)), color, sf::Vector2f(u2, v1)));
	vertices.append(sf::Vertex(sf::Vector2f(round(position.x + right - italicShear * bottom - outlineThickness), round(position.y + bottom - outlineThickness)), color, sf::Vector2f(u2, v2)));
}

void addLine(sf::VertexArray& vertices, sf::Vector2f origin, float lineLength, const sf::Color& color, float thickness, float outlineThickness = 0)
{
	float top = std::floor(origin.y - (thickness / 2) + 0.5f);
	float bottom = top + std::floor(thickness + 0.5f);

	vertices.append(sf::Vertex(sf::Vector2f(round(origin.x - outlineThickness),					round(top    - outlineThickness)), color, sf::Vector2f(1, 1)));
	vertices.append(sf::Vertex(sf::Vector2f(round(origin.x + lineLength + outlineThickness),	round(top    - outlineThickness)), color, sf::Vector2f(1, 1)));
	vertices.append(sf::Vertex(sf::Vector2f(round(origin.x - outlineThickness),					round(bottom + outlineThickness)), color, sf::Vector2f(1, 1)));
	vertices.append(sf::Vertex(sf::Vector2f(round(origin.x - outlineThickness),					round(bottom + outlineThickness)), color, sf::Vector2f(1, 1)));
	vertices.append(sf::Vertex(sf::Vector2f(round(origin.x + lineLength + outlineThickness),	round(top    - outlineThickness)), color, sf::Vector2f(1, 1)));
	vertices.append(sf::Vertex(sf::Vector2f(round(origin.x + lineLength + outlineThickness),	round(bottom + outlineThickness)), color, sf::Vector2f(1, 1)));
}

void RichText::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!m_font)
		return;
	
	states.texture = &m_font->getTexture(m_characterSize);
	states.transform *= getTransform();
	
	ensureGeometryUpdate();
	ensurePDVUpdate();
	
	if (m_characterLimit < m_displayableCharacters) {
		if (m_partialDisplayVertices.getVertexCount() > 0) {
			target.draw(m_partialDisplayVertices, states);
		}
	}
	else {
		if (m_outlineVertices.getVertexCount() > 0)
			target.draw(m_outlineVertices, states);
		if (m_lineOutlineVertices.getVertexCount() > 0)
			target.draw(m_lineOutlineVertices, states);
		if (m_vertices.getVertexCount() > 0)
			target.draw(m_vertices, states);
		if (m_lineVertices.getVertexCount() > 0)
			target.draw(m_lineVertices, states);
	}
}

void RichText::ensureGeometryUpdate() const {
	if (!m_font || !m_geometryNeedsUpdate)
		return;
	m_pDVNeedsUpdate = true;
	
	m_vertices.clear();
	m_lineVertices.clear();
	m_outlineVertices.clear();
	m_lineOutlineVertices.clear();
	m_displayedLineCorrespondingToLine.clear();
	m_displayedCharacterCorrespondingToOutline.clear();
	m_displayedLineCorrespondingToLineOutline.clear();
	m_lastDisplayedCharacterInLine.clear();
	
	bool bold = m_dBold;
	bool italic = m_dItalic;
	bool underlined = m_dUnderlined;
	bool strikeThrough = m_dStrikeThrough;
	
	std::stack<sf::Color> fillColors;
	fillColors.push(m_dFillColor);
	std::stack<sf::Color> outlineColors;
	outlineColors.push(m_dOutlineColor);
	std::stack<float> outlineThicknesses;
	outlineThicknesses.push(m_dOutlineThickness);
	
	float whitespaceWidth = m_font->getGlyph(L' ', m_characterSize, false).advance;
	std::stack<float> letterSpacings;
	letterSpacings.push((whitespaceWidth / 3.f) * (m_dLetterSpacingFactor - 1.f));
	whitespaceWidth += letterSpacings.top();
	std::stack<float> lineSpacings;
	lineSpacings.push(m_font->getLineSpacing(m_characterSize) * m_dLineSpacingFactor);
	
	bool hasOutline = outlineThicknesses.top() != 0.f;
	
	sf::Vector2f pos(0, static_cast<float>(m_characterSize));
	
	float lineThickness = m_font->getUnderlineThickness(m_characterSize);
	sf::Vector2f underlineStart(pos.x, pos.y + m_font->getUnderlinePosition(m_characterSize));
	sf::Vector2f underlineOutlineStart = underlineStart;
	sf::FloatRect xBounds = m_font->getGlyph(L'x', m_characterSize, false).bounds;
	sf::Vector2f strikeThroughStart(pos.x, pos.y + xBounds.top + xBounds.height * 0.4f);
	sf::Vector2f strikeThroughOutlineStart = strikeThroughStart;
	
	sf::Uint32 prevChar = 0;
	
	sf::VertexArray wordVertices(sf::Triangles);
	sf::VertexArray wordLineVertices(sf::Triangles);
	sf::VertexArray wordOutlineVertices(sf::Triangles);
	sf::VertexArray wordLineOutlineVertices(sf::Triangles); //Quite a mouthful.
	
	float currentLineWidth = 0;
	float lineSpacingAtWordStart = lineSpacings.top();
	float outlineThicknessAtWordStart = outlineThicknesses.top();
	float whitespaceWidthAtWordStart = whitespaceWidth;
	
	size_t currentDisplayedCharacter = 0;
	size_t currentLine = 0;
	
	size_t i = 0;
	size_t len = m_string.getSize();
	while (i < len) {	
		switch (m_string[i]) {
		case '<': {
			size_t end = m_string.find('>', i);
			if (end == std::string::npos) { //Non-closed tag
				i = len;
				continue;
			}
			std::string tags = m_string.substring(i+1, end-i-1);
			size_t tagStart = 0;
			size_t tagEnd;
			do {
				tagEnd = tags.find(',', tagStart);
				
				std::string fullTag = tags.substr(tagStart, tagEnd - tagStart);		
				tagStart = tagEnd + 1;
				
				if (fullTag.size() == 0)
					continue;
				
				bool ending = (fullTag[0] == '/');
				if (ending)
					fullTag = fullTag.substr(1);
				if (fullTag.size() == 1) { //char tag with no argument
					switch (fullTag[0]) {
					case 'b':
						bold = !ending;
						break;
					case 'i':
						italic = !ending;
						break;
					case 'u':
						if (underlined && ending) {
							addLine(wordLineVertices, underlineStart, pos.x - underlineStart.x, fillColors.top(), lineThickness);
							if (hasOutline) {
								addLine(wordLineOutlineVertices, underlineOutlineStart, pos.x - underlineOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
							}
						}
						underlined = !ending;
						if (underlined) {
							underlineStart.x = pos.x;
							if (hasOutline)
								underlineOutlineStart.x = pos.x;
						}
						break;
					case 's':
						if (strikeThrough && ending) {
							addLine(wordLineVertices, strikeThroughStart, pos.x - strikeThroughStart.x, fillColors.top(), lineThickness);
							if (hasOutline) {
								addLine(wordLineOutlineVertices, strikeThroughOutlineStart, pos.x - strikeThroughOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
							}
						}
						strikeThrough = !ending;
						if (strikeThrough) {
							strikeThroughStart.x = pos.x;
							if (hasOutline)
								strikeThroughOutlineStart.x = pos.x;
						}
						break;
					case 'c':
						if (ending && fillColors.size() > 1) {
							if (underlined) {
								addLine(wordLineVertices, underlineStart, pos.x - underlineStart.x, fillColors.top(), lineThickness);
								underlineStart.x = pos.x;
							}
							
							if (strikeThrough) {
								addLine(wordLineVertices, strikeThroughStart, pos.x - strikeThroughStart.x, fillColors.top(), lineThickness);
								strikeThroughStart.x = pos.x;
							}
							
							fillColors.pop();
						}
						break;
					default:
						break;
					}
					
					continue;
				}
				
				
				size_t argPos = fullTag.find('=');
				if (argPos == std::string::npos) { //string tag with no argument
					if (ending && fullTag.compare("oc") == 0 && outlineColors.size() > 1) {
						if (hasOutline) {
							if (underlined) {
								addLine(wordLineOutlineVertices, underlineOutlineStart, pos.x - underlineOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
								underlineOutlineStart.x = pos.x;
							}
							if (strikeThrough) {
								addLine(wordLineOutlineVertices, strikeThroughOutlineStart, pos.x - strikeThroughOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
								strikeThroughOutlineStart.x = pos.x;
							}
						}
						outlineColors.pop();
					}
					else if (ending && fullTag.compare("lns") == 0 && letterSpacings.size() > 1)
						letterSpacings.pop();
					else if (ending && fullTag.compare("lts") == 0 && lineSpacings.size() > 1)
						lineSpacings.pop();
					else if (ending && fullTag.compare("ot") == 0 && outlineThicknesses.size() > 1) {
						if (underlined) {
							addLine(wordLineOutlineVertices, underlineOutlineStart, pos.x - underlineOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
							underlineOutlineStart.x = pos.x;
						}
						if (strikeThrough) {
							addLine(wordLineOutlineVertices, strikeThroughOutlineStart, pos.x - strikeThroughOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
							strikeThroughOutlineStart.x = pos.x;
						}
						outlineThicknesses.pop();
						hasOutline = outlineThicknesses.top() != 0.f;
					}
					continue;
				}
				
				
				if (argPos + 1 >= fullTag.size()) //Empty argument
					continue;
				
				std::string tag = fullTag.substr(0, argPos);
				std::string arg = fullTag.substr(argPos+1);
				
				if (argPos == 1 || (ending && argPos == 2)) { //char tag with argument
					switch(tag[0]) {
					case 'c': {
						if (underlined) {
							addLine(wordLineVertices, underlineStart, pos.x - underlineStart.x, fillColors.top(), lineThickness);
							underlineStart.x = pos.x;
						}
						
						if (strikeThrough) {
							addLine(wordLineVertices, strikeThroughStart, pos.x - strikeThroughStart.x, fillColors.top(), lineThickness);
							strikeThroughStart.x = pos.x;
						}
						if (arg[0] == '#') { //Hexcode color
							long fullCode = 0;
							size_t j = 0;
							try {
								fullCode = std::stol(arg.substr(1), &j, 16);
							} catch (...) {
								continue;
							}
							if (j == 6) { //No transparency value
								fillColors.push(sf::Color((fullCode >> 16) & 0xFF, (fullCode >> 8) & 0xFF, fullCode & 0xFF));
							}
							else if (j == 8) {
								fillColors.push(sf::Color((fullCode >> 24) & 0xFF, (fullCode >> 16) & 0xFF, (fullCode >> 8) & 0xFF, fullCode & 0xFF));
							}
						}
						else { //String code color
						}
						break;
					}
					default:
						break;
					}
				}
				else { //string tag with argument
					if (tag.compare("oc") == 0) {
						if (hasOutline) {
							if (underlined) {
								addLine(wordLineOutlineVertices, underlineOutlineStart, pos.x - underlineOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
								underlineOutlineStart.x = pos.x;
							}
							if (strikeThrough) {
								addLine(wordLineOutlineVertices, strikeThroughOutlineStart, pos.x - strikeThroughOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
								strikeThroughOutlineStart.x = pos.x;
							}
						}
						if (arg[0] == '#') { //Hexcode color
							long fullCode = 0;
							size_t j = 0;
							try {
								fullCode = std::stol(arg.substr(1), &j, 16);
							} catch (...) {
								continue;
							}
							if (j == 6) { //No transparency value
								outlineColors.push(sf::Color((fullCode >> 16) & 0xFF, (fullCode >> 8) & 0xFF, fullCode & 0xFF));
							}
							else if (j == 8) {
								outlineColors.push(sf::Color((fullCode >> 24) & 0xFF, (fullCode >> 16) & 0xFF, (fullCode >> 8) & 0xFF, fullCode & 0xFF));
							}
						}
						else { //String code color
						}
					}
					else if (tag.compare("lts") == 0) {
						float factor = 0;
						size_t j = 0;
						try {
							factor = std::stof(arg, &j);
						} catch (...) {
							continue;
						}
						if (j == arg.size()) {
							whitespaceWidth = m_font->getGlyph(L' ', m_characterSize, false).advance;
							letterSpacings.push((whitespaceWidth / 3.f) * (factor - 1.f));
							whitespaceWidth += letterSpacings.top();
						}
					}
					else if (tag.compare("lns") == 0) {
						float factor = 0;
						size_t j = 0;
						try {
							factor = std::stof(arg, &j);
						} catch (...) {
							continue;
						}
						if (j == arg.size()) {
							lineSpacings.push(m_font->getLineSpacing(m_characterSize) * factor);
						}
					}
					else if (tag.compare("ot") == 0) {
						float t = 0;
						size_t j = 0;
						try {
							t = std::stof(arg, &j);
						} catch (...) {
							continue;
						}
						if (j == arg.size()) {
							if (hasOutline) {
								if (underlined) {
									addLine(wordLineOutlineVertices, underlineOutlineStart, pos.x - underlineOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
								}
								if (strikeThrough) {
									addLine(wordLineOutlineVertices, strikeThroughOutlineStart, pos.x - strikeThroughOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
								}
							}
							
							outlineThicknesses.push(t);
							
							hasOutline = t != 0.f;
							if (hasOutline) {
								if (underlined)
									underlineOutlineStart.x = pos.x;
								if (strikeThrough)
									strikeThroughOutlineStart.x = pos.x;
							}
						}
					}
				}
				
			} while(tagEnd != std::string::npos);
			
			i = end;
			break;
		}
		case ' ': {			
			for (size_t i = 0; i < wordVertices.getVertexCount(); i++)
				m_vertices.append(wordVertices[i]);
			wordVertices.clear();
			for (size_t i = 0; i < wordLineVertices.getVertexCount(); i+=6) {
				m_displayedLineCorrespondingToLine.push_back(currentLine);
				for (size_t j = i; j < i+6; j++)
					m_lineVertices.append(wordLineVertices[j]);
			}
			wordLineVertices.clear();
			for (size_t i = 0; i < wordOutlineVertices.getVertexCount(); i++)
				m_outlineVertices.append(wordOutlineVertices[i]);
			wordOutlineVertices.clear();
			for (size_t i = 0; i < wordLineOutlineVertices.getVertexCount(); i+=6) {
				m_displayedLineCorrespondingToLineOutline.push_back(currentLine);
				for (size_t j = i; j < i+6; j++)
					m_lineOutlineVertices.append(wordLineOutlineVertices[j]);
			}
			wordLineOutlineVertices.clear();
			
			currentLineWidth = pos.x;
			
			pos.x += whitespaceWidth;
			lineSpacingAtWordStart = lineSpacings.top();
			outlineThicknessAtWordStart = outlineThicknesses.top();
			whitespaceWidthAtWordStart = whitespaceWidth;
			break;
		}
		case '\t': {
			for (size_t i = 0; i < wordVertices.getVertexCount(); i++)
				m_vertices.append(wordVertices[i]);
			wordVertices.clear();
			for (size_t i = 0; i < wordLineVertices.getVertexCount(); i+=6) {
				m_displayedLineCorrespondingToLine.push_back(currentLine);
				for (size_t j = i; j < i+6; j++)
					m_lineVertices.append(wordLineVertices[j]);
			}
			wordLineVertices.clear();
			for (size_t i = 0; i < wordOutlineVertices.getVertexCount(); i++)
				m_outlineVertices.append(wordOutlineVertices[i]);
			wordOutlineVertices.clear();
			for (size_t i = 0; i < wordLineOutlineVertices.getVertexCount(); i+=6) {
				m_displayedLineCorrespondingToLineOutline.push_back(currentLine);
				for (size_t j = i; j < i+6; j++)
					m_lineOutlineVertices.append(wordLineOutlineVertices[j]);
			}
			wordLineOutlineVertices.clear();
			
			currentLineWidth = pos.x;
			
			pos.x += whitespaceWidth*8;
			pos.x -= fmodf(pos.x, whitespaceWidth*8);
			lineSpacingAtWordStart = lineSpacings.top();
			outlineThicknessAtWordStart = outlineThicknesses.top();
			whitespaceWidthAtWordStart = whitespaceWidth*4;
			break;
		}
		case '\n': {
			for (size_t i = 0; i < wordVertices.getVertexCount(); i++)
				m_vertices.append(wordVertices[i]);
			wordVertices.clear();
			for (size_t i = 0; i < wordLineVertices.getVertexCount(); i+=6) {
				m_displayedLineCorrespondingToLine.push_back(currentLine);
				for (size_t j = i; j < i+6; j++)
					m_lineVertices.append(wordLineVertices[j]);
			}
			wordLineVertices.clear();
			for (size_t i = 0; i < wordOutlineVertices.getVertexCount(); i++)
				m_outlineVertices.append(wordOutlineVertices[i]);
			wordOutlineVertices.clear();
			for (size_t i = 0; i < wordLineOutlineVertices.getVertexCount(); i+=6) {
				m_displayedLineCorrespondingToLineOutline.push_back(currentLine);
				for (size_t j = i; j < i+6; j++)
					m_lineOutlineVertices.append(wordLineOutlineVertices[j]);
			}
			wordLineOutlineVertices.clear();
			
			if (underlined) {
				addLine(m_lineVertices, underlineStart, pos.x - underlineStart.x, fillColors.top(), lineThickness);
				m_displayedLineCorrespondingToLine.push_back(currentLine);
				if (hasOutline) {
					addLine(m_lineOutlineVertices, underlineOutlineStart, pos.x - underlineOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
					m_displayedLineCorrespondingToLineOutline.push_back(currentLine);
				}
			}
			if (strikeThrough) {
				addLine(m_lineVertices, strikeThroughStart, pos.x - strikeThroughStart.x, fillColors.top(), lineThickness);
				m_displayedLineCorrespondingToLine.push_back(currentLine);
				if (hasOutline) {
					addLine(m_lineOutlineVertices, strikeThroughOutlineStart, pos.x - strikeThroughOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
					m_displayedLineCorrespondingToLineOutline.push_back(currentLine);
				}
			}
			pos.x = 0;
			pos.y += lineSpacings.top();
			
			underlineStart.x = 0;
			underlineStart.y += lineSpacings.top();
			underlineOutlineStart = underlineStart;
			strikeThroughStart.x = 0;
			strikeThroughStart.y += lineSpacings.top();
			strikeThroughOutlineStart = strikeThroughStart;
			
			m_lastDisplayedCharacterInLine.push_back(currentDisplayedCharacter);
			currentLine++;
			currentLineWidth = 0;
			
			lineSpacingAtWordStart = lineSpacings.top();
			outlineThicknessAtWordStart = outlineThicknesses.top();
			whitespaceWidthAtWordStart = whitespaceWidth;
			break;
		}
		case '\r':
			break;
		default: {			
			pos.x += m_font->getKerning(prevChar, m_string[i], m_characterSize);
			
			if (outlineThicknesses.top() != 0.f) {
				m_displayedCharacterCorrespondingToOutline.push_back(currentDisplayedCharacter);
				sf::Glyph const& go = m_font->getGlyph(m_string[i], m_characterSize, bold, outlineThicknesses.top());
				addGlyphQuad(wordOutlineVertices, pos, outlineColors.top(), go, italic ? 0.209f : 0, outlineThicknesses.top());
			}
			sf::Glyph const& g = m_font->getGlyph(m_string[i], m_characterSize, bold);
			addGlyphQuad(wordVertices, pos, fillColors.top(), g, italic ? 0.209f : 0);
			
			pos.x += g.advance + letterSpacings.top();
			currentDisplayedCharacter += 1;
			
			
			//Move the word down a line if it became too long
			if (currentLineWidth != 0.f && pos.x > m_horizontalLimit) {
				float extendedLineWidth = currentLineWidth + whitespaceWidthAtWordStart;
				sf::Vector2f wordMovement(-extendedLineWidth, lineSpacingAtWordStart);
								
				//If a line was in progress and started before the word, finish it before moving on
				if (underlined && underlineStart.x < currentLineWidth) {
					addLine(m_lineVertices, underlineStart, currentLineWidth - underlineStart.x, fillColors.top(), lineThickness);
					m_displayedLineCorrespondingToLine.push_back(currentLine);
					underlineStart.x = extendedLineWidth;
				}
				if (hasOutline && underlined && underlineOutlineStart.x < currentLineWidth) {
					addLine(m_lineOutlineVertices, underlineOutlineStart, currentLineWidth - underlineOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
					m_displayedLineCorrespondingToLineOutline.push_back(currentLine);
					underlineOutlineStart.x = extendedLineWidth;
				}
				if (strikeThrough && strikeThroughStart.x < currentLineWidth) {
					addLine(m_lineVertices, strikeThroughStart, currentLineWidth - strikeThroughStart.x, fillColors.top(), lineThickness);
					m_displayedLineCorrespondingToLine.push_back(currentLine);
					strikeThroughStart.x = extendedLineWidth;
				}
				if (hasOutline && strikeThrough && strikeThroughOutlineStart.x < currentLineWidth) {
					addLine(m_lineOutlineVertices, strikeThroughOutlineStart, currentLineWidth - strikeThroughOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
					m_displayedLineCorrespondingToLineOutline.push_back(currentLine);
					strikeThroughOutlineStart.x = extendedLineWidth;
				}
				
				//If any finished line in the word stemmed from before it, cut it in half at the start of the word (one half will stay, the other will move with the word)
				for (size_t i = 0; i < wordLineVertices.getVertexCount(); i += 6) {
					if (wordLineVertices[i].position.x <= currentLineWidth) {
						for (size_t j = 0; j < 6; j++) {
							sf::Vertex v = wordLineVertices[i+j];
							if (j == 1 || j == 4 || j == 5) //Shorten the end of the first half, which will stay on the line
								v.position.x = currentLineWidth;
							else
								wordLineVertices[i+j].position.x = extendedLineWidth; //Push the beginning of the second, which will go down with the word afterwards
							m_lineVertices.append(v);
						}
						m_displayedLineCorrespondingToLine.push_back(currentLine);
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
								v.position.x = currentLineWidth + outlineThicknessAtWordStart;
							else
								wordLineOutlineVertices[i+j].position.x = extendedLineWidth - outlineThicknessAtWordStart;
							m_lineOutlineVertices.append(v);
						}
						m_displayedLineCorrespondingToLineOutline.push_back(currentLine);
					}
					
					for (size_t j = i; j < i+6; j++) {
						wordLineOutlineVertices[j].position += wordMovement;
					}
				}
				
				for (size_t i = 0; i < wordVertices.getVertexCount(); i++) {
					wordVertices[i].position += wordMovement;
				}
				for (size_t i = 0; i < wordOutlineVertices.getVertexCount(); i++) {
					wordOutlineVertices[i].position += wordMovement;
				}
				
				pos += wordMovement;
				underlineStart += wordMovement;
				underlineOutlineStart += wordMovement;
				strikeThroughStart += wordMovement;
				strikeThroughOutlineStart += wordMovement;
				
				m_lastDisplayedCharacterInLine.push_back(currentDisplayedCharacter - wordVertices.getVertexCount() / 6);
				currentLine++;
				
				currentLineWidth = 0;
			}
			
			break;
		}
		}
		
		i++;
	}
	
	//Finish any unfinished lines
	if (underlined) {
		addLine(m_lineVertices, underlineStart, pos.x - underlineStart.x, fillColors.top(), lineThickness);
		m_displayedLineCorrespondingToLine.push_back(currentLine);
		if (hasOutline) {
			addLine(m_lineOutlineVertices, underlineOutlineStart, pos.x - underlineOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
			m_displayedLineCorrespondingToLineOutline.push_back(currentLine);
		}
	}
	if (strikeThrough) {
		addLine(m_lineVertices, strikeThroughStart, pos.x - strikeThroughStart.x, fillColors.top(), lineThickness);
		m_displayedLineCorrespondingToLine.push_back(currentLine);
		if (hasOutline) {
			addLine(m_lineOutlineVertices, strikeThroughOutlineStart, pos.x - strikeThroughOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
			m_displayedLineCorrespondingToLineOutline.push_back(currentLine);
		}
	}
	
	//Add the last word
	for (size_t i = 0; i < wordVertices.getVertexCount(); i++)
		m_vertices.append(wordVertices[i]);
	for (size_t i = 0; i < wordLineVertices.getVertexCount(); i+=6) {
		m_displayedLineCorrespondingToLine.push_back(currentLine);
		for (size_t j = i; j < i+6; j++)
			m_lineVertices.append(wordLineVertices[j]);
	}
	for (size_t i = 0; i < wordOutlineVertices.getVertexCount(); i++)
		m_outlineVertices.append(wordOutlineVertices[i]);
	for (size_t i = 0; i < wordLineOutlineVertices.getVertexCount(); i+=6) {
		m_displayedLineCorrespondingToLineOutline.push_back(currentLine);
		for (size_t j = i; j < i+6; j++)
			m_lineOutlineVertices.append(wordLineOutlineVertices[j]);
	}
	
	//Compute bounds; in a square of 6 vertices, the first one is the upper left and the last one the bottom right
	float minX = std::numeric_limits<float>::infinity(), minY = std::numeric_limits<float>::infinity(),
		  maxX = std::numeric_limits<float>::lowest(),	 maxY = std::numeric_limits<float>::lowest();

	for (size_t i = 0; i < m_vertices.getVertexCount(); i+=6) {
		minX = fminf(minX, m_vertices[i].position.x);
		minY = fminf(minY, m_vertices[i].position.y);
		maxX = fmaxf(maxX, m_vertices[i+5].position.x);
		maxY = fmaxf(maxY, m_vertices[i+5].position.y);
	}
	for (size_t i = 0; i < m_lineVertices.getVertexCount(); i+=6) {
		minX = fminf(minX, m_lineVertices[i].position.x);
		minY = fminf(minY, m_lineVertices[i].position.y);
		maxX = fmaxf(maxX, m_lineVertices[i+5].position.x);
		maxY = fmaxf(maxY, m_lineVertices[i+5].position.y);
	}
	for (size_t i = 0; i < m_outlineVertices.getVertexCount(); i+=6) {
		minX = fminf(minX, m_outlineVertices[i].position.x);
		minY = fminf(minY, m_outlineVertices[i].position.y);
		maxX = fmaxf(maxX, m_outlineVertices[i+5].position.x);
		maxY = fmaxf(maxY, m_outlineVertices[i+5].position.y);
	}
	for (size_t i = 0; i < m_lineOutlineVertices.getVertexCount(); i+=6) {
		minX = fminf(minX, m_lineOutlineVertices[i].position.x);
		minY = fminf(minY, m_lineOutlineVertices[i].position.y);
		maxX = fmaxf(maxX, m_lineOutlineVertices[i+5].position.x);
		maxY = fmaxf(maxY, m_lineOutlineVertices[i+5].position.y);
	}
	
	m_bounds.top = minY;
	m_bounds.left = minX;
	m_bounds.width = maxX - minX;
	m_bounds.height = maxY - minY;
	
	m_displayableCharacters = currentDisplayedCharacter;
	
	m_geometryNeedsUpdate = false;
}

void RichText::ensurePDVUpdate() const {
	if (!m_font || !m_pDVNeedsUpdate)
		return;
	
	m_partialDisplayVertices.clear();
	
	if (m_characterLimit >= m_displayableCharacters)
		return;
	
	for (size_t i = 0; i < m_outlineVertices.getVertexCount(); i+=6) {
		if (m_displayedCharacterCorrespondingToOutline[i/6] < m_characterLimit) {
			for (size_t j = i; j < i+6; j++)
				m_partialDisplayVertices.append(m_outlineVertices[j]);
		}
		else
			break;
	}
	
	//Determine the line of the character limit
	size_t line = 0;
	while (m_lastDisplayedCharacterInLine[line] < m_characterLimit)
		line++;
	
	float rightLimit = m_vertices[(m_characterLimit-1)*6+5].position.x;
	float addedLineWidth = 1.f;
	
	for (size_t i = 0; i < m_lineOutlineVertices.getVertexCount(); i+=6) {
		size_t lineLine = m_displayedLineCorrespondingToLineOutline[i/6];
		if (lineLine < line) {
			for (size_t j = i; j < i+6; j++)
				m_partialDisplayVertices.append(m_lineOutlineVertices[j]);
		}
		else if (lineLine == line && m_lineOutlineVertices[i].position.x < rightLimit) {
			for (size_t j = i; j < i+6; j++)
				m_partialDisplayVertices.append(m_lineOutlineVertices[j]);
			float thickness = std::floor((m_lineOutlineVertices[i+5].position.y - m_lineOutlineVertices[i].position.y) / 2);
			if (m_lineOutlineVertices[i+5].position.x > rightLimit) {
				size_t k = m_partialDisplayVertices.getVertexCount();
				m_partialDisplayVertices[k-5].position.x = rightLimit + thickness + addedLineWidth;
				m_partialDisplayVertices[k-2].position.x = rightLimit + thickness + addedLineWidth;
				m_partialDisplayVertices[k-1].position.x = rightLimit + thickness + addedLineWidth;
			}
		}
	}
	
	for (size_t i = 0; i < m_characterLimit; i++) {
		for (size_t j = 6*i; j < 6*i+6; j++) {
			m_partialDisplayVertices.append(m_vertices[j]);
		}
	}
	
	for (size_t i = 0; i < m_lineVertices.getVertexCount(); i+=6) {
		size_t lineLine = m_displayedLineCorrespondingToLine[i/6];
		if (lineLine < line) {
			for (size_t j = i; j < i+6; j++)
				m_partialDisplayVertices.append(m_lineVertices[j]);
		}
		else if (lineLine == line && m_lineVertices[i].position.x < rightLimit) {
			for (size_t j = i; j < i+6; j++)
				m_partialDisplayVertices.append(m_lineVertices[j]);
			if (lineLine == line && m_lineVertices[i+5].position.x > rightLimit) {
				size_t k = m_partialDisplayVertices.getVertexCount();
				m_partialDisplayVertices[k-5].position.x = rightLimit + addedLineWidth;
				m_partialDisplayVertices[k-2].position.x = rightLimit + addedLineWidth;
				m_partialDisplayVertices[k-1].position.x = rightLimit + addedLineWidth;
			}
		}
	}	
	
	m_pDVNeedsUpdate = false;
}
