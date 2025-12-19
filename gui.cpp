#include <iostream>
#include <TGUI/TGUI.hpp>
#include<TGUI/AllWidgets.hpp>
#include<TGUI/Widget.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
#include <sstream>
#include <mutex>
#include <mpg123.h>
#include <vector>
#include <memory>
#include <queue>
#include <stack>
#include "backup.cpp"
#include <algorithm>
#include <cmath>
#include <unordered_set>

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
    
public:
    Mp3Player() {}

    bool get_status() const { return (stream && stream->getStatus() == sf::SoundSource::Status::Playing); }

    bool open(const std::string& filename)
    {
        stream = std::make_unique<Mp3Stream>();
        if (!stream->open(filename))
            return false;
        return true;
    }

    void play()  { if (stream) stream->play();   }
    void pause() { if (stream) stream->pause();  }
    void stop()  { if (stream) stream->stop();   }
    void setVolume(float volume) { if (stream) stream->setVolume(volume); }
    void seek(double time) { sf::Time t = sf::seconds(time); if (stream) stream->onSeek(t); }

    void setDuration(float seconds) { durationSeconds = seconds; }
    float getDuration() const { return durationSeconds; }
};

class UI_Template
{
protected:
    map<string, Panel::Ptr> panels;

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

};

class UI_Functionality :public UI_Template
{
private:
    Gui gui;
    std::unique_ptr<Mp3Player> persistentPlayer;
    RenderWindow window;
    Player player;
    size_t userPlaylistCount = 0;
    string currentPlayingSongId;
    string previousHighlightedId;

public:

    UI_Functionality()
    {
        persistentPlayer = std::make_unique<Mp3Player>();
        player.read_from_file("songs_set_1.csv");
    }


    void showCreatePlaylistPopup()
    {
        // If popup already exists just bring it visible
        if (panels.count("create_playlist_popup"))
        {
            panels["create_playlist_popup"]->setVisible(true);
            return;
        }

        // small panel centered
        auto popup = Panel::create({400.f,140.f});
        popup->setPosition((1920.f - 400.f) / 2.f, (1080.f - 140.f) / 2.f);
        popup->getRenderer()->setBackgroundColor(tgui::Color::White);

        panels["create_playlist_popup"] = popup;

        auto edit = EditBox::create();
        edit->setDefaultText("");
        edit->setPosition(20, 20);
        edit->setSize(360, 30);
        popup->add(edit, "name_edit");

        auto ok = Button::create("OK");
        ok->setPosition(80, 70);
        ok->setSize(100, 30);
        popup->add(ok, "ok");

        auto cancel = Button::create("Cancel");
        cancel->setPosition(220, 70);
        cancel->setSize(100, 30);
        popup->add(cancel, "cancel");

        gui.add(popup);

        cancel->onPress([this, popup]() {
            popup->setVisible(false);
            gui.remove(popup);
            panels.erase("create_playlist_popup");
        });

        ok->onPress([this, edit, popup]() {
            std::string name = edit->getText().toStdString();
            if (name.empty())
            {
                edit->setText("Enter a valid name");
                return;
            }
            // create playlist in Player
            player.create_user_playlist(name);

            // remove popup cleanly
            popup->setVisible(false);
            gui.remove(popup);
            panels.erase("create_playlist_popup");

            // open song selector
            showAllSongsForPlaylistPanel(name);
        });
    }

    void showAllSongsForPlaylistPanel(const std::string& playlistName)
    {
        // create editor panel
        std::string key = "playlist_editor_" + playlistName;
        auto panel = ScrollablePanel::create({700.f, 600.f});
        panel->setPosition(200, 80);
        panel->getRenderer()->setBackgroundColor(tgui::Color::White);
        panels[key] = panel;
        gui.add(panel);

        auto title = Label::create("Add songs to: " + playlistName);
        title->setTextSize(18);
        title->setPosition(10, 10);
        panel->add(title);

        int y = 50;
        auto genres = player.get_genre();
        for (auto &g : genres)
        {
            for (auto &sid : g.second)
            {
                Song s = player.get_song(sid);
                auto chk = CheckBox::create();
                chk->setText(s.title + " - " + s.artist);
                chk->setPosition(10, y);
                chk->setSize(20, 24);
                panel->add(chk, "chk_" + sid);
                y += 28;
            }
        }

        // Save + Close buttons
        auto saveBtn = Button::create("Save");
        saveBtn->setPosition(10, y + 10);
        saveBtn->setSize(120, 30);
        panel->add(saveBtn);

        auto closeBtn = Button::create("Close");
        closeBtn->setPosition(140, y + 10);
        closeBtn->setSize(120, 30);
        panel->add(closeBtn);

        saveBtn->onPress([this, key, playlistName]() {
            auto panelPtr = panels[key];
            if (!panelPtr) return;
            // iterate widgets in panel and collect checked
            for (auto &w : panelPtr->getWidgets())
            {
                auto chk = std::dynamic_pointer_cast<CheckBox>(w);
                if (chk && chk->isChecked())
                {
                    // use getWidgetName() to obtain the widget name added via panel->add(...)
                    std::string name = chk->getWidgetName().toStdString();
                    if (name.rfind("chk_", 0) == 0)
                    {
                        std::string sid = name.substr(4);
                        player.add_song_to_user_playlist(playlistName, sid);
                    }
                }
            }

            // add playlist button to mid_panel_1 (playlist section)
            if (panels.count("mid_panel_1"))
            {
                gui.remove(panels["mid_panel_1"]);
                panels.erase("mid_panel_1");
                mid_panel_1();
                panels["mid_panel_1"]->setVisible(true);
            }

            // persist playlists to file
            player.write_user_playlist("user_playlists.csv");

            // hide editor
            panelPtr->setVisible(false);
    });

    closeBtn->onPress([this, key]() {
        if (panels.count(key)) panels[key]->setVisible(false);
    });
}

    void highlightSong(const string &songId)
    {
        if (previousHighlightedId == songId) return;

        // clear previous
        if (!previousHighlightedId.empty())
        {
            for (auto &kv : panels)
            {
                try
                {
                    auto b = kv.second->get<tgui::Button>(previousHighlightedId);
                    if (b) b->getRenderer()->setBorderColor(tgui::Color::White);
                }
                catch (...) {}
            }
        }

        previousHighlightedId.clear();

        if (!songId.empty())
        {
            for (auto &kv : panels)
            {
                try
                {
                    auto b = kv.second->get<tgui::Button>(songId);
                    if (b)
                    {
                        b->getRenderer()->setBorderColor(tgui::Color::Red);
                        previousHighlightedId = songId;
                    }
                }
                catch (...) {}
            }
        }
    }


    void current_playlist_panel(string playlist_name="", vector<string> songs={})
    {
        // Remove existing panel if present so updates refresh cleanly
        if (panels.count("current_playlist_panel"))
        {
            gui.remove(panels["current_playlist_panel"]);
            panels.erase("current_playlist_panel");
        }

        auto c_panel = ScrollablePanel::create({ 420.f,820.f });
        c_panel->setPosition(1500, 80);
        c_panel->getRenderer()->setBackgroundColor(tgui::Color::White);
        c_panel->setVisible(true);
        panels["current_playlist_panel"] = c_panel;
        gui.add(c_panel);

        // Title
        if(playlist_name=="")
        auto title = return_Label("Current Playlist", 25, 50, 80, c_panel, "curr_title");
        else
         auto title = return_Label(playlist_name, 25, 50, 80, c_panel, "curr_title");
        // No playlist playing -> show message
        if (!player.any_playlist_playing())
        {
            auto no = return_Label("No active playlist", 20, 50, 120, c_panel, "curr_none");
            return;
        }
        else
        {
            int j = 0;
            for (auto& i : songs)
            {
                string song_name = player.get_song_title(i);
                auto label = return_Label(song_name, 20, 50, (j * 40) + 120, panels["current_playlist_panel"], i, "");
                j++;
            }
        }
        highlight_playlist_panel_song(player.get_current_playlist_song_id());
    }

    void make_mid_playlist_panels(const std::string& playlistName)
    {
        // quick wrapper to open playlist as a panel similar to genre panels
        make_mid_panels(playlistName, player.get_user_playlist(playlistName));

        Panel::Ptr g_panel = panels[playlistName];

        auto del = Button::create("Delete Playlist");
        del->setPosition(420, 50);
        del->setSize(150, 40);
        g_panel->add(del, "delete_playlist_btn_" + playlistName);

        del->onPress([this, playlistName]() {
            // remove from player (map + persist)
            player.delete_user_playlist(playlistName);
            player.write_user_playlist("user_playlists.csv");
            player.clear_current_playlist();
            persistentPlayer->stop();
            persistentPlayer->pause();
            gui.remove(panels[playlistName]);
            panels.erase(playlistName);
            gui.remove(panels["mid_panel_1"]);
            panels.erase("mid_panel_1");
            mid_panel_1();
            if (panels.count("mid_panel_1")) panels["mid_panel_1"]->setVisible(true);
            });
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

        auto p_bar = panels["play_panel"]->get<tgui::Slider>("p_bar");
        auto label = panels["play_panel"]->get<tgui::Label>("time_label_1");

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
                    stringstream ss(label->getText().toStdString());
                    string min;
                    string sec;
                    getline(ss, min, ':');
                    getline(ss, sec, ':');
                    int m = stoi(min);
                    int s = stoi(sec);
                    int v = (m * 60) + s + 1;
                    string duration = to_string(v / 60) + ":" + to_string(v % 60);
                    label->setText(duration);
                    if (p_bar->getValue() < p_bar->getMaximum())
                    {
                        p_bar->setValue(p_bar->getValue()+1.0f);
                    }
                    else
                    {
                        if (player.any_playlist_playing())
                            persistentPlayer->stop();
                        else
                        {
                            persistentPlayer->stop();
                            play_song(player.get_song(player.play_next_song_of_current_playlist()));
                        }
                    }
                    clock.restart();
                }
            }

            window.clear(sf::Color(0,50,25));
            gui.draw();
            window.display();
        }
        gui.removeAllWidgets();
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

        search->onFocus([=]
            {
                string s_name = search->getText().toStdString();
                vector<string> s_names = player.search_songs_by_prefix(s_name);
                auto panel = ScrollablePanel::create({ 450.f, 500.f });
                panel->setPosition(720, 80);
                panel->getRenderer()->setBackgroundColor(tgui::Color::White);
                panels["search_sub_panel"] = panel;
                gui.add(panel);
                panel->setVisible(true);


                int j = 0;
                Song s;

                if (s_names.empty())
                {
                    auto no = return_Label("No Song Found", 16, 20, 20, panels["search_sub_panel"]);
                }
                for (auto i : s_names)
                {
                    s = player.get_song(i);
                    auto button = return_Button(s.title, 150, 30, 40, (40 * j++) + 40, panels["search_sub_panel"], s.id);
                    /*auto title = return_Label(s.title, 12, 240 - 5, 500+ 8, panels["search_sub_panel"]);*/
                }
            });

        search->onTextChange([=]
            {
                if (panels.count("search_sub_panel"))
                {
                    gui.remove(panels["search_sub_panel"]);
                    panels.erase("search_sub_panel");
                }

                string s_name=search->getText().toStdString();
                vector<string> s_names=player.search_songs_by_prefix(s_name);
                auto panel = ScrollablePanel::create({ 450.f, 500.f });
                panel->setPosition(720, 80);
                panel->getRenderer()->setBackgroundColor(tgui::Color::White);
                panels["search_sub_panel"] = panel;
                gui.add(panel);
                panel->setVisible(true);

                
                int j = 0;
                Song s;

                if (s_names.empty())
                {
                    auto no = return_Label("No Song Found", 16, 20, 20, panels["search_sub_panel"]);
                }
                for (auto i : s_names)
                {
                    s = player.get_song(i);
                    auto button = return_Button(s.title, 150, 30, 40, (40 * j++) + 40, panels["search_sub_panel"], s.id);
                    /*auto title = return_Label(s.title, 12, 240 - 5, 500+ 8, panels["search_sub_panel"]);*/
                }
            });

        search->onUnfocus([=]
            {
                if (panels.count("search_sub_panel"))
                {
                    gui.remove(panels["search_sub_panel"]);
                    panels.erase("search_sub_panel");
                }
            });

        search->onReturnKeyPress([=]
            {
                string s_name = search->getText().toStdString();
                vector<string> s_names = player.search_songs_by_prefix(s_name);
                make_mid_panels("search", s_names);
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
        auto time_label_1 = return_Label("-:-", 15, 650, 55, panels["play_panel"], "time_label_1");
        auto progressBar = return_Slider(520, 10, 700, 60, panels["play_panel"], "p_bar");
        auto time_label_2 = return_Label("-:-", 15, 1230, 55, panels["play_panel"], "time_label_2");

        tgui::Texture t1("background/pause1.jpg");
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

        prev_button->onPress([=]
            {
                string song_id = player.play_prev_song_of_current_playlist();
                if (song_id != "")
                {
                    Song song = player.get_song(song_id);
                    play_song(song);
                }
                highlight_playlist_panel_song(song_id);
            });

        next_button->onPress([=]
            {
                string song_id = player.play_next_song_of_current_playlist();
                if (song_id != "")
                {
                    Song song = player.get_song(song_id);
                    play_song(song);
                }
                highlight_playlist_panel_song(song_id);
            });

        progressBar->onMouseEnter([=]
            {
                progressBar->onValueChange([=]
                    {
                        persistentPlayer->seek(static_cast<double>(progressBar->getValue()));
                        int v = progressBar->getValue();
                        string duration = to_string(v / 60) + ":" + to_string(v % 60);
                        time_label_1->setText(duration);
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

        auto sc_panel = ScrollablePanel::create({ 420.f,820.f });
        sc_panel->setPosition(0, 80);
        sc_panel->getRenderer()->setBackgroundColor(tgui::Color::White);
        panels["main_mid_panel"] = sc_panel;

        sc_panel->setVisible(true);
        gui.add(panels["main_mid_panel"]);

        if (!sc_panel->get< Button >("create_playlist_btn"))
        {
            auto btn = return_Button("Create Playlist", 180, 40, 30, 160, sc_panel, "create_playlist_btn");
            btn->onPress([this]() {
                showCreatePlaylistPopup();
                });
        }

        // Smart Play button: plays recommended songs similar to currently playing song
        if (!sc_panel->get< Button >("smart_play_btn"))
        {
            auto smart = return_Button("Smart Play", 180, 40, 230, 160, sc_panel, "smart_play_btn");
            smart->onPress([=]()
                {
                    if (player.any_playlist_playing())
                    {
                        vector<string> rec_id = player.recommend_song_regarding_to_current_song();
                        for (auto& i : rec_id)
                        {
                            if (!player.song_exists_in_current_playlist(i))
                            {
                                player.add_song_to_current_playlist(i);
                                break;
                            }

                        }
                        
                        string name = panels["current_playlist_panel"]->get<Label>("curr_title")->getText().toStdString();
                        current_playlist_panel(name, player.get_current_playlist());
                        
                    }
            });
        }
    }

    void play_playlist(vector<string> playlist)
    {
        
        player.add_current_playlist_at_once(playlist);
        
        Song curr = player.get_song(player.start_current_playlist());
        play_song(curr);

        
    }



    void mid_panel_1()
    {
        auto sc_panel = ScrollablePanel::create({ 1080.f,820.f });
        sc_panel->setPosition(420, 80);
       
        sc_panel->getRenderer()->setBackgroundColor(tgui::Color::White);
        panels["mid_panel_1"] = sc_panel;

        auto label = return_Label("Genre", 30, 150, 50, panels["mid_panel_1"], "genre_label", "");


        map<string, vector<string>> list = player.get_genre();
        sc_panel->getVerticalScrollbar()->setValue(10);
        int j = 0;
        for (auto& i : list)
        {
            int col = j % 5;
            int row = j / 5;

            int pos_x = 150 + (col * 150);
            int pos_y = 150 + (row * 150);

            auto button = return_Button("", 80, 80, pos_x, pos_y, panels["mid_panel_1"], i.first, "background/cd.png");
            auto label = return_Label(i.first, 20, pos_x+20, pos_y+100, panels["mid_panel_1"]);

            j++;

            vector < string > s1 = list[i.first];
            button->onPress([=]
                {
                    panels["mid_panel_1"]->setVisible(false);
                    make_mid_panels(i.first, s1);
                });
        }

        map < string, vector<string>> m = player.get_user_playlists();
        auto mid = panels["mid_panel_1"];
        label = return_Label("Playlists", 30, 150, 350, panels["mid_panel_1"], "playlist_label", "");
        int userPlaylistCount = 0;
        for (auto& i : m)
        {
            string playlistName = i.first;
            int pos_x = 150 + (userPlaylistCount % 5) * 150;
            int pos_y = 450 + (userPlaylistCount / 5) * 120;
            auto btn = return_Button("", 80, 80, pos_x, pos_y, panels["mid_panel_1"], "playlist_btn_" + playlistName, "background/cd.png");
            btn->getRenderer()->setBorderColor(tgui::Color::White);
            label = return_Label(playlistName, 20, pos_x + 20, pos_y + 100, panels["mid_panel_1"]);
            btn->onPress([this, playlistName]() {
                

                make_mid_playlist_panels(playlistName);
                panels["mid_panel_1"]->setVisible(false);
                if (panels.count(playlistName)) panels[playlistName]->setVisible(true);
                player.clear_current_playlist();
                play_playlist(player.get_user_playlist(playlistName));
                current_playlist_panel(playlistName, player.get_user_playlist(playlistName));
               
  
                });
            userPlaylistCount++;
        }

        gui.add(sc_panel);

    }

    void highlight_playlist_panel_song(string id)
    {
        if (panels.count("current_playlist_panel"))
        {
            if (panels["current_playlist_panel"]->get<Label>("curr_title")->getText().toStdString() != "Current Playlist")
            {
                panels["current_playlist_panel"]->get<Label>(player.get_current_playlist_song_id())->getRenderer()->setTextColor(tgui::Color::Blue);
                for (auto& i : player.get_current_playlist())
                {
                    if (i != id)
                    {
                        panels["current_playlist_panel"]->get<Label>(i)->getRenderer()->setTextColor(tgui::Color::Black);
                    }
                }
            }
        }
    }

    void play_song(Song song)
    {
        auto song_label = panels["play_panel"]->get<tgui::Label>("song_label");

        auto label = panels["play_panel"]->get<tgui::Label>("time_label_1");
        label->setText("00:00");

        string duration = to_string(song.duration / 60) + ":" + to_string(song.duration % 60);
        label = panels["play_panel"]->get<tgui::Label>("time_label_2");
        label->setText(duration);

        string file_path = "songs/" + song.id + ".mp3";


        if (!persistentPlayer->open(file_path))
        {
            song_label->setText("Failed to Open MP3 File");
            return;
        }
        persistentPlayer->play();
        song_label->setText(song.title);

        // update highlighting
        currentPlayingSongId = song.id;
        highlightSong(song.id);

        auto p_bar = panels["play_panel"]->get<tgui::Slider>("p_bar");
        p_bar->setValue(0);
        p_bar->setMaximum(song.duration);

        auto play_button = panels["play_panel"]->get<tgui::Button>("play");

        tgui::Texture t1("background/pause1.jpg");
        play_button->getRenderer()->setTexture(t1);
        play_button->setSize(40, 40);
        play_button->setPosition(945, 15);
        
        
    }

    void switch_back_to_main_panel(string s_panel_name)
    {
        Button::Ptr back_button;
        if (!panels["main_mid_panel"]->get<Button>("back - button"))
        {
           back_button = return_Button("", 50, 50, 350, 50, panels["main_mid_panel"], "back - button", "background/back.png");
        }
        else
        {
            back_button = panels["main_mid_panel"]->get<Button>("back - button");
        }

        back_button->setVisible(true);
        
        back_button->onPress([=]
            {
               /* gui.remove(panels[s_panel_name]);
                panels.erase(s_panel_name);*/
                /*panels["search"]->setVisible(false);*/
                panels["mid_panel_1"]->setVisible(true);
                panels[s_panel_name]->setVisible(false);
            });
    }

    void make_mid_panels(string s_panel_name, vector<string> songs)
    {
        panels["main_mid_panel"]->setVisible(true);
        panels["mid_panel_1"]->setVisible(false);

        if (panels.count(s_panel_name) == 0)
        {
            auto g_panel = ScrollablePanel::create({ 1080.f,820.f });
            g_panel->setPosition(420, 80);
            g_panel->getRenderer()->setBackgroundColor(tgui::Color::White);
            panels[s_panel_name] = g_panel;

            if (songs.empty())
            {
                auto label = return_Label("No Songs Found", 30, 200, 200, g_panel);
                return;
            }

            int j = 0;

            for (auto i : songs)
            {
                int col = j % 6;
                int row = j / 6;

                int pos_x = 80 + (col * 150);
                int pos_y = 150 + (row * 150);

                Song s = player.get_song(i);

                auto button = return_Button("", 80, 80, pos_x, pos_y, panels[s_panel_name], s.id, "background/cd.png");


                button->onPress([=]
                    {
                        play_song(s);
                        
                    });

                switch_back_to_main_panel(s_panel_name);
                auto label = return_Label(s.title, 13, pos_x - 5, pos_y + 80, panels[s_panel_name]);

                j++;
            }
            gui.add(panels[s_panel_name]);
        }
        
        else
        {
            panels[s_panel_name]->setVisible(true);
        }
        

    }

    void UI_template_Maker()
    {
        search_panel();
        play_panel();
        main_mid_panel();
        mid_panel_1();
        current_playlist_panel();
    }
};

int main()
{
    UI_Functionality app;
    app.init_window();
    return 0;
}