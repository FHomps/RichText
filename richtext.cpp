#include "richtext.h"
#include <stack>
#include <cmath>

RichText::RichText() :
	m_vertices				(sf::Triangles),
	m_lineVertices			(sf::Triangles),
	m_outlineVertices		(sf::Triangles),
	m_geometryNeedsUpdate	(false)
{
}

RichText::RichText(sf::String const& string, sf::Font const& font, unsigned int characterSize) :
	m_string				(string),
	m_font					(&font),
	m_characterSize			(characterSize),
	m_vertices				(sf::Triangles),
	m_lineVertices			(sf::Triangles),
	m_outlineVertices		(sf::Triangles),
	m_geometryNeedsUpdate	(true)
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

void RichText::setCharacterLimit(size_t limit, bool ignoreTags) {
	m_characterLimit = limit;
	m_characterLimitIgnoresTags = ignoreTags;
	m_geometryNeedsUpdate = true;
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
	if (i > m_vertices.getVertexCount())
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

	vertices.append(sf::Vertex(sf::Vector2f(position.x + left  - italicShear * top    - outlineThickness, position.y + top    - outlineThickness), color, sf::Vector2f(u1, v1)));
	vertices.append(sf::Vertex(sf::Vector2f(position.x + right - italicShear * top    - outlineThickness, position.y + top    - outlineThickness), color, sf::Vector2f(u2, v1)));
	vertices.append(sf::Vertex(sf::Vector2f(position.x + left  - italicShear * bottom - outlineThickness, position.y + bottom - outlineThickness), color, sf::Vector2f(u1, v2)));
	vertices.append(sf::Vertex(sf::Vector2f(position.x + left  - italicShear * bottom - outlineThickness, position.y + bottom - outlineThickness), color, sf::Vector2f(u1, v2)));
	vertices.append(sf::Vertex(sf::Vector2f(position.x + right - italicShear * top    - outlineThickness, position.y + top    - outlineThickness), color, sf::Vector2f(u2, v1)));
	vertices.append(sf::Vertex(sf::Vector2f(position.x + right - italicShear * bottom - outlineThickness, position.y + bottom - outlineThickness), color, sf::Vector2f(u2, v2)));
}

void addLine(sf::VertexArray& vertices, sf::Vector2f origin, float lineLength, const sf::Color& color, float thickness, float outlineThickness = 0)
{
	float top = std::floor(origin.y - (thickness / 2) + 0.5f);
	float bottom = top + std::floor(thickness + 0.5f);

	vertices.append(sf::Vertex(sf::Vector2f(origin.x - outlineThickness,				top    - outlineThickness), color, sf::Vector2f(1, 1)));
	vertices.append(sf::Vertex(sf::Vector2f(origin.x + lineLength + outlineThickness,	top    - outlineThickness), color, sf::Vector2f(1, 1)));
	vertices.append(sf::Vertex(sf::Vector2f(origin.x - outlineThickness,				bottom + outlineThickness), color, sf::Vector2f(1, 1)));
	vertices.append(sf::Vertex(sf::Vector2f(origin.x - outlineThickness,				bottom + outlineThickness), color, sf::Vector2f(1, 1)));
	vertices.append(sf::Vertex(sf::Vector2f(origin.x + lineLength + outlineThickness,	top    - outlineThickness), color, sf::Vector2f(1, 1)));
	vertices.append(sf::Vertex(sf::Vector2f(origin.x + lineLength + outlineThickness,	bottom + outlineThickness), color, sf::Vector2f(1, 1)));
}

void RichText::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!m_font)
		return;
	
	states.texture = &m_font->getTexture(m_characterSize);
	states.transform *= getTransform();
	
	ensureGeometryUpdate();
	
	target.draw(m_outlineVertices, states);
	target.draw(m_vertices, states);
	target.draw(m_lineVertices, states);
}

void RichText::ensureGeometryUpdate() const {
	if (!m_font || !m_geometryNeedsUpdate)
		return;
	
	m_vertices.clear();
	m_lineVertices.clear();
	m_outlineVertices.clear();
	
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
	sf::VertexArray wordLineOutlineVertices(sf::Triangles); //Will be fused with m_outlineVertices. Quite a mouthful.
	
	float currentLineWidth = 0;
	float lineSpacingAtWordStart = lineSpacings.top();
	float outlineThicknessAtWordStart = outlineThicknesses.top();
	float whitespaceWidthAtWordStart = whitespaceWidth;
	
	size_t i = 0;
	size_t i_displayOnly = 0;
	size_t len = m_string.getSize();
	while (i < len && (m_characterLimitIgnoresTags ? i_displayOnly : i) < m_characterLimit) {	
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
							if (hasOutline)
								addLine(wordLineOutlineVertices, underlineOutlineStart, pos.x - underlineOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
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
							if (hasOutline)
								addLine(wordLineOutlineVertices, strikeThroughOutlineStart, pos.x - strikeThroughOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
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
							letterSpacings.push((whitespaceWidth / 3.f) * (factor - 1.f));
							whitespaceWidth = m_font->getGlyph(L' ', m_characterSize, false).advance + letterSpacings.top();
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
			for (size_t i = 0; i < wordLineVertices.getVertexCount(); i++)
				m_lineVertices.append(wordLineVertices[i]);
			wordLineVertices.clear();
			for (size_t i = 0; i < wordOutlineVertices.getVertexCount(); i++)
				m_outlineVertices.append(wordOutlineVertices[i]);
			wordOutlineVertices.clear();
			for (size_t i = 0; i < wordLineOutlineVertices.getVertexCount(); i++)
				m_outlineVertices.append(wordLineOutlineVertices[i]);
			wordLineOutlineVertices.clear();
			
			pos.x += whitespaceWidth;
			currentLineWidth = pos.x;
			lineSpacingAtWordStart = lineSpacings.top();
			outlineThicknessAtWordStart = outlineThicknesses.top();
			whitespaceWidthAtWordStart = whitespaceWidth;
			
			i_displayOnly++;
			break;
		}
		case '\t':
			for (size_t i = 0; i < wordVertices.getVertexCount(); i++)
				m_vertices.append(wordVertices[i]);
			wordVertices.clear();
			for (size_t i = 0; i < wordLineVertices.getVertexCount(); i++)
				m_lineVertices.append(wordLineVertices[i]);
			wordLineVertices.clear();
			for (size_t i = 0; i < wordOutlineVertices.getVertexCount(); i++)
				m_outlineVertices.append(wordOutlineVertices[i]);
			wordOutlineVertices.clear();
			for (size_t i = 0; i < wordLineOutlineVertices.getVertexCount(); i++)
				m_outlineVertices.append(wordLineOutlineVertices[i]);
			wordLineOutlineVertices.clear();
			
			pos.x += whitespaceWidth*8;
			pos.x -= fmodf(pos.x, whitespaceWidth*8);
			currentLineWidth = pos.x;
			lineSpacingAtWordStart = lineSpacings.top();
			outlineThicknessAtWordStart = outlineThicknesses.top();
			whitespaceWidthAtWordStart = whitespaceWidth*4;
			
			i_displayOnly++;
			break;
		case '\n':
			for (size_t i = 0; i < wordVertices.getVertexCount(); i++)
				m_vertices.append(wordVertices[i]);
			wordVertices.clear();
			for (size_t i = 0; i < wordLineVertices.getVertexCount(); i++)
				m_lineVertices.append(wordLineVertices[i]);
			wordLineVertices.clear();
			for (size_t i = 0; i < wordOutlineVertices.getVertexCount(); i++)
				m_outlineVertices.append(wordOutlineVertices[i]);
			wordOutlineVertices.clear();
			for (size_t i = 0; i < wordLineOutlineVertices.getVertexCount(); i++)
				m_outlineVertices.append(wordLineOutlineVertices[i]);
			wordLineOutlineVertices.clear();
			
			if (underlined) {
				addLine(m_vertices, underlineStart, pos.x - underlineStart.x, fillColors.top(), lineThickness);
				if (hasOutline)
					addLine(m_outlineVertices, underlineOutlineStart, pos.x - underlineOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
			}
			if (strikeThrough) {
				addLine(m_vertices, strikeThroughStart, pos.x - strikeThroughStart.x, fillColors.top(), lineThickness);
				if (hasOutline)
					addLine(m_outlineVertices, strikeThroughOutlineStart, pos.x - strikeThroughOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
			}
			pos.x = 0;
			pos.y += lineSpacings.top();
			
			underlineStart.x = 0;
			underlineStart.y += lineSpacings.top();
			underlineOutlineStart = underlineStart;
			strikeThroughStart.x = 0;
			strikeThroughStart.y += lineSpacings.top();
			strikeThroughOutlineStart = strikeThroughStart;
			
			lineSpacingAtWordStart = lineSpacings.top();
			outlineThicknessAtWordStart = outlineThicknesses.top();
			whitespaceWidthAtWordStart = whitespaceWidth;
			
			i_displayOnly++;
			break;
		case '\r':
			break;
		default: {			
			pos.x += m_font->getKerning(prevChar, m_string[i], m_characterSize);
			
			if (outlineThicknesses.top() != 0.f) {
				sf::Glyph const& go = m_font->getGlyph(m_string[i], m_characterSize, bold, outlineThicknesses.top());
				addGlyphQuad(wordOutlineVertices, pos, outlineColors.top(), go, italic ? 0.209f : 0, outlineThicknesses.top());
			}
			sf::Glyph const& g = m_font->getGlyph(m_string[i], m_characterSize, bold);
			addGlyphQuad(wordVertices, pos, fillColors.top(), g, italic ? 0.209f : 0);
			
			pos.x += g.advance + letterSpacings.top();
			
			if (currentLineWidth != 0.f && pos.x > m_horizontalLimit) {
				float currentVisibleLineWidth = currentLineWidth - whitespaceWidthAtWordStart;
				if (underlined && underlineStart.x < currentLineWidth) {
					addLine(m_lineVertices, underlineStart, currentVisibleLineWidth - underlineStart.x, fillColors.top(), lineThickness);
					if (hasOutline && underlineOutlineStart.x < currentLineWidth)
						addLine(m_outlineVertices, underlineOutlineStart, currentVisibleLineWidth - underlineOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
				}
				if (strikeThrough && strikeThroughStart.x < currentLineWidth) {
					addLine(m_lineVertices, strikeThroughStart, currentVisibleLineWidth - strikeThroughStart.x, fillColors.top(), lineThickness);
					if (hasOutline && strikeThroughOutlineStart.x < currentLineWidth)
						addLine(m_outlineVertices, strikeThroughOutlineStart, currentVisibleLineWidth - strikeThroughOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
				}
				
				for (size_t i = 0; i < wordLineVertices.getVertexCount(); i += 6) {
					if (wordLineVertices[i].position.x <= currentLineWidth) {
						for (size_t j = 0; j < 6; j++) {
							sf::Vertex v = wordLineVertices[i+j];
							if (j == 1 || j == 4 || j == 5)
								v.position.x = currentVisibleLineWidth;
							else
								wordLineVertices[i+j].position.x = currentLineWidth;
							m_lineVertices.append(v);
						}
					}
					
					for (size_t j = i; j < i+6; j++) {
						wordLineVertices[j].position.x -= currentLineWidth;
						wordLineVertices[j].position.y += lineSpacingAtWordStart;
					}
				}
				for (size_t i = 0; i < wordLineOutlineVertices.getVertexCount(); i += 6) {
					if (wordLineOutlineVertices[i].position.x + outlineThicknessAtWordStart <= currentLineWidth) {
						for (size_t j = 0; j < 6; j++) {
							sf::Vertex v = wordLineOutlineVertices[i+j];
							if (j == 1 || j == 4 || j == 5)
								v.position.x = currentVisibleLineWidth + outlineThicknessAtWordStart;
							else
								wordLineOutlineVertices[i+j].position.x = currentLineWidth - outlineThicknessAtWordStart;
							m_outlineVertices.append(v);
						}
					}
					
					for (size_t j = i; j < i+6; j++) {
						wordLineOutlineVertices[j].position.x -= currentLineWidth;
						wordLineOutlineVertices[j].position.y += lineSpacingAtWordStart;
					}
				}
				
				for (size_t i = 0; i < wordVertices.getVertexCount(); i++) {
					wordVertices[i].position.x -= currentLineWidth;
					wordVertices[i].position.y += lineSpacingAtWordStart;
				}
				for (size_t i = 0; i < wordOutlineVertices.getVertexCount(); i++) {
					wordOutlineVertices[i].position.x -= currentLineWidth;
					wordOutlineVertices[i].position.y += lineSpacingAtWordStart;
				}
				
				pos.x -= currentLineWidth;
				pos.y += lineSpacingAtWordStart;
				underlineStart.x = fmaxf(0, underlineStart.x - currentLineWidth);
				underlineStart.y += lineSpacingAtWordStart;
				underlineOutlineStart.x = fmaxf(0, underlineOutlineStart.x - currentLineWidth);
				underlineOutlineStart.y += lineSpacingAtWordStart;
				strikeThroughStart.x -= fmaxf(0, strikeThroughStart.x - currentLineWidth);
				strikeThroughStart.y += lineSpacingAtWordStart;
				strikeThroughOutlineStart.x = fmaxf(0, strikeThroughOutlineStart.x - currentLineWidth);
				strikeThroughOutlineStart.y += lineSpacingAtWordStart;
				
				currentLineWidth = 0;
			}
			
			i_displayOnly++;
			break;
		}
		}
		
		i++;
	}
	
	if (underlined) {
		addLine(m_lineVertices, underlineStart, pos.x - underlineStart.x, fillColors.top(), lineThickness);
		if (hasOutline)
			addLine(m_outlineVertices, underlineOutlineStart, pos.x - underlineOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
	}
	if (strikeThrough) {
		addLine(m_lineVertices, strikeThroughStart, pos.x - strikeThroughStart.x, fillColors.top(), lineThickness);
		if (hasOutline)
			addLine(m_outlineVertices, strikeThroughOutlineStart, pos.x - strikeThroughOutlineStart.x, outlineColors.top(), lineThickness, outlineThicknesses.top());
	}
	
	for (size_t i = 0; i < wordVertices.getVertexCount(); i++)
		m_vertices.append(wordVertices[i]);
	for (size_t i = 0; i < wordLineVertices.getVertexCount(); i++)
		m_lineVertices.append(wordLineVertices[i]);
	for (size_t i = 0; i < wordOutlineVertices.getVertexCount(); i++)
		m_outlineVertices.append(wordOutlineVertices[i]);
	for (size_t i = 0; i < wordLineOutlineVertices.getVertexCount(); i++)
		m_outlineVertices.append(wordLineOutlineVertices[i]);
	
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
	
	m_bounds.top = minY;
	m_bounds.left = minX;
	m_bounds.width = maxX - minX;
	m_bounds.height = maxY - minY;
	
	m_geometryNeedsUpdate = false;
}
