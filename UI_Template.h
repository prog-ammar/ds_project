#pragma once
#include <TGUI/TGUI.hpp>
#include <TGUI/Widgets/Panel.hpp>
#include <TGUI/Widgets/Button.hpp>
#include <TGUI/Widgets/Label.hpp>
#include <TGUI/Widgets/RadioButton.hpp>
#include <TGUI/Widgets/EditBox.hpp>
#include <TGUI/Widgets/Slider.hpp>
using namespace tgui;
using namespace std;

class UI_Template
{
protected:
    map<string, Panel::Ptr> panels;

public:

    Button::Ptr return_Button(string cap, int width, int height, int pos_x, int pos_y, Panel::Ptr P, string name, string texture_path = "");
    RadioButton::Ptr return_Radio(string cap, int pos_x, int pos_y, Panel::Ptr P, string name);
    EditBox::Ptr return_EditBox(string text, int width, int height, int pos_x, int pos_y, Panel::Ptr P, string name);
    Slider::Ptr return_Slider(int width, int height, int pos_x, int pos_y, Panel::Ptr P, string name);
    Label::Ptr return_Label(string text, int text_size, int pos_x, int pos_y, Panel::Ptr P, string name = "", string texture_path = "");

};