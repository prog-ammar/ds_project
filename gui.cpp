#include<iostream>
#include<SFML/Graphics.hpp>
#include<SFML/Window.hpp>
#include<TGUI/TGUI.hpp>
#include<TGUI/Backend/SFML-Graphics.hpp>

using namespace std;
using namespace sf;
using namespace tgui;

class GUI_
{
private:
    RenderWindow window;
    Gui gui;

public:
    void init_window()
    {
        window.create(VideoMode(1920, 1080), "SmartPlayer");
        window.setFramerateLimit(60);
        while (window.isOpen())
        {
            Event event;
            while (window.pollEvent(event))
            {
                gui.handleEvent(event);

                if (event.type == Event::Closed)
                    window.close();
            }
            window.clear(sf::Color::White);
            gui.draw();
            window.display();
        }
    }
};


int main()
{
    
    
}