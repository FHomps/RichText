#include <SFML/Graphics.hpp>
#include <iostream>
#include "richtext.h"

int main() {	
    sf::RenderWindow window(sf::VideoMode(1200, 600), "SFML Project", sf::Style::Close);
    window.setFramerateLimit(60);

    sf::Event haps;
	
	sf::Font font;
	font.loadFromFile("Resources/OpenSans.ttf");
	
	RichText rt;
	rt.setCharacterSize(20);
	sf::String str("Lorem ipsum dolor s<c=#FF00FF>it amet, <b>consectetur adip<u>iscing elit, sed do eiusm<ot=2,oc=#00FF00>od tempor incididunt </b>ut labore et dolore mag</u>na aliqua. Ut enim a</c>d minim veniam, quis nos</u>trud exercitation ullamco la</ot,/oc>boris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
	rt.setString(str);
	rt.setFont(font);
	float limit = 250;
	rt.setHorizontalLimit(limit);
	rt.setDefaultFillColor(sf::Color::White);
	size_t charLimit = 0;
	rt.setCharacterLimit(charLimit);
			
	sf::RectangleShape rect(sf::Vector2f(rt.getLocalBounds().width, rt.getLocalBounds().height));
	rect.setPosition(sf::Vector2f(rt.getLocalBounds().left, rt.getLocalBounds().top) + rt.getPosition());
	rect.setFillColor(sf::Color::Blue);
	
    while (window.isOpen()) {
        while (window.pollEvent(haps)) {
            if (haps.type == sf::Event::Closed)
                window.close();
			else if (haps.type == sf::Event::KeyPressed) {
				if (haps.key.code == sf::Keyboard::Space) {
					rt.setCharacterLimit(rt.getCharacterLimit()+1);
				}
				else if (haps.key.code == sf::Keyboard::BackSpace) {
					rt.setCharacterLimit(rt.getCharacterLimit()-1);
				}
				rect.setSize(sf::Vector2f(rt.getLocalBounds().width, rt.getLocalBounds().height));
				rect.setPosition(sf::Vector2f(rt.getLocalBounds().left, rt.getLocalBounds().top) + rt.getPosition());
			}
			else if (haps.type == sf::Event::MouseButtonPressed) {
				if (haps.mouseButton.button == sf::Mouse::Left)
					limit += 20;
				else if (haps.mouseButton.button == sf::Mouse::Right) {
					limit -= 20;
				}
				rt.setHorizontalLimit(limit);
				rect.setSize(sf::Vector2f(rt.getLocalBounds().width, rt.getLocalBounds().height));
				rect.setPosition(sf::Vector2f(rt.getLocalBounds().left, rt.getLocalBounds().top) + rt.getPosition());
			}
        }

        window.clear(sf::Color::Black);
		window.draw(rect);
		window.draw(rt);
	    window.display();
    }
}
