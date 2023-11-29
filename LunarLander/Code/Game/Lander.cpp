#include "Game/Lander.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Input/InputSystem.hpp"

#include "Engine/Renderer/Renderer.hpp"

#include "Game/Game.hpp"

Lander::Lander() noexcept {
    AnimatedSpriteDesc desc{};
    desc.material = g_theRenderer->GetMaterial("lander");
    desc.spriteSheet = GetGameAs<Game>()->GetLanderSheet();
    desc.frameLength = 3;
    desc.playbackMode = AnimatedSprite::SpriteAnimMode::Looping;
    m_sprite = g_theRenderer->CreateAnimatedSprite(desc);
}

void Lander::Update(TimeUtils::FPSeconds deltaSeconds) noexcept {

    m_sprite->Update(deltaSeconds);

    const auto uvs = m_sprite->GetCurrentTexCoords();

    auto& builder = m_builder;
    builder.Begin(PrimitiveType::Triangles);
    builder.SetColor(Rgba::White);

    builder.SetUV(Vector2{ uvs.maxs.x, uvs.maxs.y });
    builder.AddVertex(Vector2{ +0.5f, +0.5f });

    builder.SetUV(Vector2{ uvs.mins.x, uvs.maxs.y });
    builder.AddVertex(Vector2{ -0.5f, +0.5f });

    builder.SetUV(Vector2{ uvs.mins.x, uvs.mins.y });
    builder.AddVertex(Vector2{ -0.5f, -0.5f });

    builder.SetUV(Vector2{ uvs.maxs.x, uvs.mins.y });
    builder.AddVertex(Vector2{ +0.5f, -0.5f });

    builder.AddIndicies(Mesh::Builder::Primitive::Quad);
    builder.End(m_sprite->GetMaterial());

    {
        const auto mouse_pos = g_theInputSystem->GetCursorWindowPosition();
        m_position = Vector2{g_theRenderer->ConvertScreenToWorldCoords(mouse_pos)};
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

const Vector2 Lander::GetPosition() const noexcept {
    return m_position;
}

const float Lander::GetOrientationDegrees() const noexcept {
    return MathUtils::ConvertRadiansToDegrees(m_orientation);
}

const float Lander::GetOrientationRadians() const noexcept {
    return m_orientation;
}

