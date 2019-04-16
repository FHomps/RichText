#include <SFML/Graphics.hpp>
#include "richtext.h"
#include <functional>

int main() {

    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Project", sf::Style::Close);
    window.setFramerateLimit(60);

    sf::Event haps;

	sf::Font font;
	font.loadFromFile("Resources/OpenSans.ttf");
	
	RichText rt(font, "This is normal text. Here co<u>Mes underlined. Outlines th<ot=2>En arrive. Fancy co<c=red,id=0>Lors, too. They can cha<oc=green,id=1>Nge through pressing Return. A line pa\nSses. It brings sp<lts=3,lns=1.1>Acing. Now strik<s,id=1>Ethrough joins - it can change too -, and underlined lea</u>Ves. No more c</c>Olo</oc>Rs. No more </ot,/oc>Outlines either. A line pass\nEs, and the spa</lts,/lns>Cing ends. Finally striket</s>Hrough leaves. Two li\n\nNes pass. It ends here. An extension can be added by pressing X.\n", 25);
	bool state = 0;
	float horizontalLimit = 600;
	rt.setHorizontalLimit(horizontalLimit);
	rt.setCharacterLimit(200);
	
	size_t highlighted = 0;
	sf::RectangleShape highlightRect;
	highlightRect.setFillColor(sf::Color::Yellow);
	sf::FloatRect charBounds = rt.findCharacterBounds(highlighted);
	highlightRect.setPosition(charBounds.left, charBounds.top);
	highlightRect.setSize(sf::Vector2f(charBounds.width, charBounds.height));
	
	sf::RectangleShape boundsRect;
	boundsRect.setFillColor(sf::Color::Blue);
	sf::FloatRect bounds = rt.getLocalBounds();
	boundsRect.setPosition(bounds.left, bounds.top);
	boundsRect.setSize(sf::Vector2f(bounds.width, bounds.height));
	
    while (window.isOpen()) {
        while (window.pollEvent(haps)) {
            if (haps.type == sf::Event::Closed)
                window.close();
			if (haps.type ==  sf::Event::KeyPressed) {
				switch (haps.key.code) {
				case sf::Keyboard::Return: {
					rt.setFillColor(0, state ? sf::Color::Red : sf::Color::Green);
					rt.setOutlineColor(1, state ? sf::Color::Green : sf::Color::Red);
					rt.setStyle(1, state ? sf::Text::StrikeThrough : sf::Text::Regular);
					state = !state;
					break;
				}
				case sf::Keyboard::O:
					highlighted -= std::min(highlighted, size_t(1));
					charBounds = rt.findCharacterBounds(highlighted);
					highlightRect.setPosition(charBounds.left, charBounds.top);
					highlightRect.setSize(sf::Vector2f(charBounds.width, charBounds.height));
					break;
				case sf::Keyboard::P:
					highlighted++;
					charBounds = rt.findCharacterBounds(highlighted);
					highlightRect.setPosition(charBounds.left, charBounds.top);
					highlightRect.setSize(sf::Vector2f(charBounds.width, charBounds.height));
					break;
				case sf::Keyboard::Left: {
					rt.setCharacterLimit(rt.getCharacterLimit() - std::min(rt.getCharacterLimit(), size_t(1)));
					break;
				case sf::Keyboard::Right:
					rt.setCharacterLimit(rt.getCharacterLimit()+1);
					break;	
				}
				case sf::Keyboard::PageDown:
					rt.setCharacterLimit(rt.getMaxEffectiveCharacterLimit());
					break;
				case sf::Keyboard::PageUp:
					rt.setCharacterLimit(0);
					break;
				case sf::Keyboard::X:
					rt.parseString("Another extension can be added by pressing X. ", true);
					break;
				default:
					break;				
				}
				bounds = rt.getLocalBounds();
				boundsRect.setPosition(bounds.left, bounds.top);
				boundsRect.setSize(sf::Vector2f(bounds.width, bounds.height));
			}
        }

        window.clear(sf::Color::Magenta);
		window.draw(boundsRect);
		window.draw(highlightRect);
		window.draw(rt);
        window.display();
    }
}
