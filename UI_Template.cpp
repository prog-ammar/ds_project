#include "UI_Template.h"


tgui::Button::Ptr UI_Template::return_Button(string cap, int width, int height, int pos_x, int pos_y, tgui::Panel::Ptr P, string name, string texture_path)
{
    tgui::Button::Ptr button = tgui::Button::create();
    button->setText(cap);
    button->setPosition(pos_x, pos_y);
    button->setSize(width, height);
    button->getRenderer()->setBorders(1.4);
    button->getRenderer()->setTextColorHover(sf::Color::White);
    button->getRenderer()->setBorderColorHover(sf::Color::White);
    if (texture_path != "")
    {
        tgui::Texture t(texture_path);
        button->getRenderer()->setTexture(t);
        button->getRenderer()->setBorderColor(tgui::Color::White);
        t.setDefaultSmooth(true);
    }
    P->add(button, name);
    return button;
}

tgui::RadioButton::Ptr UI_Template::return_Radio(string cap, int pos_x, int pos_y, tgui::Panel::Ptr P, string name)
{
    tgui::RadioButton::Ptr button = tgui::RadioButton::create();
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

tgui::EditBox::Ptr UI_Template::return_EditBox(string text, int width, int height, int pos_x, int pos_y, tgui::Panel::Ptr P, string name)
{
    tgui::EditBox::Ptr editBox = tgui::EditBox::create();
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


tgui::Slider::Ptr UI_Template::return_Slider(int width, int height, int pos_x, int pos_y, tgui::Panel::Ptr P, string name)
{

    tgui::Slider::Ptr slider = tgui::Slider::create();
    slider->setPosition(pos_x, pos_y);
    slider->setSize(width, height);
    slider->setMinimum(0);
    slider->setMaximum(100);
    slider->getRenderer()->setThumbColor(tgui::Color::Blue);
    slider->getRenderer()->setThumbColorHover(tgui::Color::Blue);
    P->add(slider, name);
    return slider;
}

tgui::Label::Ptr UI_Template::return_Label(string text, int text_size, int pos_x, int pos_y, tgui::Panel::Ptr P, string name , string texture_path )
{
    tgui::Label::Ptr label = tgui::Label::create(text);
    label->setPosition(pos_x, pos_y);
    label->setTextSize(text_size);
    if (texture_path != "")
    {
        tgui::Texture t(texture_path);
        label->getRenderer()->setTextureBackground(t);
        t.setDefaultSmooth(true);
    }
    P->add(label, name);
    return label;
}