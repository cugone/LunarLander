#pragma once

#include "Engine/Game/GameBase.hpp"

#include "Engine/Core/TimeUtils.hpp"
#include "Engine/Core/OrthographicCameraController.hpp"

#include "Engine/Renderer/Camera2D.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/Lander.hpp"

#include <memory>

class GameOptions : public GameSettings {
public:
    GameOptions() noexcept = default;
    GameOptions(const GameOptions& other) noexcept = default;
    GameOptions(GameOptions&& other) noexcept = default;
    virtual ~GameOptions() noexcept = default;
    GameOptions& operator=(const GameOptions& rhs) noexcept = default;
    GameOptions& operator=(GameOptions&& rhs) noexcept = default;

    virtual void SaveToConfig(Config& config) noexcept override;
    virtual void SetToDefault() noexcept override;

    bool IsCameraRotationLocked() const noexcept;
    bool IsCameraPositionLocked() const noexcept;

    const float GetMaxShakeAngle() const noexcept;
    const float GetMaxShakeOffsetHorizontal() const noexcept;
    const float GetMaxShakeOffsetVertical() const noexcept;

protected:
private:
    bool m_lockCameraRotation{ false };
    bool m_defaultLockCameraRotation{ false };
    bool m_lockCameraPosition{ false };
    bool m_defaultLockCameraPosition{ false };
    bool m_lockPositionToMouse{ false };
    bool m_defaultLockPositionToMouse{ false };
    float m_maxShakeAngle{ 2.5f };
    float m_maxShakeOffsetHorizontal{25.0f};
    float m_maxShakeOffsetVertical{25.0f};
};

class Game : public GameBase {
public:
    Game() = default;
    Game(const Game& other) = default;
    Game(Game&& other) = default;
    Game& operator=(const Game& other) = default;
    Game& operator=(Game&& other) = default;
    ~Game() = default;

    void Initialize() noexcept override;
    void BeginFrame() noexcept override;
    void Update(TimeUtils::FPSeconds deltaSeconds) noexcept override;
    void Render() const noexcept override;
    void EndFrame() noexcept override;

    const GameOptions& GetSettings() const noexcept override;
    GameOptions& GetSettings() noexcept override;

    [[nodiscard]] std::weak_ptr<SpriteSheet> GetLanderSheet() const noexcept;

    bool IsCameraRotationLockedToLander() const noexcept;
    void LockCameraRotationToLander() noexcept;
    void UnlockCameraRotationToLander() noexcept;

    void LockCameraPositionToLander() noexcept;
    void UnlockCameraPositionToLander() noexcept;
    bool IsCameraPositionLocked() const noexcept;

    bool Debug_IsPositionLockedToMouse() const noexcept;
    void Debug_LockPositionToMouse() noexcept;
    void Debug_UnlockPositionToMouse() noexcept;

protected:
private:

    void HandleDebugInput(TimeUtils::FPSeconds deltaSeconds);
    void HandleDebugKeyboardInput(TimeUtils::FPSeconds deltaSeconds);
    void HandleDebugMouseInput(TimeUtils::FPSeconds deltaSeconds);

    void HandlePlayerInput(TimeUtils::FPSeconds deltaSeconds);
    void HandleKeyboardInput(TimeUtils::FPSeconds deltaSeconds);
    void HandleControllerInput(TimeUtils::FPSeconds deltaSeconds);
    void HandleMouseInput(TimeUtils::FPSeconds deltaSeconds);

    mutable Camera2D m_ui_camera2D{};
    mutable OrthographicCameraController m_cameraController{};
    GameOptions m_settings{};
    std::shared_ptr<SpriteSheet> m_landerSheet{};
    std::unique_ptr<Lander> m_lander{};
    bool m_debug_render{ false };
    bool m_lockPositionToMouse{ false };
    bool m_lockCameraRotation{ false };
    bool m_lockCameraPosition{ false };
};

