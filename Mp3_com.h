#pragma once
#include<iostream>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
#include <mpg123.h>


using namespace std;


class Mp3Stream : public sf::SoundStream
{

private:

    mpg123_handle* mh;
    size_t bufferSize ;
    std::vector<unsigned char> buffer;
    size_t samplesProcessed;

    long rate ;
    int channels ;
    int encoding ;
    int samplecount;
public:
    Mp3Stream();
    bool open(const std::string& filename);
    virtual void onSeek(sf::Time timeOffset) override;
    ~Mp3Stream();

protected:

    virtual bool onGetData(Chunk& data) override;
};


class Mp3Player
{
private:
    std::unique_ptr<Mp3Stream> stream;
    float durationSeconds = 0.f;

public:
    Mp3Player();
    bool get_status() const;
    bool open(const std::string& filename);
    void play();
    void pause();
    void stop();
    void setVolume(float volume);
    void seek(double time);
    sf::Time getOffset() const;
    void setDuration(float seconds);
    float getDuration();
};