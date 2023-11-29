#pragma once

#include "Engine/Core/TimeUtils.hpp"
#include "Engine/Renderer/AnimatedSprite.hpp"
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

    void Update(TimeUtils::FPSeconds deltaSeconds) noexcept;
    void Render() const noexcept;

    const Vector2 GetPosition() const noexcept;
    const float GetOrientationDegrees() const noexcept;
    const float GetOrientationRadians() const noexcept;

protected:
private:
    static inline std::unique_ptr<AnimatedSprite> m_sprite{};
    Mesh::Builder m_builder{};
    Matrix4 m_transform{};
    float m_orientation{};
    Vector2 m_position{};
};
