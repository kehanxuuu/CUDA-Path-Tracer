#pragma once

#include <cuda.h>
#include <cuda_runtime.h>

__host__ __device__
inline bool vecElementPositive(glm::vec3 vec) {
    return (vec.x > 0 || vec.y > 0 || vec.z > 0);
}

__host__ __device__
float fresnel(float cosThetaI, float RelIOR) {
    // accurate, not Schlick's approximation
    // cosThetaI guarantee to be > 0
    // RelIOR = extIOR / intIOR (if ray from outside) or intIOR / extIOR (otherwise)
    if (RelIOR == 1.0f)
        return 0.0f;

    float sinThetaOSqr = RelIOR * RelIOR * (1.0 - cosThetaI * cosThetaI);
    if (sinThetaOSqr > 1.0f)
        return 1.0f; // total internal reflection

    float cosThetaO = sqrt(1.0f - sinThetaOSqr);
    float Rs = (RelIOR * cosThetaI - cosThetaO) / (RelIOR * cosThetaI + cosThetaO);
    float Rp = (cosThetaI - RelIOR * cosThetaO) / (cosThetaI + RelIOR * cosThetaO);
    return (Rs * Rs + Rp * Rp) / 2.0f;
}