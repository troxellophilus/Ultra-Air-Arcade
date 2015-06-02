#ifndef PLANESOUND_HPP
#define PLANESOUND_HPP

#include <SFML/Audio.hpp>

using namespace std;

class PlaneSound {
private:
    sf::SoundBuffer buffer;
    sf::Sound sound;
    
public:
    PlaneSound();
    PlaneSound(std::string filename);
    virtual ~PlaneSound();
    void play();
    void playLooped();
    void stop();
};

PlaneSound::PlaneSound() { }

PlaneSound::PlaneSound(std::string filename) {
	buffer = sf::SoundBuffer();
	sound = sf::Sound();

    if (!buffer.loadFromFile(filename)) {
        cout << "Error loading " << filename << endl;
    }
    sound.setBuffer(buffer);
}

PlaneSound::~PlaneSound() { }

void PlaneSound::play() {
    sound.setLoop(false);
    sound.play();
}

void PlaneSound::playLooped() {
    sound.setLoop(true);
    sound.play();
}

void PlaneSound::stop() {
    sound.stop();
}

#endif
