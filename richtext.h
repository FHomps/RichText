#ifndef RICHTEXT_H
#define RICHTEXT_H

#include <SFML/Graphics.hpp>

class RichText : public sf::Drawable, public sf::Transformable {
public:
	RichText();
	RichText(sf::String const& string, sf::Font const& font, unsigned int baseCharacterSize = 30);
	
	void setString(sf::String const& string);
	
	void setFont(sf::Font const& font);
		
	void setCharacterSize(unsigned int size);
	
	void setDefaultLineSpacing(float spacingFactor);
	
	void setDefaultLetterSpacing(float spacingFactor);
	
	void setDefaultFillColor(sf::Color const& color);
	
	void setDefaultOutlineColor(sf::Color const& color);
	
	void setDefaultOutlineThickness(float thickness);
	
	void setDefaultStyle(sf::Uint32 style);
	
	void setHorizontalLimit(float limit);
	
	void setCharacterLimit(size_t limit);
	
	sf::String const& getString() const;
	
	sf::Font const* getFont() const;
	
	unsigned int getCharacterSize() const;
	
	float getBaseLetterSpacing() const;
	
	float getBaseLineSpacing() const;
	
	sf::Color const& getBaseFillColor() const;
	
	sf::Color const& getBaseOutlineColor() const;
	
	float getBaseOutlineThickness() const;
	
	sf::Uint32 getDefaultStyle() const;
	
	float getHorizontalLimit() const;
	
	size_t getCharacterLimit() const;
	
	sf::FloatRect findCharacterBounds(size_t index) const;
	
	sf::FloatRect getLocalBounds() const;
	
	sf::FloatRect getGlobalBounds() const;
	
private:
	float m_dLineSpacingFactor = 1.f;
	float m_dLetterSpacingFactor = 1.f;
	sf::Color m_dFillColor = sf::Color::Black;
	sf::Color m_dOutlineColor = sf::Color::White;
	float m_dOutlineThickness = 0.f;
	
	bool m_dBold = false;
	bool m_dItalic = false;
	bool m_dUnderlined = false;
	bool m_dStrikeThrough = false;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	
	void ensureGeometryUpdate() const;
	
	void ensurePDVUpdate() const;
	
	sf::String m_string;
	mutable size_t m_displayableCharacters = 0;
	sf::Font const* m_font;
	
	uint m_characterSize;
	
	float m_horizontalLimit = std::numeric_limits<float>::infinity();
	
	size_t m_characterLimit = std::numeric_limits<size_t>::max();
	
	mutable sf::VertexArray m_vertices;
	mutable sf::VertexArray m_lineVertices;
	mutable std::vector<size_t> m_displayedLineCorrespondingToLine;
	mutable sf::VertexArray m_outlineVertices;
	mutable std::vector<float> m_displayedCharacterCorrespondingToOutline;
	mutable sf::VertexArray m_lineOutlineVertices;
	mutable std::vector<size_t> m_displayedLineCorrespondingToLineOutline;
	mutable sf::FloatRect m_bounds;
	
	mutable bool m_geometryNeedsUpdate;
	
	mutable std::vector<size_t> m_lastDisplayedCharacterInLine;
	mutable sf::VertexArray m_partialDisplayVertices;
	mutable bool m_pDVNeedsUpdate;	
};

#endif // RICHTEXT_H
