#include "Game/Lander.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Input/InputSystem.hpp"

#include "Engine/Physics/Collider.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include "Game/Game.hpp"

#include "Engine/Core/DataUtils.hpp"

#include <string>

Lander::Lander() noexcept {
    {
        tinyxml2::XMLDocument doc;
        const std::string str =
R"(
<animation name="thrust">
    <animationset startindex="1" framelength="2" duration="0.33" loop="true" />
</animation>
)";
        doc.Parse(str.c_str(), str.size());
        auto xml_first = doc.RootElement();
        m_sprite = g_theRenderer->CreateAnimatedSprite(*xml_first);
    }
    {
        tinyxml2::XMLDocument doc;
        const std::string str =
            R"(
<animation name="nothrust">
    <animationset startindex="0" framelength="1" duration="0" />
</animation>
)";
        doc.Parse(str.c_str(), str.size());
        auto xml_first = doc.RootElement();
        m_noThrustSprite = g_theRenderer->CreateAnimatedSprite(*xml_first);
    }
    {
        AnimatedSpriteDesc desc{};
        desc.material = g_theRenderer->GetMaterial("lander");
        desc.spriteSheet = GetGameAs<Game>()->GetLanderSheet();
        desc.frameLength = 2;
        desc.startSpriteIndex = 1;
        desc.playbackMode = AnimatedSprite::SpriteAnimMode::Looping;
        desc.durationSeconds = TimeUtils::FPSeconds{0.25f};
        m_sprite = g_theRenderer->CreateAnimatedSprite(desc);
    }
    {
        AnimatedSpriteDesc desc{};
        desc.material = g_theRenderer->GetMaterial("lander");
        desc.spriteSheet = GetGameAs<Game>()->GetLanderSheet();
        desc.frameLength = 1;
        desc.startSpriteIndex = 0;
        desc.playbackMode = AnimatedSprite::SpriteAnimMode::Play_To_End;
        desc.durationSeconds = TimeUtils::FPFrames{1};
        m_noThrustSprite = g_theRenderer->CreateAnimatedSprite(desc);
    }
    m_currentSprite = m_noThrustSprite.get();

    {
        RigidBodyDesc desc{};
        desc.physicsDesc = PhysicsDesc{};
        desc.physicsDesc.angularDamping = 1.0f;
        desc.physicsDesc.enableGravity = true;
        desc.physicsDesc.enablePhysics = true;
        desc.collider = new ColliderOBB(Vector2::Zero, Vector2::One * 23.0f);
        m_body = RigidBody{ desc };
    }


}

void Lander::BeginFrame() noexcept {
    /* DO NOTHING */
}

void Lander::Update(TimeUtils::FPSeconds deltaSeconds) noexcept {
    if(m_isThrusting && HasFuel()) {
        //m_currentSprite->Resume();
        m_body.ApplyImpulse(-Vector2::Y_Axis, m_thrustForceKiloNewtons * 1000.0f);
    } else {
        m_currentSprite = m_noThrustSprite.get();
    }

    m_orientation += m_deltaOrientation * deltaSeconds.count();
    m_body.Update(deltaSeconds);
    m_currentSprite->Update(deltaSeconds);

    const auto uvs = m_currentSprite->GetCurrentTexCoords();

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
    builder.End(m_currentSprite->GetMaterial());

    {
        if (auto* game = GetGameAs<Game>(); game != nullptr) {
            if (game->Debug_IsPositionLockedToMouse()) {
                const auto mouse_pos = g_theInputSystem->GetCursorWindowPosition();
                m_body.SetPosition(Vector2{ g_theRenderer->ConvertScreenToWorldCoords(mouse_pos) });
            }
        }
        const auto S = Matrix4::CreateScaleMatrix(Vector2{ m_currentSprite->GetFrameDimensions()});
        const auto R = Matrix4::Create2DRotationMatrix(MathUtils::ConvertDegreesToRadians(m_body.GetOrientationDegrees()));
        const auto T = Matrix4::CreateTranslationMatrix(m_body.GetPosition());
        m_transform = Matrix4::MakeSRT(S, R, T);
    }

}

void Lander::Render() const noexcept {
    g_theRenderer->SetModelMatrix(m_transform);
    Mesh::Render(m_builder);
}

void Lander::DebugRender() const noexcept {
    //auto landerCollision = OBB2{ GetTransform().GetTranslation().GetXY(), GetTransform().GetScale().GetXY(), GetTransform().GetRotation2D() };
    g_theRenderer->SetMaterial("__2D");
    g_theRenderer->SetModelMatrix(Matrix4::I);
    m_body.DebugRender();
    //g_theRenderer->DrawOBB2(landerCollision, Rgba::Green);
}

void Lander::EndFrame() noexcept {
    m_deltaOrientation = 0.0f;
    if (!m_isThrusting || !HasFuel()) {
        EndThrust();
    }
}

void Lander::RotateLeft() noexcept {
    //m_deltaOrientation -= MathUtils::ConvertDegreesToRadians(m_rotationSpeedDegrees);
    //if(MathUtils::IsEquivalentToZero(m_deltaOrientation)) {
    //    m_deltaOrientation = 0.0f;
    //}
    m_body.ApplyTorque(-Vector2::X_Axis, m_thrustForceKiloNewtons * 1000.0f, TimeUtils::Frames{ 1 });
}

void Lander::RotateRight() noexcept {
    //m_deltaOrientation += MathUtils::ConvertDegreesToRadians(m_rotationSpeedDegrees);
    //if (MathUtils::IsEquivalentToZero(m_deltaOrientation)) {
    //    m_deltaOrientation = 0.0f;
    //}
    m_body.ApplyTorque(Vector2::X_Axis, m_thrustForceKiloNewtons * 1000.0f, TimeUtils::Frames{ 1 });
}

void Lander::TranslateLeft() noexcept {
    m_body.ApplyImpulse(-Vector2::X_Axis, m_thrustForceKiloNewtons * 1000.0f);
}

void Lander::TranslateRight() noexcept {
    m_body.ApplyImpulse(Vector2::X_Axis, m_thrustForceKiloNewtons * 1000.0f);
}

void Lander::BeginThrust() noexcept {
    m_isThrusting = true;
    //if (!m_isThrusting) {
    //    m_isThrusting = true;
    //    m_currentSprite = m_sprite.get();
    //}
}

void Lander::EndThrust() noexcept {
    if (m_isThrusting) {
        m_isThrusting = false;
        m_currentSprite = m_noThrustSprite.get();
    }
}

const Vector2 Lander::GetPosition() const noexcept {
    return m_body.GetPosition();
}

void Lander::SetPosition(const Vector2& newPosition) noexcept {
    m_body.SetPosition(newPosition, true);
}

const float Lander::GetOrientationDegrees() const noexcept {
    return m_body.GetOrientationDegrees();
}

const float Lander::GetOrientationRadians() const noexcept {
    return MathUtils::ConvertDegreesToRadians(GetOrientationDegrees());
}

const Matrix4& Lander::GetTransform() const noexcept {
    return m_body.transform;
}

bool Lander::HasFuel() const noexcept {
    return m_fuelPounds > 0.0f;
}

