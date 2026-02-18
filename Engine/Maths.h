#pragma once
#include <cmath>

struct Vec3
{
    float x, y, z;
};

inline Vec3 operator-(const Vec3& a, const Vec3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
inline Vec3 operator+(const Vec3& a, const Vec3& b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
inline Vec3 operator*(const Vec3& v, float s) { return { v.x * s, v.y * s, v.z * s }; }

inline float Dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

inline Vec3 Cross(const Vec3& a, const Vec3& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

inline float Length(const Vec3& v) { return std::sqrt(Dot(v, v)); }

inline Vec3 Normalize(const Vec3& v)
{
    float len = Length(v);
    if (len <= 1e-8f) return { 0,0,0 };
    return v * (1.0f / len);
}


struct Mat4
{
    float m[16]; // row-major: m[row*4 + col]

    static Mat4 Identity()
    {
        Mat4 r{};
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
        return r;
    }

    static void Transpose(const Mat4& in, float out[16])
    {
        out[0]  = in.m[0];  out[1]  = in.m[4];  out[2]  = in.m[8];  out[3]  = in.m[12];
        out[4]  = in.m[1];  out[5]  = in.m[5];  out[6]  = in.m[9];  out[7]  = in.m[13];
        out[8]  = in.m[2];  out[9]  = in.m[6];  out[10] = in.m[10]; out[11] = in.m[14];
        out[12] = in.m[3];  out[13] = in.m[7];  out[14] = in.m[11]; out[15] = in.m[15];
    }

	static Mat4 Translation(float x, float y, float z)
	{
		Mat4 r = Identity();
		r.m[12] = x;
		r.m[13] = y;
		r.m[14] = z;
		return r;
	}
};

inline Mat4 Mul(const Mat4& a, const Mat4& b)
{
    Mat4 r{};
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            r.m[row * 4 + col] =
                a.m[row * 4 + 0] * b.m[0 * 4 + col] +
                a.m[row * 4 + 1] * b.m[1 * 4 + col] +
                a.m[row * 4 + 2] * b.m[2 * 4 + col] +
                a.m[row * 4 + 3] * b.m[3 * 4 + col];
        }
    }
    return r;
}

inline Mat4 LookAtLH(const Vec3& eye, const Vec3& at, const Vec3& up)
{
    Vec3 zaxis = Normalize(at - eye);           // forward
    Vec3 xaxis = Normalize(Cross(up, zaxis));   // right
    Vec3 yaxis = Cross(zaxis, xaxis);           // up

    Mat4 r = Mat4::Identity();

    r.m[0] = xaxis.x; r.m[1] = yaxis.x; r.m[2] = zaxis.x; r.m[3] = 0.0f;
    r.m[4] = xaxis.y; r.m[5] = yaxis.y; r.m[6] = zaxis.y; r.m[7] = 0.0f;
    r.m[8] = xaxis.z; r.m[9] = yaxis.z; r.m[10] = zaxis.z; r.m[11] = 0.0f;

    r.m[12] = -Dot(xaxis, eye);
    r.m[13] = -Dot(yaxis, eye);
    r.m[14] = -Dot(zaxis, eye);
    r.m[15] = 1.0f;

    return r;
}

inline Mat4 PerspectiveFovLH(float fovYRadians, float aspect, float zn, float zf)
{
    float yScale = 1.0f / std::tan(fovYRadians * 0.5f);
    float xScale = yScale / aspect;

    Mat4 r{};
    r.m[0] = xScale;
    r.m[5] = yScale;
    r.m[10] = zf / (zf - zn);
    r.m[11] = 1.0f;
    r.m[14] = (-zn * zf) / (zf - zn);
    return r;
}
