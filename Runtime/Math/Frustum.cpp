#include "PreCompiledHeader.h"
#include "Frustum.h"
#include <cmath> 

namespace EngineCore
{
    // 平面推导方式：
    // 比如左平面，NDC空间满足 xyz 在 -1 ~1之间
    // 也就是 -1 ≤ x / w ≤ 1;
    // 左边平面可得， x' + w' ≥ 0
    // x, w 是由 Mmvp x P得到的。也就是第一行和第四行
    // x' = m00·x + m01·y + m02·z + m03·1
    // w' = m30·x + m31·y + m32·z + m33·1
    // 那么我们已经得到了平面方程的 ABCD了， 剩下的就是normalize
    // 【注： 得到的平面朝向都指向内部】
    void Frustum::UpdateFrustumPlane(const Matrix4x4 &matrix)
    {
        // left
        frustumPlane[0] = Plane{
            Vector3(
                matrix.m00 + matrix.m30,
                matrix.m01 + matrix.m31,
                matrix.m02 + matrix.m32),
            matrix.m03 + matrix.m33};
            
        // right
        frustumPlane[1] = Plane{
            Vector3(
                matrix.m30 - matrix.m00,
                matrix.m31 - matrix.m01,
                matrix.m32 - matrix.m02),
            matrix.m33 - matrix.m03};

        // Bottom
        frustumPlane[2] = Plane{
            Vector3(
                matrix.m30 + matrix.m10,
                matrix.m31 + matrix.m11,
                matrix.m32 + matrix.m12),
            matrix.m33 + matrix.m13};

        // Top
        frustumPlane[3] = Plane{
            Vector3(
                matrix.m30 - matrix.m10,
                matrix.m31 - matrix.m11,
                matrix.m32 - matrix.m12),
            matrix.m33 - matrix.m13};

        // Near
        frustumPlane[4] = Plane{
            Vector3(
                matrix.m30 + matrix.m20,
                matrix.m31 + matrix.m21,
                matrix.m32 + matrix.m22),
            matrix.m33 + matrix.m23};

        // far
        frustumPlane[5] = Plane{
            Vector3(
                matrix.m30 - matrix.m20,
                matrix.m31 - matrix.m21,
                matrix.m32 - matrix.m22),
            matrix.m33 - matrix.m23};

        // nromalized
        for(int i = 0; i < 6; i++)
        {
            Plane& plane = frustumPlane[i];
            float length = Vector3::Length(plane.normal);
            plane.distance /= length;
            plane.normal = plane.normal / length;
        }
    }

    // 判断逻辑，normal正向说明plane指向正向，所以boundingbox
    // 距离plane最远的点是maxvalue，
    // 如果normal中某个轴是负的， 那么就要取minvalue
    // 第二个，为什么要安排Inside OutSize，这个主要是为了空间算法服务
    // 就是算到某个节点或者某个bounds已经在外部的时候，直接能优化了。
    // 但是不是为Scene服务，因为空间算法严格保证父子关系和大小关系的对齐
    IntersectResult Frustum::TestAABB(const AABB& bound)
    {
        IntersectResult result = IntersectResult::Inside;
        for(int i = 0; i < 6; i++)
        {
            Plane& plane = frustumPlane[i];
            Vector3 positiveVertex = bound.minValue;
            if(plane.normal.x >= 0) positiveVertex.x = bound.maxValue.x;
            if(plane.normal.y >= 0) positiveVertex.y = bound.maxValue.y;
            if(plane.normal.z >= 0) positiveVertex.z = bound.maxValue.z;
            
            Vector3 negativeVertex = bound.maxValue;
            if(plane.normal.x >= 0) negativeVertex.x = bound.minValue.x;
            if(plane.normal.y >= 0) negativeVertex.y = bound.minValue.y;
            if(plane.normal.z >= 0) negativeVertex.z = bound.minValue.z;

            if(plane.GetDistanceToPoint(positiveVertex) < 0)
            {
                return IntersectResult::Outside;
            }

            if(plane.GetDistanceToPoint(negativeVertex) < 0)
            {
                return IntersectResult::Intersect;
            }
            return result;
        }

    }

}
