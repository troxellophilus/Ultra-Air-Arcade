#ifndef PLANESOUND_HPP
#define PLANESOUND_HPP

#include <SFML/Audio.hpp>

using namespace std;

class PlaneSound {
private:
    sf::SoundBuffer buffer; // The buffre for the sound object
    sf::Sound sound; // The actual sound to be played
    float pitch = 1.f; // The pitch of the sound
    
public:
    PlaneSound();
    PlaneSound(std::string filename);
    virtual ~PlaneSound();
    void play();
    void playLooped();
    void stop();
    void changePitch(int upOrDown);
    void setPitch(float newPitch);
    void setVolume(float newVolume);
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

void PlaneSound::changePitch(int upOrDown) {
    // Brings up the pitch to a limit when the player is speeding up
    if (upOrDown == 1) {
        if (pitch < 2.f) {
            pitch += .025f;
        }
        sound.setPitch(pitch);
    }
    // Brings down the pitch to a limit when the player is slowing down
    else if (upOrDown == -1) {
        if (pitch > 0.3f) {
            pitch -= .025f;
        }
        sound.setPitch(pitch);
    }
    // Brings the pitch back to normal
    else {
        if (pitch > 1.0f) {
            pitch -= .025f;
        }
        if (pitch < 1.0f) {
            pitch += .025f;
        }
        sound.setPitch(pitch);
    }
}

void PlaneSound::setPitch(float newPitch) {
    sound.setPitch(newPitch);
}

void PlaneSound::setVolume(float newVolume) {
    sound.setVolume(newVolume);
}

#endif
