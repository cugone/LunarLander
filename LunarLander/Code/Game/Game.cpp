#include "Game/Game.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/KerningFont.hpp"

#include "Engine/Input/InputSystem.hpp"

#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Disc2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/MathUtils.hpp"

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

    m_cameraController = OrthographicCameraController();
    m_cameraController.SetPosition(Vector2::Zero);
    m_cameraController.SetZoomLevelRange(Vector2{ g_theRenderer->GetOutput()->GetDimensions().y * 0.10f, g_theRenderer->GetOutput()->GetDimensions().y * 0.50f});
    m_cameraController.SetZoomLevel(g_theRenderer->GetOutput()->GetDimensions().y * 0.10f);

    m_lockCameraRotation = GetSettings().IsCameraRotationLocked();
    m_lockCameraPosition = GetSettings().IsCameraPositionLocked();

    m_landerSheet = g_theRenderer->CreateSpriteSheet("Data/Images/Lander.png", 3, 1);

    m_lander = std::make_unique<Lander>();
    m_lander->SetPosition(Vector2::Zero);

}

void Game::BeginFrame() noexcept {
    m_lander->BeginFrame();
}

void Game::Update(TimeUtils::FPSeconds deltaSeconds) noexcept {
    g_theRenderer->UpdateGameTime(deltaSeconds);

    HandlePlayerInput(deltaSeconds);

    m_ui_camera2D.Update(deltaSeconds);
    m_cameraController.Update(deltaSeconds);

    m_lander->Update(deltaSeconds);
    m_cameraController.SetPosition(Vector2::Zero);
    m_cameraController.SetRotationDegrees(0.0f);
    if(IsCameraRotationLockedToLander()) {
        m_cameraController.SetRotationDegrees(m_lander->GetOrientationDegrees());
    }
    if(IsCameraPositionLocked()) {
        m_cameraController.SetPosition(m_lander->GetPosition());
    }
}
AABB2 Game::CalcOrthoBounds() const noexcept {
    float half_view_height = m_cameraController.GetCamera().GetViewHeight() * 0.5f;
    float half_view_width = half_view_height * m_cameraController.GetAspectRatio();
    auto ortho_mins = Vector2{ -half_view_width, -half_view_height };
    auto ortho_maxs = Vector2{ half_view_width, half_view_height };
    return AABB2{ ortho_mins, ortho_maxs };
}

AABB2 Game::CalcViewBounds(const Vector2& cam_pos) const noexcept {
    auto view_bounds = CalcOrthoBounds();
    view_bounds.Translate(cam_pos);
    return view_bounds;
}

void Game::SetModelViewProjectionBounds() const noexcept {
    const auto ortho_bounds = CalcOrthoBounds();

    g_theRenderer->SetModelMatrix(Matrix4::I);
    g_theRenderer->SetViewMatrix(Matrix4::I);
    const auto leftBottom = Vector2{ ortho_bounds.mins.x, ortho_bounds.maxs.y };
    const auto rightTop = Vector2{ ortho_bounds.maxs.x, ortho_bounds.mins.y };
    m_cameraController.GetCamera().SetupView(leftBottom, rightTop, Vector2(0.0f, 1000.0f));
    g_theRenderer->SetCamera(m_cameraController.GetCamera());

    const Camera2D& base_camera = m_cameraController.GetCamera();
    Camera2D shakyCam = m_cameraController.GetCamera();
    const float shake = shakyCam.GetShake();
    const float shaky_angle = GetSettings().GetMaxShakeAngle() * shake * MathUtils::GetRandomNegOneToOne<float>();
    const float shaky_offsetX = GetSettings().GetMaxShakeOffsetHorizontal() * shake * MathUtils::GetRandomNegOneToOne<float>();
    const float shaky_offsetY = GetSettings().GetMaxShakeOffsetVertical()* shake* MathUtils::GetRandomNegOneToOne<float>();
    shakyCam.orientation_degrees = base_camera.orientation_degrees + shaky_angle;
    shakyCam.position = base_camera.position + Vector2{ shaky_offsetX, shaky_offsetY };

    const float cam_rotation_z = shakyCam.GetOrientation();
    const auto VRz = Matrix4::Create2DRotationDegreesMatrix(-cam_rotation_z);

    const auto& cam_pos = shakyCam.GetPosition();
    const auto Vt = Matrix4::CreateTranslationMatrix(-cam_pos);
    const auto v = Matrix4::MakeRT(Vt, VRz);
    g_theRenderer->SetViewMatrix(v);

}

void Game::Render() const noexcept {
    g_theRenderer->BeginRenderToBackbuffer();


    //World View
    SetModelViewProjectionBounds();

    g_theRenderer->SetMaterial("__2D");
    AABB2 ground = AABB2::Neg_One_to_One;
    ground.ScalePadding(100.0, 50.0f);
    ground.Translate(Vector2{ 0.0f, 100.0f - ground.CalcDimensions().y * 0.5f });

    g_theRenderer->SetModelMatrix(Matrix4::I);
    g_theRenderer->DrawAABB2(ground, Rgba::White, Rgba::LightGray, Vector2::One);

    m_lander->Render();
    if (m_debug_render) {
        m_lander->DebugRender();
    }
    // HUD View

    const auto ui_view_height = static_cast<float>(GetSettings().GetWindowHeight());
    const auto ui_view_width = ui_view_height * m_ui_camera2D.GetAspectRatio();
    const auto ui_view_extents = Vector2{ui_view_width, ui_view_height};
    const auto ui_view_half_extents = ui_view_extents * 0.5f;
    const auto ui_cam_pos = Vector2::Zero;
    g_theRenderer->BeginHUDRender(m_ui_camera2D, ui_cam_pos, ui_view_height);

}

void Game::EndFrame() noexcept {
    m_lander->EndFrame();
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

void Game::HandleKeyboardInput(TimeUtils::FPSeconds deltaSeconds) {
    if(g_theInputSystem->WasKeyJustPressed(KeyCode::Esc)) {
        auto* app = ServiceLocator::get<IAppService>();
        app->SetIsQuitting(true);
        return;
    }
    HandleDebugInput(deltaSeconds);
    if (g_theInputSystem->IsKeyDown(KeyCode::Q)) {
        m_lander->RotateLeft();
    }
    if (g_theInputSystem->IsKeyDown(KeyCode::E)) {
        m_lander->RotateRight();
    }
    if (g_theInputSystem->IsKeyDown(KeyCode::S)) {
        m_lander->BeginThrust();
    }
    if (g_theInputSystem->WasKeyJustReleased(KeyCode::S)) {
        m_lander->EndThrust();
    }
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
