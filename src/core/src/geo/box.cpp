#include "core/geo/box.h"
#include "core/geo/geometric.h"
#include "core/geo/tri_cube_intersection.h"
#include "core/geo/triangle_vec.h"
#include "core/scene_data.h"

#include "utilities/mapping_iterator_adapter.h"

namespace wayverb {
namespace core {
namespace geo {

glm::vec3 mirror_inside(const box& b, const glm::vec3& v, direction d) {
    return mirror_on_axis(v, centre(b), d);
}

box mirror(const box& b, wall w) {
    return box(mirror(b, b.get_min(), w), mirror(b, b.get_max(), w));
}

bool overlaps(const box& b, const triangle_vec3& t) {
    auto coll = t;
    for (auto& i : coll.s) {
        i = (i - centre(b)) / dimensions(b);
    }
    return t_c_intersection(coll) == where::inside;
}

std::optional<std::pair<float, float>> intersection_distances(
        const box& b, const ray& ray) {
    /// from http://people.csail.mit.edu/amy/papers/box-jgt.pdf
    const auto inv = 1.0f / ray.get_direction();
    const auto sign = glm::lessThan(inv, glm::vec3{0});
    const auto get_bounds = [&](auto ind) {
        return sign[ind] ? glm::vec2{b.get_max()[ind], b.get_min()[ind]}
                         : glm::vec2{b.get_min()[ind], b.get_max()[ind]};
    };

    const auto xbounds = get_bounds(0);
    auto t = (xbounds - ray.get_position().x) * inv.x;
    const auto ybounds = get_bounds(1);
    const auto ty = (ybounds - ray.get_position().y) * inv.y;
    if (ty[1] < t[0] || t[1] < ty[0]) {
        return std::nullopt;
    }
    t[0] = std::max(ty[0], t[0]);
    t[1] = std::min(ty[1], t[1]);
    const auto zbounds = get_bounds(2);
    const auto tz = (zbounds - ray.get_position().z) * inv.z;
    if (tz[1] < t[0] || t[1] < tz[0]) {
        return std::nullopt;
    }
    return std::make_pair(std::max(tz[0], t[0]), std::min(tz[1], t[1]));
}

std::optional<float> intersects(const box& b, const ray& ray) {
    if (const auto i = intersection_distances(b, ray)) {
        if (0 < i->first) {
            return i->first;
        }
        if (0 < i->second) {
            return i->second;
        }
    }
    return std::nullopt;
}

bool intersects(const box& b, const ray& ray, float t0, float t1) {
    if (const auto i = intersection_distances(b, ray)) {
        return i->first < t1 && t0 < i->second;
    }
    return false;
}

template <typename T>
geo::box compute_aabb(const T* b, const T* e) {
    const auto make_iterator = [](auto it) {
        return util::make_mapping_iterator_adapter(std::move(it), to_vec3{});
    };
    return util::enclosing_range(make_iterator(b), make_iterator(e));
}

template geo::box compute_aabb<glm::vec3>(const glm::vec3* b,
                                          const glm::vec3* e);

template geo::box compute_aabb<cl_float3>(const cl_float3* b,
                                          const cl_float3* e);

}  // namespace geo
}  // namespace core
}  // namespace wayverb
