#include "AudioDB.h"
#include "AudioHelper.h"
#include "SDL2_mixer/SDL_mixer.h"
#include <filesystem>
#include <iostream>
#include <string>

void AudioDB::Initialize() {
  AudioHelper::Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
  std::string path = "resources/audio";
  if (!std::filesystem::exists(path)) {
    return;
  }
  for (const auto &file : std::filesystem::directory_iterator(path)) {
    std::string audioName = file.path().stem().string();
    Mix_Chunk* audioChunk = AudioHelper::Mix_LoadWAV(file.path().string().c_str());
    audioCache[audioName] = audioChunk;
  }
  AudioHelper::Mix_AllocateChannels(50);
}



Mix_Chunk* AudioDB::GetAudio(const std::string &name) {
  auto it = audioCache.find(name);
  if (it == audioCache.end()) {
    std::cout << "error: failed to play audio clip " << name;
    exit(0);
  }
  return it->second;
}

void AudioDB::PlayAudio(int channel, const std::string &name, bool loops) {
  Mix_Chunk* audioChunk = GetAudio(name);
  int loopInt = loops ? -1 : 0;
  AudioHelper::Mix_PlayChannel(channel, audioChunk, loopInt);
}

void AudioDB::HaltChannel(int channel) {
  AudioHelper::Mix_HaltChannel(channel);
}

void AudioDB::SetVolume(int channel, int volume) {
  AudioHelper::Mix_Volume(channel, volume);
}
