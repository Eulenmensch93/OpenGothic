﻿#include "gamemusic.h"

#include <Tempest/Sound>
#include <Tempest/Log>

#include "dmusic/mixer.h"
#include "resources.h"

using namespace Tempest;

struct GameMusic::MusicProducer : Tempest::SoundProducer {
  MusicProducer():SoundProducer(44100,2){
    }

  void renderSound(int16_t* out,size_t n) override {
    updateTheme();
    mix.mix(out,n);
    }

  void updateTheme() {
    Daedalus::GEngineClasses::C_MusicTheme theme;
    bool                                   updateTheme=false;
    bool                                   reloadTheme=false;

    {
      std::lock_guard<std::mutex> guard(pendingSync);
      if(hasPending) {
        hasPending  = false;
        updateTheme = true;
        reloadTheme = this->reloadTheme;
        theme       = pendingMusic;
        }
    }

    if(!updateTheme)
      return;
    updateTheme = false;

    try {
      if(reloadTheme) {
        Dx8::PatternList p = Resources::loadDxMusic(theme.file.c_str());

        const char* tagsStr="Std";
        if(pendingTags&Tags::Fgt)
          tagsStr="Fgt";
        else if(pendingTags&Tags::Thr)
          tagsStr="Thr";

        Dx8::Music m;
        if(!fillPattern(m,p,tagsStr))
          m.addPattern(p);

        // m.setVolume(theme.vol);
        mix.setMusic(m);
        }
      mix.setMusicVolume(theme.vol);
      }
    catch(std::runtime_error&) {
      Log::e("unable to load sound: \"",theme.file.c_str(),"\"");
      }
    }

  bool fillPattern(Dx8::Music& m,const Dx8::PatternList& p,const char* tag){
    for(size_t i=0;i<p.size();++i) {
      auto& pat = p[i];
      if(pat.name.find(tag)!=std::string::npos)
        m.addPattern(p,i);
      }
    return m.size()!=0;
    }

  bool setMusic(const Daedalus::GEngineClasses::C_MusicTheme &theme, Tags tags){
    std::lock_guard<std::mutex> guard(pendingSync);
    reloadTheme  = pendingMusic.file!=theme.file;
    pendingMusic = theme;
    pendingTags  = tags;
    hasPending   = true;
    return true;
    }

  void stopMusic() {
    std::lock_guard<std::mutex> guard(pendingSync);
    mix.setMusic(Dx8::Music());
    }

  void setVolume(float v) {
    mix.setVolume(v);
    }

  Dx8::Mixer                                    mix;

  std::mutex                                    pendingSync;
  bool                                          hasPending=false;
  bool                                          reloadTheme=false;
  Daedalus::GEngineClasses::C_MusicTheme        pendingMusic;
  Tags                                          pendingTags=Tags::Day;
  };

struct GameMusic::Impl final {
  Impl() {
    std::unique_ptr<MusicProducer> mix(new MusicProducer());
    dxMixer = mix.get();
    sound   = device.load(std::move(mix));

    dxMixer->setVolume(masterVolume);
    }

  void setMusic(const Daedalus::GEngineClasses::C_MusicTheme &theme, Tags tags) {
    if(!dxMixer->setMusic(theme,tags))
      return;
    sound.play();
    }

  void stopMusic() {
    dxMixer->stopMusic();
    }

  Tempest::SoundDevice                          device;
  Tempest::SoundEffect                          sound;

  MusicProducer*                                dxMixer=nullptr;
  float                                         masterVolume=0.5f;
  bool                                          enableMusic=true;
  };

GameMusic::GameMusic() {
  impl.reset(new Impl());
  }

GameMusic::~GameMusic() {
  }

GameMusic::Tags GameMusic::mkTags(GameMusic::Tags daytime, GameMusic::Tags mode) {
  return Tags(daytime|mode);
  }

void GameMusic::setEnabled(bool e) {
  impl->enableMusic = e;
  if(!e)
    impl->stopMusic();
  }

bool GameMusic::isEnabled() const {
  return impl->enableMusic;
  }

void GameMusic::setMusic(const Daedalus::GEngineClasses::C_MusicTheme &theme, Tags tags) {
  if(!impl->enableMusic)
    return;
  impl->setMusic(theme,tags);
  }

void GameMusic::stopMusic() {
  impl->stopMusic();
  }
