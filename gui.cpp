#include <iostream>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <SFML/Window.hpp>

using namespace std;
using namespace sf;
using namespace tgui;


class UI_Template
{
protected:
    map<string,Panel::Ptr> panels;
    vector<tgui::Label::Ptr> text_labels;
    RenderWindow window;
    Gui gui;

public:

    auto return_Button(string cap, int width, int height, int pos_x, int pos_y, Panel::Ptr P, string name)
    {
        auto button = Button::create();
        button->setText(cap);
        button->setPosition(pos_x, pos_y);
        button->setSize(width, height);
        button->getRenderer()->setBorders(1.4);
        button->getRenderer()->setTextColorHover(sf::Color::Blue);
        button->getRenderer()->setBorderColorHover(sf::Color::Blue);
        P->add(button, name);;
        return button;
    }

    auto return_Radio(string cap, int pos_x, int pos_y, Panel::Ptr P, string name)
    {
        auto button = RadioButton::create();
        button->setText(cap);
        button->setPosition(pos_x, pos_y);
        button->getRenderer()->setBorders(1.4);
        button->getRenderer()->setTextColorHover(sf::Color::Blue);
        button->getRenderer()->setBorderColorHover(sf::Color::Blue);
        button->getRenderer()->setBorderColorChecked(sf::Color::Blue);
        button->getRenderer()->setTextColorChecked(sf::Color::Blue);
        P->add(button, name);
        return button;
    }

    auto return_EditBox(string text, int width, int height, int pos_x, int pos_y, Panel::Ptr P, string name)
    {
        auto editBox = EditBox::create();
        editBox->setPosition(pos_x, pos_y);
        editBox->setSize(width, height);
        editBox->setDefaultText(text);
        editBox->getRenderer()->setBorders(1.4);
        editBox->getRenderer()->setBorderColorHover(sf::Color::Blue);
        editBox->getRenderer()->setBorderColorFocused(sf::Color::Blue);
        editBox->getRenderer()->setTextSize(15);
        P->add(editBox, name);
        return editBox;
    }


    void display_panel(string name)
    {
        if (panels.count(name) > 0)
        {
            panels[name]->setVisible(true);
        }
    }


    void Animated_Text_Logo(string logo_name,int text_size,int pos_x,int pos_y)
    {
        for (int i = 0; i < logo_name.length() ; i++)
        {
            Label::Ptr label = Label::create();
            label->setText(logo_name[i]);
            label->setTextSize(text_size);
            label->setPosition(pos_x, pos_y);
            label->getRenderer()->setFont("fonts/Himalayas-owAP4.otf");
            label->getRenderer()->setTextOutlineThickness(1.2);
            label->getRenderer()->setTextOutlineColor(sf::Color::Black);
            label->getRenderer()->setTextColor(sf::Color::White);
            text_labels.push_back(label);
            gui.add(text_labels[i]);
        }
    }

    void UI_template_Maker()
    {
        Animated_Text_Logo("Smart Player", 40, 500, 500);
    }

};


class UI_Functionality :public UI_Template
{
private:
    RenderWindow window;
public:
    void init_window()
    {
        
        window.create(VideoMode({ 1920,1080 }), "Smart Music Player");
        gui.setTarget(window);
        
        int l = 0;
        sf::Clock clock;
        float animation_time = 0.5f;

        while (window.isOpen())
        {
            while (const std::optional event = window.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                    window.close();
            }

            if (clock.getElapsedTime().asSeconds() > animation_time)
            {
                text_labels[l % 13]->getRenderer()->setTextColor(sf::Color::Green);
                for (int i = 0; i < 13; i++)
                {
                    if (i != l % 13)
                    {
                        text_labels[i]->getRenderer()->setTextColor(sf::Color::White);
                    }

                }
                l++;
                clock.restart();
            }

            window.clear(sf::Color::White);
            gui.draw();
            window.display();
        }
    }
};

int main()
{
    UI_Functionality app;
    app.init_window();
    return 0;
}