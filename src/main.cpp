#include <Geode/Geode.hpp>
#include <prevter.imageplus/include/api.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/utils/string.hpp>
#include <Geode/loader/GameEvent.hpp>
#include <Geode/loader/SettingV3.hpp>

using namespace geode::prelude;

auto playSFX = true;
auto useGif = false;

$on_mod(Loaded) {
    auto mod = Mod::get();

    playSFX = mod->getSettingValue<bool>("playsound");
    listenForSettingChanges<bool>("playsound", [](bool value) {
        playSFX = value;
    }, mod);

    useGif = mod->getSettingValue<bool>("customimages");
    listenForSettingChanges<bool>("customimages", [](bool value) {
        useGif = value;
    }, mod);
}

$on_game(Loaded) {
    listenForKeybindSettingPresses("throw-keybind", [](Keybind const& keybind, bool down, bool repeat, double time) {
        if (down) { 
            auto director = CCDirector::sharedDirector();
            auto scene = director->getRunningScene();
            if (!scene) return;
            
            auto mousePos = geode::cocos::getMousePos();
            auto cTomatoImage = Mod::get()->getSettingValue<std::filesystem::path>("ctomatoimage");

            if (!useGif || cTomatoImage.empty()) {
                auto tomatoThrow = CCSprite::create("tomatothrow.webp"_spr);
                if (!tomatoThrow) {
                    log::error("failed to load gif");
                    return;
                }

                auto tomato = imgp::AnimatedSprite::from(tomatoThrow);
                tomato->setScale(2.0f); 
                scene->addChild(tomato, 999999); 
                tomato->setPosition(mousePos);

                tomato->setForceLoop(false);
                tomato->setCurrentFrame(0);
                tomato->play();

                tomato->runAction(CCSequence::create(
                    CCDelayTime::create(0.47f),
                    CallFuncExt::create([]() {
                        if (playSFX) {
                            FMODAudioEngine::get()->playEffect("splat.mp3"_spr);
                        }
                    }),
                    CCDelayTime::create(0.3f),
                    CallFuncExt::create([tomato]() {
                        tomato->pause();
                        tomato->unscheduleUpdate();
                    }),
                    CCDelayTime::create(3.0f),
                    CCFadeOut::create(1.f),
                    CCRemoveSelf::create(),
                    nullptr
                ));

                return;
            } else {
                auto pathStr = geode::utils::string::pathToString(cTomatoImage);
                auto tomato = CCSprite::create(pathStr.c_str());

                if (!tomato) {
                    log::error("failed to load custom tomato image");
                    return;
                }

                auto ctScale = Mod::get()->getSettingValue<double>("ctomatoscale");
                auto endScale = Mod::get()->getSettingValue<double>("ctomatoendingscale");

                auto winSize = director->getWinSize();
                CCPoint startPos = ccp(winSize.width / 2.f, -tomato->getContentSize().height);

                tomato->setPosition(startPos);
                tomato->setScale(static_cast<float>(ctScale));
                scene->addChild(tomato, 999999);

                float totalTime = 0.35f;
                float peakScale = static_cast<float>(ctScale) * 1.3f;

                auto move = CCEaseSineOut::create(CCMoveTo::create(totalTime, mousePos));
                auto scaleSeq = CCSequence::create(
                    CCEaseSineOut::create(CCScaleTo::create(totalTime / 2.0f, peakScale)),
                    CCEaseSineIn::create(CCScaleTo::create(totalTime / 2.0f, static_cast<float>(endScale))),
                    nullptr
                );

                tomato->runAction(CCSequence::create(
                    CCSpawn::create(move, scaleSeq, nullptr),
                    CallFuncExt::create([tomato]() { 
                        if (playSFX) {
                            FMODAudioEngine::get()->playEffect("splat.mp3"_spr);
                        }

                        auto splatPath = Mod::get()->getSettingValue<std::filesystem::path>("ctomatosplatimage");
                        auto splatScale = Mod::get()->getSettingValue<double>("ctomatosplatscale");
                        auto splatPathStr = geode::utils::string::pathToString(splatPath);
                        auto texture = CCTextureCache::sharedTextureCache()->addImage(splatPathStr.c_str(), false);
                        
                        if (texture) {
                            tomato->setTexture(texture);
                            tomato->setTextureRect({0, 0, texture->getContentSize().width, texture->getContentSize().height});
                        }
                        tomato->setScale(static_cast<float>(splatScale));
                    }),
                    CCDelayTime::create(3.0f),
                    CCFadeOut::create(1.f),
                    CCRemoveSelf::create(),
                    nullptr
                ));
            }
        }
    }); 
}