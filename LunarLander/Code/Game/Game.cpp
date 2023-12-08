#include "Game/Game.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/KerningFont.hpp"

#include "Engine/Input/InputSystem.hpp"

#include "Engine/Math/Disc2.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Material.hpp"

#include "Engine/Services/ServiceLocator.hpp"
#include "Engine/Services/IAppService.hpp"

#include "Engine/UI/UISystem.hpp"

#include "Game/GameCommon.hpp"
#include "Game/GameConfig.hpp"


void GameOptions::SaveToConfig(Config& config) noexcept {
    GameSettings::SaveToConfig(config);
    config.SetValue("lockCameraRotation", m_lockCameraRotation);
    config.SetValue("lockCameraPosition", m_lockCameraPosition);
}

void GameOptions::SetToDefault() noexcept {
    GameSettings::SetToDefault();
    m_lockCameraRotation = m_defaultLockCameraRotation;
    m_lockPositionToMouse = m_defaultLockPositionToMouse;
}

bool GameOptions::IsCameraRotationLocked() const noexcept {
    return m_lockCameraRotation;
}

bool GameOptions::IsCameraPositionLocked() const noexcept {
    return m_lockCameraPosition;
}

const float GameOptions::GetMaxShakeAngle() const noexcept {
    return m_maxShakeAngle;
}

const float GameOptions::GetMaxShakeOffsetHorizontal() const noexcept {
    return m_maxShakeOffsetHorizontal;
}

const float GameOptions::GetMaxShakeOffsetVertical() const noexcept {
    return m_maxShakeOffsetVertical;
}

void Game::Initialize() noexcept {
    if(!g_theConfig->LoadFromFile(FileUtils::GetKnownFolderPath(FileUtils::KnownPathID::GameConfig) / "options.config")) {
        g_theFileLogger->LogWarnLine("Config not loaded. Reverting to default settings.");
        m_settings.SetToDefault();
    }

    g_theRenderer->RegisterMaterialsFromFolder(FileUtils::GetKnownFolderPath(FileUtils::KnownPathID::GameMaterials));
    g_theRenderer->RegisterFontsFromFolder(FileUtils::GetKnownFolderPath(FileUtils::KnownPathID::GameFonts));

    _cameraController = OrthographicCameraController();
    _cameraController.SetPosition(Vector2::Zero);
}

void Game::BeginFrame() noexcept {
    /* DO NOTHING */
}

void Game::Update(TimeUtils::FPSeconds deltaSeconds) noexcept {
    g_theRenderer->UpdateGameTime(deltaSeconds);

    HandlePlayerInput(deltaSeconds);

    _ui_camera2D.Update(deltaSeconds);
    _cameraController.Update(deltaSeconds);
}

void Game::Render() const noexcept {
    g_theRenderer->BeginRenderToBackbuffer();


    //World View
    g_theRenderer->SetMaterial(g_theRenderer->GetMaterial("__2D"));
    {
        const auto S = Matrix4::CreateScaleMatrix(Vector2::One);
        const auto R = Matrix4::I;
        const auto T = Matrix4::I;
        const auto M = Matrix4::MakeSRT(S, R, T);
        g_theRenderer->DrawQuad2D(M, Rgba::ForestGreen);
    }

    // HUD View
    {
        const auto ui_view_height = static_cast<float>(GetSettings().GetWindowHeight());
        const auto ui_view_width = ui_view_height * _ui_camera2D.GetAspectRatio();
        const auto ui_view_extents = Vector2{ui_view_width, ui_view_height};
        const auto ui_view_half_extents = ui_view_extents * 0.5f;
        const auto ui_cam_pos = Vector2::Zero;
        g_theRenderer->BeginHUDRender(_ui_camera2D, ui_cam_pos, ui_view_height);

        {
            const auto S = Matrix4::CreateScaleMatrix(Vector2::One * (1.0f + MathUtils::SineWaveDegrees(g_theRenderer->GetGameTime().count())));
            static float r = 0.0f;
            const std::string text = "Abrams 2022 Template";
            const auto* font = g_theRenderer->GetFont("System32");
            const auto T = Matrix4::I;
            const auto nT = Matrix4::CreateTranslationMatrix(-Vector2{font->CalculateTextWidth(text), font->CalculateTextHeight(text)} * 0.5f);
            const auto R = Matrix4::Create2DRotationDegreesMatrix(r);
            static const float w = 90.0f;
            r += g_theRenderer->GetGameFrameTime().count() * w;
            const auto M = Matrix4::MakeRT(nT, Matrix4::MakeSRT(S, R, T));
            g_theRenderer->DrawTextLine(M, font, text);
        }
    }
}

void Game::EndFrame() noexcept {
    /* DO NOTHING */
}

const GameOptions& Game::GetSettings() const noexcept {
    return m_settings;
}

GameOptions& Game::GetSettings() noexcept {
    return m_settings;
}

std::weak_ptr<SpriteSheet> Game::GetLanderSheet() const noexcept {
    return m_landerSheet;
}

void Game::LockCameraRotationToLander() noexcept {
    m_lockCameraRotation = true;
}

void Game::UnlockCameraRotationToLander() noexcept {
    m_lockCameraRotation = false;
}

bool Game::IsCameraRotationLockedToLander() const noexcept {
    return m_lockCameraRotation;
}

void Game::LockCameraPositionToLander() noexcept {
    m_lockCameraPosition = true;
}

void Game::UnlockCameraPositionToLander() noexcept {
    m_lockCameraPosition = false;
}

bool Game::IsCameraPositionLocked() const noexcept {
    return m_lockCameraPosition;
}

bool Game::Debug_IsPositionLockedToMouse() const noexcept {
    return m_lockPositionToMouse;
}

void Game::Debug_LockPositionToMouse() noexcept {
    m_lockPositionToMouse = true;
}

void Game::Debug_UnlockPositionToMouse() noexcept {
    m_lockPositionToMouse = false;
}

void Game::HandlePlayerInput(TimeUtils::FPSeconds deltaSeconds) {
    HandleKeyboardInput(deltaSeconds);
    HandleControllerInput(deltaSeconds);
    HandleMouseInput(deltaSeconds);
}

void Game::HandleKeyboardInput(TimeUtils::FPSeconds /*deltaSeconds*/) {
    if(g_theInputSystem->WasKeyJustPressed(KeyCode::Esc)) {
        auto* app = ServiceLocator::get<IAppService>();
        app->SetIsQuitting(true);
        return;
    }
    HandleDebugInput(deltaSeconds);
}

void Game::HandleControllerInput(TimeUtils::FPSeconds /*deltaSeconds*/) {

}

void Game::HandleMouseInput(TimeUtils::FPSeconds /*deltaSeconds*/) {

}

void Game::HandleDebugInput(TimeUtils::FPSeconds deltaSeconds) {
    HandleDebugKeyboardInput(deltaSeconds);
    HandleDebugMouseInput(deltaSeconds);
}

void Game::HandleDebugKeyboardInput(TimeUtils::FPSeconds /*deltaSeconds*/) {
    if(g_theUISystem->WantsInputKeyboardCapture()) {
        return;
    }
    if (g_theInputSystem->WasKeyJustPressed(KeyCode::F1)) {
        m_debug_render = !m_debug_render;
    }
    if(g_theInputSystem->WasKeyJustPressed(KeyCode::F2)) {
        if(Debug_IsPositionLockedToMouse()) {
            Debug_UnlockPositionToMouse();
        } else {
            Debug_LockPositionToMouse();
        }
    }
    if (g_theInputSystem->WasKeyJustPressed(KeyCode::F3)) {
        if(IsCameraRotationLockedToLander()) {
            UnlockCameraRotationToLander();
        } else {
            LockCameraRotationToLander();
        }
    }
    if(g_theInputSystem->WasKeyJustPressed(KeyCode::F4)) {
        g_theUISystem->ToggleImguiDemoWindow();
    }
}

void Game::HandleDebugMouseInput(TimeUtils::FPSeconds /*deltaSeconds*/) {
    if(g_theUISystem->WantsInputMouseCapture()) {
        return;
    }
}
