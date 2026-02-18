#include "Camera.h"
#include <algorithm>
#include <cmath> 
#include <numbers>

namespace
{
    constexpr float DegToRad(float deg) { return deg * std::numbers::pi_v<float> / 180.0f; }

    constexpr float CAMERA_DAMPING = 10.0f;
    constexpr float ZOOM_SPEED = 2.0f;
    constexpr float PITCH_LIMIT = DegToRad(89.0f);
    constexpr float ORBIT_SENSITIVITY = 0.005f;
}

Camera::Camera() 
    : m_target{ 0.0f, 0.0f, 0.0f }
    , m_yaw(0.0f)
    , m_pitch(0.0f)
    , m_distance(10.0f)
    , m_targetDistance(10.0f) {}

void Camera::Process(float dt)
{
    float alpha = 1.0f - std::exp(-CAMERA_DAMPING * dt);
    m_distance += (m_targetDistance - m_distance) * alpha;
}

void Camera::Orbit(float deltaYaw, float deltaPitch)
{
    m_yaw += deltaYaw * ORBIT_SENSITIVITY;
    m_pitch += deltaPitch * ORBIT_SENSITIVITY;

    if (m_pitch > PITCH_LIMIT) m_pitch = PITCH_LIMIT;
    if (m_pitch < -PITCH_LIMIT) m_pitch = -PITCH_LIMIT;
}

void Camera::Pan(float deltaX, float deltaY)
{
	float cosPitch = std::cos(m_pitch);
	float sinPitch = std::sin(m_pitch);
	float cosYaw = std::cos(m_yaw);
	float sinYaw = std::sin(m_yaw);

	Vec3 right { cosYaw, 0.0f, sinYaw };
	Vec3 up { -sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };

	m_target = m_target + right * (-deltaX * ORBIT_SENSITIVITY) + up * (deltaY * ORBIT_SENSITIVITY);
}

void Camera::Zoom(float steps)
{
    m_targetDistance -= steps * ZOOM_SPEED;
    m_targetDistance = std::clamp(m_targetDistance, 3.0f, 50.0f);
}

void Camera::SetDistance(float d)
{
    m_distance = std::clamp(d, 3.0f, 50.0f);
    m_targetDistance = m_distance;
}

Vec3 Camera::GetEye() const
{
    float cosPitch = std::cos(m_pitch);
    float sinPitch = std::sin(m_pitch);
    float cosYaw = std::cos(m_yaw);
    float sinYaw = std::sin(m_yaw);

    Vec3 eye;
    eye.x = m_target.x + m_distance * sinYaw * cosPitch;
    eye.y = m_target.y + m_distance * sinPitch;
    eye.z = m_target.z - m_distance * cosYaw * cosPitch;

    return eye;
}

Vec3 Camera::GetUp() const
{
    return { 0.0f, 1.0f, 0.0f };
}

Mat4 Camera::GetViewLH() const
{
    return LookAtLH(GetEye(), m_target, GetUp());
}

Mat4 Camera::GetProjLH(float aspect) const
{
    return PerspectiveFovLH(DegToRad(m_fovYDeg), aspect, m_nearZ, m_farZ);
}
