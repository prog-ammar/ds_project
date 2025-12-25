
#include "Mp3_com.h"


Mp3Stream::Mp3Stream() {
    mh = nullptr;
    bufferSize = 0;

    rate = 0;
     channels = 0;
    encoding = 0;
     samplecount = 0;
    mpg123_init();
    int err = MPG123_OK;

    mh = mpg123_new(NULL, &err);
    if (!mh)
        cout << "Erorr in opeing handle";
}

bool Mp3Stream::open(const std::string& filename)
{


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
    initialize(static_cast<unsigned int>(channels), static_cast<unsigned int>(rate), {});
    samplecount = channels * length;
    bufferSize = mpg123_outblock(mh);
    buffer.resize(bufferSize);

    return true;
}

void Mp3Stream::onSeek(sf::Time timeOffset) 
{
    if (!mh)
        return;
    SoundSource::Status old = getStatus();

    if (SoundSource::Status::Playing == old)
    {
        pause();
    }

    double t = timeOffset.asSeconds();
    // Convert requested time -> decoded PCM sample offset (interleaved samples).
    const off_t new_offset =
        mpg123_seek(mh, static_cast<off_t>(t * rate), SEEK_SET);
    if (new_offset < 0)
    {
        sf::err() << "Failed to seek with mpg123: "
            << mpg123_plain_strerror(static_cast<int>(new_offset)) << std::endl;
    }
    else
    {
        initialize(channels, rate, {});
        samplesProcessed = new_offset;

    }

    if (SoundSource::Status::Playing == old)
    {
        play();
    }

}

Mp3Stream::~Mp3Stream()
{
    if (mh)
    {
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
    }
}


bool Mp3Stream::onGetData(Chunk& data) 
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




Mp3Player::Mp3Player() {}

bool Mp3Player::get_status() const { return (stream && stream->getStatus() == sf::SoundSource::Status::Playing); }

bool  Mp3Player::open(const std::string& filename)
{
    stream = std::make_unique<Mp3Stream>();
    if (!stream->open(filename))
        return false;
    return true;
}

void  Mp3Player::play() { if (stream) stream->play(); }
void  Mp3Player::pause() { if (stream) stream->pause(); }
void  Mp3Player::stop() { if (stream) stream->stop(); }
void  Mp3Player::setVolume(float volume) { if (stream) stream->setVolume(volume); }
void  Mp3Player::seek(double time) { sf::Time t = sf::seconds(time); if (stream) stream->setPlayingOffset(t); }

sf::Time  Mp3Player::getOffset() const
{
    if (stream)
        return stream->getPlayingOffset();
    return sf::Time::Zero;
}

void  Mp3Player::setDuration(float seconds) { durationSeconds = seconds; }
float  Mp3Player::getDuration() { return durationSeconds; }