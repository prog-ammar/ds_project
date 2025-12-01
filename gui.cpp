#include<iostream>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <SFML/Window.hpp>

using namespace std;
using namespace sf;
using namespace tgui;

// Simple GUI wrapper
class GUI
{
private:
    Gui gui;
    RenderWindow window;
public:
    void run()
    {
        window.create(VideoMode({1920,1080}), "SmartPlayer");

        gui.setTarget(window);

        window.setFramerateLimit(60);
        while (window.isOpen())
        {
                while (const std::optional event = window.pollEvent())
                {
                    if (event->is<sf::Event::Closed>())
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
    GUI app;
    app.run();
    return 0;
}