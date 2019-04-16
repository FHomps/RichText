#ifndef RICHTEXT_H
#define RICHTEXT_H

#include <SFML/Graphics.hpp>
#include <map>
#include <deque>

class RichText : public sf::Drawable, public sf::Transformable
{
public:
	RichText();
	RichText(sf::Font const& font, sf::String const& string, uint characterSize = 20);
	~RichText();
	
	void parseString(sf::String const& s, bool append = false);
	sf::String const& getParsedString() const;
	
	void setFont(sf::Font const& font);
	void setCharacterSize(uint size);
	
	void setStyle(sf::Uint32 style);
	void setStyle(int ID, sf::Uint32 style);
	void setStyle(int ID, sf::Uint32 style, bool activated);
	
	void setFillColor(sf::Color color);
	void setFillColor(int ID, sf::Color color);
	void setFillColor(int ID, bool activated);
	
	void setOutlineThickness(float thickness);
	void setOutlineThickness(int ID, float thickness);
	void setOutlineThickness(int ID, bool activated);
	
	void setOutlineColor(sf::Color color);
	void setOutlineColor(int ID, sf::Color color);
	void setOutlineColor(int ID, bool activated);
	
	void setLetterSpacingFactor(float factor);
	void setLetterSpacingFactor(int ID, float factor);
	void setLetterSpacingFactor(int ID, bool activated);
	
	void setLineSpacingFactor(float factor);
	void setLineSpacingFactor(int ID, float factor);
	void setLineSpacingFactor(int ID, bool activated);
	
	sf::Font const& getFont() const;
	uint getCharacterSize() const;
	sf::Uint32 getStyle() const;
	sf::Color getFillColor() const;
	float getOutlineThickness() const;
	sf::Color getOutlineColor() const;
	float getLetterSpacingFactor() const;
	float getLineSpacingFactor() const;	
	
	void setHorizontalLimit(float limit);
	float getHorizontalLimit() const;
	
	void setCharacterLimit(size_t limit);
	size_t getCharacterLimit() const;
	size_t getMaxEffectiveCharacterLimit() const;
	
	sf::FloatRect findCharacterBounds(size_t index) const;

	sf::FloatRect getLocalBounds() const;
	sf::FloatRect getGlobalBounds() const;	
	
private:
	sf::Font const* m_font;
	sf::String m_string;
	uint m_characterSize;
	
	class VariableStyle {
	public:
		VariableStyle();		
		void rewind();
		
		std::deque<bool> bolds;
		std::deque<bool> italics;
		std::deque<bool> underlineds;
		std::deque<bool> strikeThroughs;
		std::deque<sf::Color> fillColors;
		std::deque<float> outlineThicknesses;
		std::deque<sf::Color> outlineColors;
		std::deque<float> letterSpacingFactors;
		std::deque<float> lineSpacingFactors;
	};
	
	class Stylizer {
	public:
		enum StyleProperty { None, Bold, Italic, Underlined, StrikeThrough, FillColor, OutlineThickness, OutlineColor, LetterSpacing, LineSpacing };
		
		Stylizer(StyleProperty const& type) : m_type(type) {}
		virtual ~Stylizer() {}
		virtual StyleProperty stylize(VariableStyle& vs) const = 0; //Return is not necessarily m_type (stylize() computes if the change to vs was visually noticeable)
		StyleProperty getType() const { return m_type; }
		
		size_t line = std::numeric_limits<size_t>::max(); //At what line was the stylizer last sighted
		
	protected:
		StyleProperty m_type;
	};
	
	template<class T>
	class SpecializedStylizer : public Stylizer {
	public:
		SpecializedStylizer(StyleProperty const& type);
	protected:
		std::deque<T> VariableStyle::* m_styleMember;
	};
	
	template<class T>
	class EnderStylizer : public SpecializedStylizer<T> {
	public:
		EnderStylizer(Stylizer::StyleProperty type);
		virtual Stylizer::StyleProperty stylize(VariableStyle& vs) const;
	};
	
	template<class T>
	class StarterStylizer : public SpecializedStylizer<T> {
	public:
		StarterStylizer(Stylizer::StyleProperty type); //Inactive stylizer - will copy current state of the property (stylize() will return None)
		StarterStylizer(Stylizer::StyleProperty type, T value);
		virtual Stylizer::StyleProperty stylize(VariableStyle& vs) const;
		
		void setValue(T value);
		bool activated;
	private:
		T m_value;
	};
	
	
	std::multimap<size_t, Stylizer*> m_stylizers; //Stylizers, mapped to the character they activate at
	
	std::multimap<int, Stylizer*> m_modifiableStylizers; //Stylizers accessible by ID
	
	mutable VariableStyle m_style;
	
	mutable sf::VertexArray m_charVertices;
	mutable sf::VertexArray m_charOutlineVertices;
	mutable sf::VertexArray m_lineVertices;
	mutable sf::VertexArray m_lineOutlineVertices;
	
	mutable std::vector<size_t> m_lineStart_i;
	mutable std::vector<float> m_lineStart_verticalPos;
	mutable std::vector<size_t> m_lineStart_char;
	mutable std::map<size_t, size_t> m_lineStart_line;
	mutable std::map<size_t, size_t> m_lineStart_charOutline;
	mutable std::map<size_t, size_t> m_lineStart_lineOutline;
	
	void initializeLineStarts();
	
	size_t m_totalDisplayableCharacters = 0;
	
	float m_horizontalLimit = std::numeric_limits<float>::infinity();
	
	size_t m_characterLimit = std::numeric_limits<size_t>::max();
	
	mutable sf::FloatRect m_bounds;
	
	mutable bool m_shouldUpdateVertices;
	mutable size_t m_updateStartLine = 0;
	void updateVertices() const;
	
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

#endif // RICHTEXT_H
