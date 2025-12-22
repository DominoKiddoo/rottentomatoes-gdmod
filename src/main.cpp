#include <Geode/Geode.hpp>
#include <geode.custom-keybinds/include/Keybinds.hpp>
#include <prevter.imageplus/include/api.hpp>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;
using namespace keybinds;
auto playSFX = true;

auto useGif = false;

$on_mod(Loaded) {
	auto mod = Mod::get();

    // for whatever reason i have to do it this way cuz just getting it in the execute block doesnt work :(

    playSFX = mod->getSettingValue<bool>("playsound");
	listenForSettingChangesV3<bool>("playsound", [](bool value) {
		playSFX = value;
	}, mod);


    useGif = mod->getSettingValue<bool>("customimages");
	listenForSettingChangesV3<bool>("customimages", [](bool value) {
		useGif = value;
	}, mod);

}

$execute {
    const char* BIND_ID = "dominodev.rottentomatoes/throw-tomato";

    BindManager::get()->registerBindable({
        BIND_ID,
        "Throw Tomato",
        "Throws a tomato at the screen.", 
        { Keybind::create(KEY_T, Modifier::Alt) }, 
        "Rotten Tomatoes"
    });

    new EventListener(+[](InvokeBindEvent* event) {
        if (event->isDown()) {
            auto director = CCDirector::sharedDirector();
            auto scene = director->getRunningScene();
            if (!scene) return ListenerResult::Propagate;
            auto tomatoThrow = CCSprite::create("tomatothrow.gif"_spr);

            if (!tomatoThrow) {
                log::error("failed to load gif");
                return ListenerResult::Propagate;
            }
            
            auto mousePos = geode::cocos::getMousePos();
            if (!useGif) {
                auto tomato = imgp::AnimatedSprite::from(tomatoThrow);
                log::info("Tomato thrown!");

                tomato->setScale(2.0f); 
                scene->addChild(tomato, 999999); 
                tomato->setPosition(mousePos);

                tomato->setForceLoop(false);
                tomato->setCurrentFrame(0);
                tomato->play();

                tomato->runAction(CCSequence::create(
                    CCDelayTime::create(0.47f), // i completly guessed that timing and it works so yeah
                    CallFuncExt::create([tomato]() {
                        if (playSFX) {
                            FMODAudioEngine::get()->playEffect("splat.mp3"_spr);
                        }
                    }),
                    
                    CCDelayTime::create(0.3f),

                    CallFuncExt::create([tomato]() {
                    tomato->stop();
                    tomato->unscheduleUpdate();
                    }),

                    CCDelayTime::create(3.0f),
                    CCFadeOut::create(1.f),

                    CallFuncExt::create([tomato]() {
                        tomato->removeFromParentAndCleanup(true);
                    }),

                    nullptr
                ));


                return ListenerResult::Propagate;
            } else {
                log::info("Running custom tomato logic");

                auto cTomatoImage = Mod::get()->getSettingValue<std::filesystem::path>("ctomatoimage");
                auto tomato = CCSprite::create(cTomatoImage.string().c_str());

                if (!tomato) {
                    log::error("failed to load custom tomato image");
                    return ListenerResult::Propagate;
                }

                auto ctScale = Mod::get()->getSettingValue<double>("ctomatoscale");
                auto endScale = Mod::get()->getSettingValue<double>("ctomatoendingscale");

                auto director = CCDirector::sharedDirector();
                auto winSize = director->getWinSize();

                CCPoint startPos = ccp(winSize.width / 2.f, -tomato->getContentSize().height);
                CCPoint endPos = mousePos;

                tomato->setPosition(startPos);
                tomato->setScale(static_cast<float>(ctScale));
                scene->addChild(tomato, 999999);

                float totalTime = 0.35f;
                float halfTime = totalTime / 2.0f;

                float peakScale = static_cast<float>(ctScale) * 1.3f;

                auto rawMove = CCMoveTo::create(totalTime, endPos);
                auto move = CCEaseSineOut::create(rawMove);

                auto tomatoSplatTexture = Mod::get()->getSettingValue<std::filesystem::path>("ctomatosplatimage");
                auto tomatoSplatScale = Mod::get()->getSettingValue<double>("ctomatosplatscale");
                
                auto scaleUpRaw = CCScaleTo::create(halfTime, peakScale);
                auto scaleUp = CCEaseSineOut::create(scaleUpRaw);

                auto scaleDownRaw = CCScaleTo::create(halfTime, static_cast<float>(endScale));
                auto scaleDown = CCEaseSineIn::create(scaleDownRaw);

                auto scaleSeq = CCSequence::create(scaleUp, scaleDown, nullptr);

                auto throwAction = CCSpawn::create(
                    move,
                    scaleSeq,
                    nullptr
                );

                tomato->runAction(CCSequence::create(
                    throwAction,

                    CallFuncExt::create([tomato, tomatoSplatTexture, tomatoSplatScale]() { 
                        if (playSFX) {
                            FMODAudioEngine::get()->playEffect("splat.mp3"_spr);
                        }

                        auto texture = CCTextureCache::sharedTextureCache()->addImage(tomatoSplatTexture.string().c_str(), false);
                        if (texture) {
                            tomato->setTexture(texture);
                            tomato->setTextureRect({0, 0, texture->getContentSize().width, texture->getContentSize().height});
                        }

                        tomato->setScale(static_cast<float>(tomatoSplatScale));
                    }),
                        
                    CCDelayTime::create(3.0f),
                    CCFadeOut::create(1.f),
                    CallFuncExt::create([tomato]() {
                        tomato->removeFromParentAndCleanup(true);
                    }),

                    nullptr
                ));

                return ListenerResult::Propagate;
            }
        } 

        return ListenerResult::Propagate; 

    }, InvokeBindFilter(nullptr, BIND_ID)); 
}