#include "PreCompiledHeader.h"
#include "Quaternion.h"

namespace EngineCore
{
    // 单位四元数，满足 w = 1；
    // xyz = 0， 且模长为1
    Quaternion Quaternion::Identity = Quaternion(0,0,0,1);

    Quaternion::Quaternion(float x, float y, float z, float w)
     :x(x), y(y), z(z), w(w)
    {
    }

    Vector3 Quaternion::GetForward()
    {
        // Unity中 +Z作为前向。
        return Vector3(
            2.0f * (x * z + w * y),
            2.0f * (y * z - w * x),
            1.0f - 2.0f * (x * x + y * y)
        );
    }

    Vector3 Quaternion::GetUp()
    {
        return Vector3(
            2.0f * (x * y - w * z),
            1.0f - 2.0f * (x * x + z * z),
            2.0f * (y * z + w * x)
        );
    }

    Vector3 Quaternion::GetRight()
    {
        return Vector3(
            1.0f - 2.0f * (y * y + z * z),
            2.0f * (x * y + w * z),
            2.0f * (x * z - w * y)
        );
    }

    Quaternion Quaternion::FromRotationMatrix(const Vector3 &col0, const Vector3 &col1, const Vector3 &col2)
    {
        // 矩阵元素：
        // col0 = (m00, m10, m20) - 第一列
        // col1 = (m01, m11, m21) - 第二列
        // col2 = (m02, m12, m22) - 第三列
        
        float m00 = col0.x, m10 = col0.y, m20 = col0.z;
        float m01 = col1.x, m11 = col1.y, m21 = col1.z;
        float m02 = col2.x, m12 = col2.y, m22 = col2.z;
        
        float trace = m00 + m11 + m22;
        
        Quaternion q;
        
        if (trace > 0.0f)
        {
            // w 是最大的分量
            float s = std::sqrt(trace + 1.0f);
            q.w = s * 0.5f;
            s = 0.5f / s;
            q.x = (m21 - m12) * s;
            q.y = (m02 - m20) * s;
            q.z = (m10 - m01) * s;
        }
        else if (m00 >= m11 && m00 >= m22)
        {
            // x 是最大的分量
            float s = std::sqrt(1.0f + m00 - m11 - m22);
            q.x = s * 0.5f;
            s = 0.5f / s;
            q.y = (m10 + m01) * s;
            q.z = (m02 + m20) * s;
            q.w = (m21 - m12) * s;
        }
        else if (m11 > m22)
        {
            // y 是最大的分量
            float s = std::sqrt(1.0f + m11 - m00 - m22);
            q.y = s * 0.5f;
            s = 0.5f / s;
            q.x = (m10 + m01) * s;
            q.z = (m21 + m12) * s;
            q.w = (m02 - m20) * s;
        }
        else
        {
            // z 是最大的分量
            float s = std::sqrt(1.0f + m22 - m00 - m11);
            q.z = s * 0.5f;
            s = 0.5f / s;
            q.x = (m02 + m20) * s;
            q.y = (m21 + m12) * s;
            q.w = (m10 - m01) * s;
        }
        
        return q;
    }

    Quaternion Quaternion::AngleAxisX(float degree)
    {
        float radian = degree * 3.14159265f / 180.0f;
        float halfAngle = radian * 0.5f;
        return Quaternion(
            std::sin(halfAngle),  // x
            0.0f,            // y
            0.0f,            // z
            std::cos(halfAngle)   // w
        );
    }

    Quaternion Quaternion::AngleAxisY(float degree)
    {
        float radian = degree * 3.14159265f / 180.0f;
        float halfAngle = radian * 0.5f;
        return Quaternion(
            0.0f,            // x
            std::sin(halfAngle),  // y
            0.0f,            // z
            std::cos(halfAngle)   // w
        );
    }

    Quaternion Quaternion::AngleAxisZ(float degree)
    {
        float radian = degree * 3.14159265f / 180.0f;
        float halfAngle = radian * 0.5f;
        return Quaternion(
            0.0f,            // x
            0.0f,            // y
            std::sin(halfAngle),  // z
            std::cos(halfAngle)   // w
        );
    }

    // 将四元数归一化为单位四元数，模长为1
    // 旋转四元数一定是单元四元数
    Quaternion Quaternion::Normalized(const Quaternion &quaternion)
    {
        float magnitude = std::sqrt(quaternion.x * quaternion.x + 
            quaternion.y * quaternion.y + 
            quaternion.z * quaternion.z + 
            quaternion.w * quaternion.w);

        // 防止除以零
        if (magnitude < 0.0001f)
        {
        return Identity;
        }

        float invMagnitude = 1.0f / magnitude;
        return Quaternion(
            quaternion.x * invMagnitude,
            quaternion.y * invMagnitude,
            quaternion.z * invMagnitude,
            quaternion.w * invMagnitude
        );
    }

    Quaternion Quaternion::Slerp(const Quaternion &a, const Quaternion &b, float t)
    {
        return Quaternion();
    }

    Quaternion Quaternion::Lerp(const Quaternion &a, const Quaternion &b, float t)
    {
        return Quaternion();
    }

    // 相当于一个Cross操作，可以推导下
    Quaternion Quaternion::operator*(const Quaternion &other) const
    {
        return Quaternion(
            w * other.x + x * other.w + y * other.z - z * other.y,  // x
            w * other.y - x * other.z + y * other.w + z * other.x,  // y
            w * other.z + x * other.y - y * other.x + z * other.w,  // z
            w * other.w - x * other.x - y * other.y - z * other.z   // w
        );
    }

} // namespace EngineCore
