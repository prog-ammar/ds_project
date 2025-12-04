#include <iostream>
#include <TGUI/TGUI.hpp>
#include<TGUI/AllWidgets.hpp>
#include<TGUI/Widget.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
#include <mutex>
#include <mpg123.h>
#include <vector>
#include <memory>
#include "backup.cpp"

using namespace std;
using namespace sf;
using namespace tgui;



class Mp3Stream : public sf::SoundStream
{

private:

    mpg123_handle* mh = nullptr;
    size_t bufferSize = 0;
    std::vector<unsigned char> buffer;
    size_t samplesProcessed;

    long rate = 0;
    int channels = 0;
    int encoding = 0;
    int samplecount = 0;
public:
    Mp3Stream() {}

    bool open(const std::string& filename)
    {
        mpg123_init();
        int err = MPG123_OK;

        mh = mpg123_new(NULL, &err);
        if (!mh)
            return false;

        if (mpg123_open(mh, filename.c_str()) != MPG123_OK)
            return false;

        // Some builds of libmpg123 restrict supported output sample rates.
        // Ensure we request a supported rate (44100, 22050 or 11025). 44100 is the safe default.
        mpg123_format_none(mh);
        int setRet = mpg123_format(mh, 44100, MPG123_STEREO, MPG123_ENC_SIGNED_16);
        if (setRet != MPG123_OK)
        {
            // Failed to set a supported output format
            mpg123_close(mh);
            mpg123_delete(mh);
            mh = nullptr;
            mpg123_exit();
            return false;
        }

        const off_t length = mpg123_length(mh);

        mpg123_getformat(mh, &rate, &channels, &encoding);

        // initialize(channelCount, sampleRate)
        initialize(static_cast<unsigned int>(channels), static_cast<unsigned int>(rate),{});
        samplecount = channels * length;
        bufferSize = mpg123_outblock(mh);
        buffer.resize(bufferSize);

        return true;
    }

    virtual void onSeek(sf::Time timeOffset) override
    {
        if(!mh)
            return;
        SoundSource::Status old = getStatus();
           
        if (SoundSource::Status::Playing == old)
        {
            pause();
        }

        double t = timeOffset.asSeconds();
        // Convert requested time -> decoded PCM sample offset (interleaved samples).
        const off_t new_offset =
            mpg123_seek(mh, static_cast<off_t>(t*rate), SEEK_SET);
        if (new_offset < 0)
        {
            sf::err() << "Failed to seek with mpg123: "
                << mpg123_plain_strerror(static_cast<int>(new_offset)) << std::endl;
        }
        else
        {
            initialize(channels, rate,{});
            samplesProcessed = new_offset;
            
        }

        if (SoundSource::Status::Playing == old)
        {
            play();
        }

    }

    ~Mp3Stream()
    {
        if (mh)
        {
            mpg123_close(mh);
            mpg123_delete(mh);
            mpg123_exit();
        }
    }

protected:

    virtual bool onGetData(Chunk& data) override
    {
        size_t done = 0;
        int ret = mpg123_read(mh, buffer.data(), bufferSize, &done);

        if (ret == MPG123_DONE || done == 0)
            return false;

        data.samples = reinterpret_cast<short*>(buffer.data());
        data.sampleCount = done / sizeof(short);

        samplesProcessed += data.sampleCount;
        return true;
    }


};

class Mp3Player
{
private:
    std::unique_ptr<Mp3Stream> stream;
    float durationSeconds = 0.f;
    bool is_playing;
public:
    Mp3Player() { is_playing = false; }


    bool get_status() { return is_playing; }
    void change_status() { is_playing = !is_playing; }

    bool open(const std::string& filename)
    {
        stream = std::make_unique<Mp3Stream>();
        if (!stream->open(filename))
            return false;

        return true;
    }

    void play() { if (stream) stream->play(); change_status(); }
    void pause() { if (stream) stream->pause(); change_status(); }
    void stop() { if (stream) stream->stop(); }
    void setVolume(float volume) { if (stream) stream->setVolume(volume); }
    void seek(double time) { sf::Time t= sf::seconds(time);if (stream) stream->onSeek(t); }

    // Optional: duration if you compute it externally
    void setDuration(float seconds) { durationSeconds = seconds; }
    float getDuration() const { return durationSeconds; }


};



class UI_Template
{
protected:
    map<string, Panel::Ptr> panels;
    Gui gui;

public:

    auto return_Button(string cap, int width, int height, int pos_x, int pos_y, Panel::Ptr P, string name, string texture_path = "")
    {
        auto button = Button::create();
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

    
    auto return_Slider(int width, int height, int pos_x, int pos_y, Panel::Ptr P, string name)
    {

        auto slider = Slider::create();
        slider->setPosition(pos_x, pos_y);
        slider->setSize(width, height);
        slider->setMinimum(0);
        slider->setMaximum(100);
        slider->getRenderer()->setThumbColor(tgui::Color::Blue);
        slider->getRenderer()->setThumbColorHover(tgui::Color::Blue);
        P->add(slider, name);
        return slider;
    }

    auto return_Label(string text, int text_size , int pos_x, int pos_y, Panel::Ptr P, string name="", string texture_path = "")
    {
        auto label = Label::create(text);
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

    /*void display_panel(string name)
    {
        if (panels.count(name) > 0)
        {
            panels[name]->setVisible(true);
        }
    }*/


    /*void Animated_Text_Logo(string logo_name,int text_size,int pos_x,int pos_y,int spacing)
    {
        for (int i = 0; i < logo_name.length() ; i++)
        {
            Label::Ptr label = Label::create();
            label->setText(logo_name[i]);
            label->setTextSize(text_size);
            label->setPosition(pos_x+i*30, pos_y);
            label->getRenderer()->setFont("fonts/SuperCartoon-6R791.ttf");
            label->getRenderer()->setTextOutlineThickness(1.2);
            label->getRenderer()->setTextOutlineColor(sf::Color::Black);
            label->getRenderer()->setTextColor(sf::Color::White);
 
            gui.add(text_labels[i]);
        }
    }*/

};


class UI_Functionality :public UI_Template
{
private:
    std::unique_ptr<Mp3Player> persistentPlayer;
    RenderWindow window;

public:

    UI_Functionality()
    {
        persistentPlayer = std::make_unique<Mp3Player>();
    }
    void init_window()
    {
        
        window.create(VideoMode({ 1920,1080 }), "Smart Music Player");
        window.setVerticalSyncEnabled(true);
        gui.setTarget(window);
        
        int l = 0;
        sf::Clock clock;
        float animation_time = 0.5f;

        UI_template_Maker();
        /*unsigned int v = p_bar->getValue();
        sf::Clock ck;
        while (v < p_bar->getMaximum())
        {
            if (ck.getElapsedTime().asSeconds() >= 1)
            {
                p_bar->incrementValue();
                v = p_bar->getValue();
                ck.restart();
            }

        }*/
        auto p_bar = panels["play_panel"]->get<tgui::Slider>("p_bar");
        while (window.isOpen())
        {
            while (const std::optional event = window.pollEvent())
            {
                gui.handleEvent(*event);
                if (event->is<sf::Event::Closed>())
                {   
                    window.close();
                }
            }

            if (persistentPlayer->get_status())
            {
                if (clock.getElapsedTime().asSeconds() >= 1.0f)
                {
                    if (p_bar->getValue() < p_bar->getMaximum())
                    {
                        p_bar->setValue(p_bar->getValue()+1.0f);
                    }
                    else
                    {
                        persistentPlayer->pause();
                    }
                    clock.restart();
                }
            }
            

            /*if (clock.getElapsedTime().asSeconds() > animation_time)
            {
                text_labels[l % 18]->getRenderer()->setTextColor(sf::Color(0,100,10));
                for (int i = 0; i < 18; i++)
                {
                    if (i != l % 18)
                    {
                        text_labels[i]->getRenderer()->setTextColor(sf::Color::White);
                    }

                }
                l++;
                clock.restart();
            }*/

            window.clear(sf::Color(0,50,25));
            gui.draw();
            window.display();
        }
        gui.removeAllWidgets();
    }


    void intro_panel()
    {
        auto init_panel = Panel::create({ "100%","100%" });
        panels["Intro Menu"] = init_panel;

        tgui::Texture background;
        background.load("background/bck.png");
        init_panel.get()->getRenderer()->setTextureBackground(background);
        gui.add(init_panel);


    }

    void search_panel()
    {
        auto init_panel = Panel::create({ 1920.f, 80.f });
        panels["search_panel"] = init_panel;
        auto search = return_EditBox("Search", 360, 40, 750, 25, panels["search_panel"], "search");
        auto label = return_Label("", 0, 150, 20, panels["search_panel"], "bg", "background/logo.png");
        label->setSize(50, 50);

        label = return_Label("Smart Music Player", 25, 220, 30, panels["search_panel"], "logo");
        tgui::Font font("fonts/Vulturemotor Demo.otf");
        label->getRenderer()->setFont(font);
        label->getRenderer()->setTextColor(tgui::Color(0, 0, 139));

        panels["search_panel"]->getRenderer()->setBackgroundColor(sf::Color::White);
        panels["search_panel"]->setVisible(true);

        search->onReturnKeyPress([=]
            {

            });

        gui.add(panels["search_panel"]);

    }

    void play_panel()
    {
        auto init_panel = Panel::create({ 1920.f, 100.f });
        init_panel->setPosition({ 0.f,900.f });
        panels["play_panel"] = init_panel;


        auto music_button = return_Button("", 40, 40, 150, 20, panels["play_panel"], "music_img", "background/music.png");
        auto song_label = return_Label("", 12, 210, 30, panels["play_panel"], "song_label");
        auto prev_button = return_Button("", 30, 30, 900, 20, panels["play_panel"], "prev", "background/back.png");
        auto play_button = return_Button("", 30, 30, 950, 20, panels["play_panel"], "play", "background/play1.png");
        auto next_button = return_Button("", 30, 30, 1000, 20, panels["play_panel"], "next", "background/next.png");
        auto progressBar = return_Slider(520, 10, 700, 60, panels["play_panel"], "p_bar");

        /*tgui::Texture t1("background/pause1.jpg");
        tgui::Texture t2("background/play1.png");

        play_button->onPress([=] {
            if (persistentPlayer->get_status())
            {
                persistentPlayer->pause();
                play_button->getRenderer()->setTexture(t2);
                play_button->setSize(30, 30);
                play_button->setPosition(950, 20);
            }

            else
            {
                persistentPlayer->play();
                play_button->getRenderer()->setTexture(t1);
                play_button->setSize(40, 40);
                play_button->setPosition(945, 15);
            }

            }); */

        progressBar->onMouseEnter([=]
            {
                progressBar->onValueChange([=]
                    {
                        persistentPlayer->seek(static_cast<double>(progressBar->getValue()));
                    });

            });

        auto sound_button = return_Button("", 30, 30, 1600, 50, panels["play_panel"], "sound", "background/volume.png");
        auto sBar = return_Slider(100, 10, 1640, 60, panels["play_panel"], "sound_bar");
        sBar->setValue(100);
        sBar->onValueChange([=]
            {
                persistentPlayer->setVolume(sBar->getValue());
            });


        panels["play_panel"]->getRenderer()->setBackgroundColor(sf::Color::White);
        panels["play_panel"]->setVisible(true);

        gui.add(panels["play_panel"]);
    }

    void main_mid_panel()
    {

        auto sc_panel = ScrollablePanel::create({ 1920.f,820.f });
        sc_panel->setPosition(0, 80);
        sc_panel->getRenderer()->setBackgroundColor(tgui::Color::White);
        panels["main_mid_panel"] = sc_panel;

        sc_panel->setVisible(false);
        gui.add(panels["main_mid_panel"]);
    }

    void mid_panel_1()
    {
        auto sc_panel = ScrollablePanel::create({ 1920.f,820.f });
        sc_panel->setPosition(0, 80);
        sc_panel->getRenderer()->setBackgroundColor(tgui::Color::White);
        panels["mid_panel_1"] = sc_panel;

        auto label = return_Label("Genre", 30, 600, 50, panels["mid_panel_1"], "genre_label", "");

        Player player;
        player.read_from_file("songs_set.csv");
        map<string, vector<Song>> list = player.get_genre();
        sc_panel->getVerticalScrollbar()->setValue(10);
        int j = 0;
        for (auto& i : list)
        {
            auto button = return_Button("", 80, 80, 600 + (140 * j), 150, panels["mid_panel_1"], i.first, "background/cd.png");
            auto label = return_Label(i.first, 20, 580 + (150 * j), 250, panels["mid_panel_1"]);
            j++;

            vector < Song > s1 = list[i.first];
            button->onPress([=]
                {
                    panels["mid_panel_1"]->setVisible(false);
                    make_mid_panels_of_each_genre(i.first, s1);
                });
        }




        /*label = tgui::Label::create("Artists");
        label->setTextSize(30);
        label->setPosition(600, 500);
        panels["mid_panel_1"]->add(label);

        j = 0;
        list = player.get_artist();
        for (auto& i : list)
        {
            auto button = return_Button(i.first, 150, 50, 650, 600 + (60 * j++), panels["mid_panel_1"], "");
            button->getRenderer()->setRoundedBorderRadius(20);
            panels["mid_panel_1"]->add(button);
        }*/


        gui.add(sc_panel);

    }

    void play_song(Song song)
    {
        auto song_label = panels["play_panel"]->get<tgui::Label>("song_label");

        string file_path = "songs/" + song.id + ".mp3";


        if (!persistentPlayer->open(file_path))
        {
            song_label->setText("Failed to Open MP3 File");
            return;
        }
        persistentPlayer->play();
        song_label->setText(song.title);


        auto p_bar = panels["play_panel"]->get<tgui::Slider>("p_bar");
        p_bar->setValue(0);
        p_bar->setMaximum(song.duration);

        auto play_button = panels["play_panel"]->get<tgui::Button>("play");

        tgui::Texture t1("background/pause1.jpg");
        play_button->getRenderer()->setTexture(t1);
        play_button->setSize(40, 40);
        play_button->setPosition(945, 15);

        tgui::Texture t2("background/play1.png");

        play_button->onPress([=] {
            if (persistentPlayer->get_status())
            {
                persistentPlayer->pause();
                play_button->getRenderer()->setTexture(t2);
                play_button->setSize(30, 30);
                play_button->setPosition(950, 20);
            }

            else
            {
                persistentPlayer->play();
                play_button->getRenderer()->setTexture(t1);
                play_button->setSize(40, 40);
                play_button->setPosition(945, 15);
            }

            });
    }

    void make_mid_panels_of_each_genre(string genre, vector<Song> g_songs)
    {
        panels["main_mid_panel"]->setVisible(true);
        auto g_panel = ScrollablePanel::create({ 1500.f,820.f });
        g_panel->setPosition(420, 80);
        g_panel->getRenderer()->setBackgroundColor(tgui::Color::White);
        panels[genre] = g_panel;

        int j = 0;

        for (auto i : g_songs)
        {
            int col = j % 6;
            int row = j / 6;

            int pos_x = 80 + (col * 150);
            int pos_y = 150 + (row * 150);

            auto button = return_Button("", 80, 80, pos_x, pos_y, panels[genre], i.id, "background/cd.png");

            button->onPress([=]
                {
                    play_song(i);
                });

            auto label = return_Label(i.title, 13, pos_x - 5, pos_y + 80, panels[genre]);

            j++;
        }
        gui.add(panels[genre]);
        auto back_button = return_Button("", 50, 50, 350, 50, panels["main_mid_panel"], "back - button", "background/back.png");
        back_button->onPress([=]
            {
                panels["main_mid_panel"]->setVisible(false);
                panels[genre]->setVisible(false);
                panels["mid_panel_1"]->setVisible(true);

            });

    }

    void UI_template_Maker()
    {
        /*intro_panel();*/
        /*Animated_Text_Logo("Smart Music Player", 60, 750, 100,30);*/
        search_panel();
        play_panel();
        main_mid_panel();
        mid_panel_1();

    }
};

int main()
{
    UI_Functionality app;
    app.init_window();
    return 0;
}