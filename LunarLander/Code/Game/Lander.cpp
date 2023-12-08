#include "Game/Lander.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Input/InputSystem.hpp"

#include "Engine/Physics/Collider.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include "Game/Game.hpp"

Lander::Lander() noexcept {
    {
        AnimatedSpriteDesc desc{};
        desc.material = g_theRenderer->GetMaterial("lander");
        desc.spriteSheet = GetGameAs<Game>()->GetLanderSheet();
        desc.frameLength = 3;
        desc.playbackMode = AnimatedSprite::SpriteAnimMode::Looping;
        desc.durationSeconds = TimeUtils::FPFrames{3};
        m_sprite = g_theRenderer->CreateAnimatedSprite(desc);
    }

    {
        RigidBodyDesc desc{};
        desc.physicsDesc = PhysicsDesc{};
        desc.physicsDesc.angularDamping = 1.0f;
        desc.collider = new ColliderOBB(Vector2::Zero, Vector2::One * 5.0f);
        m_body = RigidBody{ desc };
    }
}

void Lander::BeginFrame() noexcept {
    /* DO NOTHING */
}

void Lander::Update(TimeUtils::FPSeconds deltaSeconds) noexcept {
    if(m_isThrusting) {
        if(HasFuel()) {
            m_sprite->Resume();
        }
    }

    m_orientation += m_deltaOrientation * deltaSeconds.count();
    m_body.Update(deltaSeconds);
    m_sprite->Update(deltaSeconds);

    const auto uvs = m_sprite->GetCurrentTexCoords();

    auto& builder = m_builder;
    builder.Begin(PrimitiveType::Triangles);
    builder.SetColor(Rgba::White);

    builder.SetUV(Vector2{ uvs.mins.x, uvs.maxs.y });
    builder.AddVertex(Vector2{ -0.5f, +0.5f });

    builder.SetUV(Vector2{ uvs.mins.x, uvs.mins.y });
    builder.AddVertex(Vector2{ -0.5f, -0.5f });

    builder.SetUV(Vector2{ uvs.maxs.x, uvs.mins.y });
    builder.AddVertex(Vector2{ +0.5f, -0.5f });

    builder.SetUV(Vector2{ uvs.maxs.x, uvs.maxs.y });
    builder.AddVertex(Vector2{ +0.5f, +0.5f });

    builder.AddIndicies(Mesh::Builder::Primitive::Quad);
    builder.End(m_sprite->GetMaterial());

    {
        if (auto* game = GetGameAs<Game>(); game != nullptr) {
            if (game->Debug_IsPositionLockedToMouse()) {
                const auto mouse_pos = g_theInputSystem->GetCursorWindowPosition();
                m_position = Vector2{ g_theRenderer->ConvertScreenToWorldCoords(mouse_pos) };
            }
        }
        const auto S = Matrix4::CreateScaleMatrix(Vector2{m_sprite->GetFrameDimensions()});
        const auto R = Matrix4::Create2DRotationMatrix(m_orientation);
        const auto T = Matrix4::CreateTranslationMatrix(m_position);
        m_transform = Matrix4::MakeSRT(S, R, T);
    }

}

void Lander::Render() const noexcept {
    g_theRenderer->SetModelMatrix(m_transform);
    Mesh::Render(m_builder);
}

void Lander::DebugRender() const noexcept {
    auto landerCollision = OBB2{ GetTransform().GetTranslation().GetXY(), GetTransform().GetScale().GetXY(), GetTransform().GetRotation2D() };
    g_theRenderer->SetMaterial("__2D");
    g_theRenderer->SetModelMatrix(Matrix4::I);
    g_theRenderer->DrawOBB2(landerCollision, Rgba::Green);
}

void Lander::EndFrame() noexcept {
    m_deltaOrientation = 0.0f;
    if (!(m_isThrusting || HasFuel())) {
        m_sprite->Reset();
        m_sprite->Pause();
    }
}

void Lander::RotateLeft() noexcept {
    m_deltaOrientation -= MathUtils::ConvertDegreesToRadians(m_rotationSpeedDegrees);
    if(MathUtils::IsEquivalentToZero(m_deltaOrientation)) {
        m_deltaOrientation = 0.0f;
    }
}

void Lander::RotateRight() noexcept {
    m_deltaOrientation += MathUtils::ConvertDegreesToRadians(m_rotationSpeedDegrees);
    if (MathUtils::IsEquivalentToZero(m_deltaOrientation)) {
        m_deltaOrientation = 0.0f;
    }
}

void Lander::BeginThrust() noexcept {
    m_isThrusting = true;
}

void Lander::EndThrust() noexcept {
    m_isThrusting = false;
}

const Vector2 Lander::GetPosition() const noexcept {
    return m_position;
}

void Lander::SetPosition(const Vector2& newPosition) noexcept {
    m_position = newPosition;
}

const float Lander::GetOrientationDegrees() const noexcept {
    return MathUtils::ConvertRadiansToDegrees(m_orientation);
}

const float Lander::GetOrientationRadians() const noexcept {
    return m_orientation;
}

const Matrix4& Lander::GetTransform() const noexcept {
    return m_transform;
}

bool Lander::HasFuel() const noexcept {
    return m_fuelPounds > 0.0f;
}

