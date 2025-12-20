#include <Geode/Geode.hpp>
#include <geode.custom-keybinds/include/Keybinds.hpp>
#include <prevter.imageplus/include/api.hpp>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;
using namespace keybinds;
auto playSFX = true;
$on_mod(Loaded) {
	auto mod = Mod::get();

    // for whatever reason i have to do it this way cuz just getting it in the execute block doesnt work :(

    playSFX = mod->getSettingValue<bool>("playsound");
	listenForSettingChangesV3<bool>("playsound", [](bool value) {
		playSFX = value;
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

            auto tomato = imgp::AnimatedSprite::from(tomatoThrow);
            auto mousePos = geode::cocos::getMousePos();

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
        }
        return ListenerResult::Propagate;
    }, InvokeBindFilter(nullptr, BIND_ID)); 
}