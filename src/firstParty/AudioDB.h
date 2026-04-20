#ifndef AUDIODB_H
#define AUDIODB_H

#include "SDL2_mixer/SDL_mixer.h"
#include <string>
#include <unordered_map>
class AudioDB {
private:

  inline static std::unordered_map<std::string, Mix_Chunk*> audioCache;

public:
  static void Initialize();

  static Mix_Chunk* GetAudio(const std::string &name);

  static void PlayAudio(int channel, const std::string &name, bool loops);

  static void HaltChannel(int channel);

  static void SetVolume(int channel, int volume);
};

#endif
