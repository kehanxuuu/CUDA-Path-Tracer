#pragma once

#include "intersections.h"

// CHECKITOUT
/**
 * Computes a cosine-weighted random direction in a hemisphere.
 * Used for diffuse lighting.
 */
__host__ __device__
glm::vec3 calculateRandomDirectionInHemisphere(
        glm::vec3 normal, thrust::default_random_engine &rng) {
    thrust::uniform_real_distribution<float> u01(0, 1);

    float up = sqrt(u01(rng)); // cos(theta)
    float over = sqrt(1 - up * up); // sin(theta)
    float around = u01(rng) * TWO_PI;

    // Find a direction that is not the normal based off of whether or not the
    // normal's components are all equal to sqrt(1/3) or whether or not at
    // least one component is less than sqrt(1/3). Learned this trick from
    // Peter Kutz.

    glm::vec3 directionNotNormal;
    if (abs(normal.x) < SQRT_OF_ONE_THIRD) {
        directionNotNormal = glm::vec3(1, 0, 0);
    } else if (abs(normal.y) < SQRT_OF_ONE_THIRD) {
        directionNotNormal = glm::vec3(0, 1, 0);
    } else {
        directionNotNormal = glm::vec3(0, 0, 1);
    }

    // Use not-normal direction to generate two perpendicular directions
    glm::vec3 perpendicularDirection1 =
        glm::normalize(glm::cross(normal, directionNotNormal));
    glm::vec3 perpendicularDirection2 =
        glm::normalize(glm::cross(normal, perpendicularDirection1));

    return up * normal
        + cos(around) * over * perpendicularDirection1
        + sin(around) * over * perpendicularDirection2;
}

/**
 * Scatter a ray with some probabilities according to the material properties.
 * For example, a diffuse surface scatters in a cosine-weighted hemisphere.
 * A perfect specular surface scatters in the reflected ray direction.
 * In order to apply multiple effects to one surface, probabilistically choose
 * between them.
 *
 * The visual effect you want is to straight-up add the diffuse and specular
 * components. You can do this in a few ways. This logic also applies to
 * combining other types of materias (such as refractive).
 *
 * - Always take an even (50/50) split between a each effect (a diffuse bounce
 *   and a specular bounce), but divide the resulting color of either branch
 *   by its probability (0.5), to counteract the chance (0.5) of the branch
 *   being taken.
 *   - This way is inefficient, but serves as a good starting point - it
 *     converges slowly, especially for pure-diffuse or pure-specular.
 * - Pick the split based on the intensity of each material color, and divide
 *   branch result by that branch's probability (whatever probability you use).
 *
 * This method applies its changes to the Ray parameter `ray` in place.
 * It also modifies the color `color` of the ray in place.
 *
 * You may need to change the parameter list for your purposes!
 */
__host__ __device__
void scatterRay(
        PathSegment & pathSegment,
        glm::vec3 intersect, // position
        glm::vec3 normal, // points in the direction of intersection
        bool outside, // whether the normal points outside
        const Material &mat,
        thrust::default_random_engine &rng) {
    // TODO: implement this.
    // A basic implementation of pure-diffuse shading will just call the
    // calculateRandomDirectionInHemisphere defined above.
    if (vecElementPositive(mat.emittance)) {
        pathSegment.radiance += pathSegment.throughput * mat.emittance;
    }

    glm::vec3 inDir = -pathSegment.ray.direction; // point out towards the surface

    if (mat.type == Material::Type::Diffuse) {
        glm::vec3 outDir = calculateRandomDirectionInHemisphere(normal, rng);
        pathSegment.ray = makeOffsetedRay(intersect, outDir);
        pathSegment.throughput *= mat.albedo;
    }
    else if (mat.type == Material::Type::Mirror) {
        glm::vec3 outDir = glm::reflect(-inDir, normal); // 2 * glm::dot(inDir, normal) * normal - inDir
        pathSegment.ray = makeOffsetedRay(intersect, outDir);
        pathSegment.throughput *= mat.albedo; // note: either 1.0 or color, both not physically accurate
    }
    else if (mat.type == Material::Type::Dielectric) {
        float cosTheta = glm::dot(inDir, normal);
        float IOR = outside ? mat.indexOfRefraction : 1.0 / mat.indexOfRefraction;
        float fresnelTerm = fresnel(cosTheta, IOR);

        thrust::uniform_real_distribution<float> u01(0, 1);
        glm::vec3 outDir;
        
        if (u01(rng) <= fresnelTerm) {
            // reflects
            outDir = glm::reflect(-inDir, normal);
        }
        else {
            // refract
            outDir = glm::refract(-inDir, normal, IOR);
            // float sqrtTerm = sqrt(1.0 - IOR * IOR * (1.0 - cosTheta * cosTheta));
            // outDir = - IOR * (inDir - cosTheta * normal) - sqrtTerm * normal;
            outDir = glm::normalize(outDir);
        }
        
        pathSegment.ray = makeOffsetedRay(intersect, outDir);
        pathSegment.throughput *= mat.albedo; // note: either 1.0 or color, both not physically accurate
    }

    pathSegment.remainingBounces -= 1;
}
