#pragma once

#include <TGUI/TGUI.hpp>
#include <string>
#include <map>

using namespace std;

class UI_Template
{
protected:
    map<string, tgui::Panel::Ptr> panels;

public:

    tgui::Button::Ptr return_Button(string cap, int width, int height, int pos_x, int pos_y, tgui::Panel::Ptr P, string name, string texture_path = "");
    tgui::RadioButton::Ptr return_Radio(string cap, int pos_x, int pos_y, tgui::Panel::Ptr P, string name);
    tgui::EditBox::Ptr return_EditBox(string text, int width, int height, int pos_x, int pos_y, tgui::Panel::Ptr P, string name);
    tgui::Slider::Ptr return_Slider(int width, int height, int pos_x, int pos_y, tgui::Panel::Ptr P, string name);
    tgui::Label::Ptr return_Label(string text, int text_size, int pos_x, int pos_y, tgui::Panel::Ptr P, string name = "", string texture_path = "");

};