// Fake Mod Menu - a just-for-fun, client-side-only prank mod.
// Press K anywhere in Geometry Dash to open the menu.
//
// Everything here only changes what YOU see on YOUR screen. Nothing is sent
// to RobTop's servers, nobody else can see it, and it does not actually
// rate/feature anything.
//
// Written for Geode v5.x / GD 2.2081. If your SDK version differs, see the
// notes in README.md (the keyboard hook signature is the most likely thing
// to need a tweak).

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/ProfilePage.hpp>

using namespace geode::prelude;

// ---------------------------------------------------------------------------
// Settings helpers (saved between sessions in the mod's save data)
// ---------------------------------------------------------------------------
namespace fake {
    inline bool getBool(char const* key) {
        return Mod::get()->getSavedValue<bool>(key, false);
    }
    inline void setBool(char const* key, bool v) {
        Mod::get()->setSavedValue<bool>(key, v);
    }
    inline int getInt(char const* key, int def = 0) {
        return Mod::get()->getSavedValue<int>(key, def);
    }
    inline void setInt(char const* key, int v) {
        Mod::get()->setSavedValue<int>(key, v);
    }

    // Toggle keys, indexed by menu item tag
    static constexpr char const* TOGGLE_KEYS[] = {
        "rate-on",     // 0: fake rate levels
        "featured",    // 1: fake featured
        "epic",        // 2: fake epic
        "hype",        // 3: fake likes / downloads
        "stats-on",    // 4: fake profile stats
    };
}

// ---------------------------------------------------------------------------
// The popup menu
// ---------------------------------------------------------------------------
class FakeMenu : public Popup {
protected:
    CCLabelBMFont* m_starsLabel = nullptr;
    CCLabelBMFont* m_badgeLabel = nullptr;

    static char const* badgeName(int b) {
        switch (b) {
            case 1: return "Moderator";
            case 2: return "Elder Mod";
            default: return "None";
        }
    }

    void addLabel(char const* text, float x, float y, float scale = 0.5f) {
        auto lbl = CCLabelBMFont::create(text, "bigFont.fnt");
        lbl->setScale(scale);
        lbl->setAnchorPoint({0.f, 0.5f});
        lbl->setPosition({x, y});
        m_mainLayer->addChild(lbl);
    }

    void addToggle(int tag, char const* text, float y) {
        auto toggler = CCMenuItemToggler::createWithStandardSprites(
            this, menu_selector(FakeMenu::onToggle), 0.6f
        );
        toggler->setTag(tag);
        toggler->toggle(fake::getBool(fake::TOGGLE_KEYS[tag]));
        toggler->setPosition({m_size.width - 35.f, y});
        m_menu->addChild(toggler);
        addLabel(text, 20.f, y, 0.45f);
    }

    CCMenuItemSpriteExtra* makeButton(char const* text, float x, float y, SEL_MenuHandler cb, float scale = 0.6f) {
        auto spr = ButtonSprite::create(text);
        spr->setScale(scale);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, cb);
        btn->setPosition({x, y});
        m_menu->addChild(btn);
        return btn;
    }

    CCMenu* m_menu = nullptr;

    bool init() {
        if (!Popup::init(300.f, 270.f)) return false;
        this->setTitle("Fake Mod Menu");

        m_menu = CCMenu::create();
        m_menu->setPosition({0.f, 0.f});
        m_menu->setContentSize(m_size);
        m_mainLayer->addChild(m_menu);

        float top = m_size.height - 55.f;
        float step = 26.f;

        // --- Toggles ---------------------------------------------------
        addToggle(0, "Fake Rate Levels",    top);
        addToggle(1, "Fake Featured",       top - step);
        addToggle(2, "Fake Epic",           top - step * 2);
        addToggle(3, "Fake Likes/Downloads", top - step * 3);
        addToggle(4, "Fake Profile Stats",  top - step * 4);

        // --- Star stepper ---------------------------------------------
        float y = top - step * 5;
        addLabel("Stars:", 20.f, y, 0.45f);
        makeButton("-", m_size.width - 110.f, y, menu_selector(FakeMenu::onStarsDown), 0.5f);
        m_starsLabel = CCLabelBMFont::create("0", "bigFont.fnt");
        m_starsLabel->setScale(0.5f);
        m_starsLabel->setPosition({m_size.width - 75.f, y});
        m_mainLayer->addChild(m_starsLabel);
        makeButton("+", m_size.width - 40.f, y, menu_selector(FakeMenu::onStarsUp), 0.5f);

        // --- Mod badge stepper ----------------------------------------
        y -= step;
        addLabel("Badge:", 20.f, y, 0.45f);
        makeButton("<", m_size.width - 135.f, y, menu_selector(FakeMenu::onBadgeDown), 0.5f);
        m_badgeLabel = CCLabelBMFont::create("None", "bigFont.fnt");
        m_badgeLabel->setScale(0.4f);
        m_badgeLabel->setPosition({m_size.width - 85.f, y});
        m_mainLayer->addChild(m_badgeLabel);
        makeButton(">", m_size.width - 35.f, y, menu_selector(FakeMenu::onBadgeUp), 0.5f);

        // --- Prank popups ---------------------------------------------
        y -= step + 6.f;
        makeButton("Fake Rate Msg", m_size.width * 0.28f, y, menu_selector(FakeMenu::onFakeRateMsg), 0.5f);
        makeButton("Fake Mod Msg", m_size.width * 0.72f, y, menu_selector(FakeMenu::onFakeModMsg), 0.5f);

        y -= step + 4.f;
        makeButton("Reset All", m_size.width * 0.5f, y, menu_selector(FakeMenu::onReset), 0.5f);

        this->refresh();
        return true;
    }

    void refresh() {
        m_starsLabel->setString(fmt::format("{}", fake::getInt("stars", 10)).c_str());
        m_badgeLabel->setString(badgeName(fake::getInt("badge", 0)));
    }

    void onToggle(CCObject* sender) {
        auto t = static_cast<CCMenuItemToggler*>(sender);
        // The callback fires before the toggler flips itself.
        bool newState = !t->isToggled();
        fake::setBool(fake::TOGGLE_KEYS[t->getTag()], newState);
    }

    void onStarsUp(CCObject*) {
        fake::setInt("stars", std::min(10, fake::getInt("stars", 10) + 1));
        refresh();
    }
    void onStarsDown(CCObject*) {
        fake::setInt("stars", std::max(0, fake::getInt("stars", 10) - 1));
        refresh();
    }
    void onBadgeUp(CCObject*) {
        fake::setInt("badge", (fake::getInt("badge", 0) + 1) % 3);
        refresh();
    }
    void onBadgeDown(CCObject*) {
        fake::setInt("badge", (fake::getInt("badge", 0) + 2) % 3);
        refresh();
    }

    void onFakeRateMsg(CCObject*) {
        FLAlertLayer::create(
            "Congratulations!",
            fmt::format(
                "Your level has been <cy>rated</c>!\n"
                "It was awarded <cg>{}</c> stars{}{}.\n"
                "<cr>(This is a joke. Nothing was really rated.)</c>",
                fake::getInt("stars", 10),
                fake::getBool("featured") ? " and <co>featured</c>" : "",
                fake::getBool("epic") ? " with an <cp>epic</c> rating" : ""
            ),
            "Woohoo!"
        )->show();
    }

    void onFakeModMsg(CCObject*) {
        FLAlertLayer::create(
            "Message from RobTop",
            "Congratulations! You've been accepted as a <cy>Geometry Dash Moderator</c>!\n"
            "<cr>(Just kidding. This is a joke mod.)</c>",
            "Yay"
        )->show();
        Notification::create("You are now a (fake) moderator!", NotificationIcon::Success)->show();
    }

    void onReset(CCObject*) {
        for (auto key : fake::TOGGLE_KEYS) fake::setBool(key, false);
        fake::setInt("stars", 10);
        fake::setInt("badge", 0);
        Notification::create("Fake settings reset. Reopen the menu to refresh toggles.", NotificationIcon::Info)->show();
        this->onClose(nullptr);
    }

public:
    static FakeMenu* create() {
        auto ret = new FakeMenu();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

// ---------------------------------------------------------------------------
// Press K to open / close the menu
// ---------------------------------------------------------------------------
class $modify(FakeKeyboard, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double timestamp) {
        if (down && !repeat && key == KEY_K) {
            // Don't hijack K while the player is typing in a text box.
            if (!CCIMEDispatcher::sharedDispatcher()->hasDelegate()) {
                if (auto existing = CCDirector::get()->getRunningScene()->getChildByID("fake-mod-menu"_spr)) {
                    static_cast<FakeMenu*>(existing)->onClose(nullptr);
                } else if (auto menu = FakeMenu::create()) {
                    menu->setID("fake-mod-menu"_spr);
                    menu->show();
                }
                return true;
            }
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, timestamp);
    }
};

// ---------------------------------------------------------------------------
// Fake rate / feature / epic / likes / downloads on the level page
// ---------------------------------------------------------------------------
class $modify(FakeLevelInfo, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (level) {
            if (fake::getBool("rate-on")) {
                level->m_stars = fake::getInt("stars", 10);
            }
            if (fake::getBool("featured")) {
                level->m_featured = 1;
            }
            if (fake::getBool("epic")) {
                level->m_isEpic = 1;
            }
            if (fake::getBool("hype")) {
                level->m_likes = 1234567;
                level->m_downloads = 9876543;
            }
        }
        return LevelInfoLayer::init(level, challenge);
    }
};

// ---------------------------------------------------------------------------
// Fake mod badge + stats, ONLY on your own profile
// ---------------------------------------------------------------------------
class $modify(FakeProfile, ProfilePage) {
    void loadPageFromUserInfo(GJUserScore* score) {
        if (score && score->m_accountID == GJAccountManager::get()->m_accountID) {
            int badge = fake::getInt("badge", 0);
            if (badge > 0) {
                score->m_modBadge = badge;
            }
            if (fake::getBool("stats-on")) {
                score->m_stars = 999999;
                score->m_demons = 1337;
                score->m_diamonds = 99999;
                score->m_secretCoins = 164;
                score->m_userCoins = 9999;
                score->m_creatorPoints = 420;
            }
        }
        ProfilePage::loadPageFromUserInfo(score);
    }
};
