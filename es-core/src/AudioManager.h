#pragma once
#ifndef ES_CORE_AUDIO_MANAGER_H
#define ES_CORE_AUDIO_MANAGER_H

#include <SDL_audio.h>
#include <memory>
#include <vector>

class Sound;

class AudioManager
{
	static SDL_AudioSpec sAudioFormat;
	static std::vector<std::shared_ptr<Sound>> sSoundVector;
	static std::shared_ptr<AudioManager> sInstance;

	static void mixAudio(void *unused, Uint8 *stream, int len);

	AudioManager();

public:
	static std::shared_ptr<AudioManager> & getInstance();

	void init();
	void deinit();

	void registerSound(std::shared_ptr<Sound> & sound);
	void unregisterSound(std::shared_ptr<Sound> & sound);

	void play();
	void stop();

	virtual ~AudioManager();

	//añadido

	float mMusicVolume;
	int mVideoPlaying;

	static void setVideoPlaying(bool state);
	static void update(int deltaTime);
};

#endif // ES_CORE_AUDIO_MANAGER_H
