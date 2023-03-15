#pragma once

#include <map>
#include <string>
#include "glm/glm.hpp"

struct Material {
    enum Type {
        Diffuse,
        Mirror,
        Dielectric
    };

    int type;
    glm::vec3 albedo;
    glm::vec3 emittance;
    float indexOfRefraction; // extIOR / intIOR (world space) -> convenient for coding
    // input: intIOR / extIOR -> convenient for setting input value -> take inverse when parsing scene files
};