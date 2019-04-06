#include <SFML/Graphics.hpp>
#include "richtext.h"

int main() {	
    sf::RenderWindow window(sf::VideoMode(1200, 600), "SFML Project", sf::Style::Close);
    window.setFramerateLimit(60);

    sf::Event haps;
	
	sf::Font font;
	font.loadFromFile("Resources/OpenSans.ttf");
	
	RichText rt;
	rt.setCharacterSize(20);
	sf::String str("This is normal text. Here co<u>Mes underlined. Outlines th<ot=2,oc=#000000>En arrive. Fancy co<c=#FF0000>Lor<oc=#00FF00>S, too. A line pa\nSses. It brings sp<lts=3,lns=1.1>Acing. Now strik<s>Ethrough joins, and underlined lea</u>Ves. No more c</c>Olo</oc>Rs. No more out</ot,/oc>Lines either. A line pass\nEs, and the spa</lts,/lns>Cing ends. Finally striket</s>Hrough leaves.");
	rt.setString(str);
	rt.setFont(font);
	float limit = 230;
	rt.setHorizontalLimit(limit);
	rt.setDefaultFillColor(sf::Color::White);
	rt.setPosition(50, 50);
	size_t charLimit = 186;
	rt.setCharacterLimit(charLimit);
			
	sf::RectangleShape rect(sf::Vector2f(limit, rt.getLocalBounds().height));
	rect.setPosition(sf::Vector2f(rt.getLocalBounds().left, rt.getLocalBounds().top) + rt.getPosition());
	rect.setFillColor(sf::Color::Blue);
	
    while (window.isOpen()) {
        while (window.pollEvent(haps)) {
            if (haps.type == sf::Event::Closed)
                window.close();
			else if (haps.type == sf::Event::KeyPressed) {
				if (haps.key.code == sf::Keyboard::Space) {
					charLimit++;
					rt.setCharacterLimit(charLimit);
				}
				else if (haps.key.code == sf::Keyboard::BackSpace) {
					charLimit--;
					rt.setCharacterLimit(charLimit);
				}
			}
			else if (haps.type == sf::Event::MouseButtonPressed) {
				if (haps.mouseButton.button == sf::Mouse::Left)
					limit += 20;
				else if (haps.mouseButton.button == sf::Mouse::Right) {
					limit -= 20;
				}
				rt.setHorizontalLimit(limit);
			}
        }
		
		rect.setSize(sf::Vector2f(limit, rt.getLocalBounds().height));
		rect.setPosition(sf::Vector2f(rt.getLocalBounds().left, rt.getLocalBounds().top) + rt.getPosition());

        window.clear(sf::Color::Black);
		window.draw(rect);
		window.draw(rt);
	    window.display();
    }
}
