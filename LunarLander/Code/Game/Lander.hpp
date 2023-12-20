#pragma once

#include "Engine/Core/TimeUtils.hpp"
#include "Engine/Renderer/AnimatedSprite.hpp"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Physics/RigidBody.hpp"
#include "Engine/Renderer/Mesh.hpp"

#include <memory>

class Lander {
public:
    Lander() noexcept;
    Lander(const Lander& other) = default;
    Lander(Lander&& other) = default;
    Lander& operator=(const Lander& other) = default;
    Lander& operator=(Lander&& other) = default;
    ~Lander() = default;

    void BeginFrame() noexcept;
    void Update(TimeUtils::FPSeconds deltaSeconds) noexcept;
    void DebugRender() const noexcept;
    void Render() const noexcept;
    void EndFrame() noexcept;

    void RotateLeft() noexcept;
    void RotateRight() noexcept;

    void TranslateLeft() noexcept;
    void TranslateRight() noexcept;

    void BeginThrust() noexcept;
    void EndThrust() noexcept;

    const Vector2 GetPosition() const noexcept;
    void SetPosition(const Vector2& newPosition) noexcept;

    const float GetOrientationDegrees() const noexcept;
    const float GetOrientationRadians() const noexcept;

    const Matrix4& GetTransform() const noexcept;

    bool HasFuel() const noexcept;
protected:
private:
    static inline std::unique_ptr<AnimatedSprite> m_sprite{};
    static inline std::unique_ptr<AnimatedSprite> m_noThrustSprite{};
    AnimatedSprite* m_currentSprite{ nullptr };
    Mesh::Builder m_builder{};
    Matrix4 m_transform{};
    float m_orientation{};
    Vector2 m_position{};
    Vector2 m_velocity{};
    RigidBody m_body{RigidBodyDesc{}};
    float m_rotationSpeedDegrees{1.0f};
    float m_deltaOrientation{0.0f};
    float m_fuelPounds{1.0f};
    const float m_thrustForceKiloNewtons{10.0f};
    bool m_isThrusting{ false };
};
