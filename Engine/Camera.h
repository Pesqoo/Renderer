#pragma once
#include "Maths.h"

class Camera
{
public:
    Camera();

    void Process(float dt);

    void Orbit(float deltaYaw, float deltaPitch);
    void Zoom(float steps);

    void SetTarget(const Vec3& target) { m_target = target; }
    const Vec3& GetTarget() const { return m_target; }

    void SetDistance(float d);
    float GetDistance() const { return m_distance; }

    Vec3 GetEye() const;
    Vec3 GetUp() const;

    Mat4 GetViewLH() const;
    Mat4 GetProjLH(float aspect) const;
    Mat4 GetViewProjLH(float aspect) const { return Mul(GetViewLH(), GetProjLH(aspect)); }

private:
    Vec3 m_target;

    float m_yaw = 0.0f;
    float m_pitch = 0.0f;

    float m_distance = 10.0f;
    float m_targetDistance = 10.0f;

    float m_fovYDeg = 60.0f;
    float m_nearZ = 0.1f;
    float m_farZ = 100.0f;
};
