#include "ShaderStructs.hlsli"
//
////https://web.archive.org/web/20140224205853/http://obge.paradice-insight.us/wiki/Includes_(Effects)
//float rand_1_05(in float2 uv)
//{
//    float2 noise = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
//    return clamp(abs(noise.x + noise.y) * 0.5, 0, 1);
//}
//
//bool solveQuadratic(float a, float b, float c, float& x1, float& x2)
//{
//    if (b == 0) {
//        // Handle special case where the the two vector ray.dir and V are perpendicular
//        // with V = ray.orig - sphere.centre
//        if (a == 0) return false;
//        x1 = 0; x2 = sqrt(-c / a);
//        return true;
//    }
//    float discr = b * b - 4 * a * c;
//
//    if (discr < 0) return false;
//
//    float q = (b < 0.f) ? -0.5f * (b - sqrt(discr)) : -0.5f * (b + sqrt(discr));
//    x1 = q / a;
//    x2 = c / q;
//
//    return true;
//}
//
//bool raySphereIntersect(const float3 orig, const float3 dir, const float radius, float& t0, float& t1)
//{
//    // They ray dir is normalized so A = 1 
//    float A = pow(dir.x,2) + pow(dir.y, 2) + pow(dir.z, 2);
//    float B = 2 * (dir.x * orig.x + dir.y * orig.y + dir.z * orig.z);
//    float C = pow(orig.x, 2) + pow(orig.y, 2) + pow(orig.z, 2) - pow(radius, 2);
//
//    if (!solveQuadratic(A, B, C, t0, t1)) return false;
//
//    if (t0 > t1) swap(t0, t1);
//
//    return true;
//}
//
float4 main(in SkyboxVertexToPixel input) : SV_TARGET
{ return input.col; };
//    float earthRadius = 1000.f;
//    // Render from a normal camera
//    float aspectRatio = 0.2f;// width / float(height);
//    float fov = 65;
//    float angle = tan(radians(fov) * 0.5f);
//    unsigned int numPixelSamples = 4;
//    float3 orig{ 0.f, earthRadius + 1000.f, 30000.f }; // camera position 
//
//    for (unsigned int m = 0; m < numPixelSamples; ++m) {
//        for (unsigned int n = 0; n < numPixelSamples; ++n) {
//            float rayx = (2 * (x + (m + rand_1_05(input.tex)) / numPixelSamples) / float(width) - 1) * aspectRatio * angle;
//            float rayy = (1 - (y + (n + rand_1_05(input.tex)) / numPixelSamples) / float(height) * 2) * angle;
//            float3 dir(rayx, rayy, -1);
//            normalize(dir);
//            // Does the ray intersect the planetory body? (the intersection test is against the Earth here
//            // not against the atmosphere). If the ray intersects the Earth body and that the intersection
//            // is ahead of us, then the ray intersects the planet in 2 points, t0 and t1. But we
//            // only want to comupute the atmosphere between t=0 and t=t0 (where the ray hits
//            // the Earth first). If the viewing ray doesn't hit the Earth, or course the ray
//            // is then bounded to the range [0:INF]. In the method computeIncidentLight() we then
//            // compute where this primary ray intersects the atmosphere and we limit the max t range 
//            // of the ray to the point where it leaves the atmosphere.
//            float t0, t1, tMax = kInfinity;
//            if (raySphereIntersect(orig, dir, atmosphere.earthRadius, t0, t1) && t1 > 0)
//                tMax = max(0.f, t0);
//            // The *viewing or camera ray* is bounded to the range [0:tMax]
//            *p += atmosphere.computeIncidentLight(orig, dir, 0, tMax);
//        }
//    }
//    *p *= 1.f / (numPixelSamples * numPixelSamples);
//}