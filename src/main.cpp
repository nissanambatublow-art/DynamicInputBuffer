#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

// Global state tracking for buffered inputs
struct InputBufferState {
    bool isBuffered = false;
    int remainingTicks = 0;
    PlayerButton currentButton = PlayerButton::Jump;
} g_bufferState;

class $modify(BufferedPlayerObject, PlayerObject) {
    bool pushButton(PlayerButton btn) {
        bool modEnabled = Mod::get()->getSettingValue<bool>("enabled");
        int maxTicks = Mod::get()->getSettingValue<int64_t>("buffer-ticks");

        if (!modEnabled) {
            return PlayerObject::pushButton(btn);
        }

        // Check if the player can perform a normal jump immediately
        // (e.g. touching ground or currently inside an interactive trigger/orb)
        bool canPerformAction = this->m_isOnGround || this->m_touchingRings->count() > 0;

        if (canPerformAction) {
            // Register input normally
            g_bufferState.isBuffered = false;
            return PlayerObject::pushButton(btn);
        } else {
            // Input clicked too early: queue the buffer for specified ticks
            g_bufferState.isBuffered = true;
            g_bufferState.remainingTicks = static_cast<int>(maxTicks);
            g_bufferState.currentButton = btn;

            log::debug("Input buffered for {} ticks", maxTicks);
            return false;
        }
    }
};

class $modify(BufferedGameLayer, GJBaseGameLayer) {
    void update(float dt) {
        GJBaseGameLayer::update(dt);

        bool modEnabled = Mod::get()->getSettingValue<bool>("enabled");
        if (!modEnabled || !g_bufferState.isBuffered) return;

        if (auto player = this->m_player1) {
            // Check if player has reached a valid interactable condition during the buffer frame
            bool canPerformAction = player->m_isOnGround || player->m_touchingRings->count() > 0;

            if (canPerformAction) {
                // Execute the buffered jump input automatically
                player->pushButton(g_bufferState.currentButton);
                g_bufferState.isBuffered = false;
                log::debug("Buffered input triggered successfully!");
            } else {
                // Decrement tick timer each frame tick
                g_bufferState.remainingTicks--;
                if (g_bufferState.remainingTicks <= 0) {
                    g_bufferState.isBuffered = false; // Buffer expired
                }
            }
        }
    }
};
