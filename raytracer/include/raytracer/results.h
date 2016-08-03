#pragma once

#include "raytracer/cl_structs.h"

#include "common/aligned/map.h"
#include "common/aligned/vector.h"
#include "common/cl_include.h"

#include <experimental/optional>

namespace raytracer {

class results final {
public:
    results(std::experimental::optional<impulse>&& direct,
            aligned::vector<impulse>&& image_source,
            aligned::vector<aligned::vector<impulse>>&& diffuse,
            const glm::vec3& receiver);

    aligned::vector<impulse> get_impulses(bool direct       = true,
                                          bool image_source = true,
                                          bool diffuse      = true) const;

    std::experimental::optional<impulse> get_direct() const;
    aligned::vector<impulse> get_image_source() const;
    aligned::vector<aligned::vector<impulse>> get_diffuse() const;

    glm::vec3 get_receiver() const;

private:
    std::experimental::optional<impulse> direct;
    aligned::vector<impulse> image_source;
    aligned::vector<aligned::vector<impulse>> diffuse;

    glm::vec3 receiver;
};

}  // namespace raytracer
