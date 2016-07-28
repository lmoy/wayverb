#pragma once

#include "box.h"
#include "reduce.h"
#include "scene_data.h"
#include "triangle.h"
#include "triangle_vec.h"

#include "common/aligned/vector.h"

#include "glm/glm.hpp"

namespace geo {
class Ray;
}  // namespace geo

class Boundary {
public:
    Boundary()                    = default;
    virtual ~Boundary() noexcept  = default;
    Boundary(Boundary&&) noexcept = default;
    Boundary& operator=(Boundary&&) noexcept = default;
    Boundary(const Boundary&)                = default;
    Boundary& operator=(const Boundary&) = default;

    virtual bool inside(const glm::vec3& v) const = 0;
    virtual box get_aabb() const                  = 0;

    template <typename Archive>
    void serialize(Archive& archive);
};

class CuboidBoundary : public Boundary {
public:
    CuboidBoundary() = default;
    CuboidBoundary(const glm::vec3& c0, const glm::vec3& c1);

    bool inside(const glm::vec3& v) const override;
    box get_aabb() const override;

    template <typename Archive>
    void serialize(Archive& archive);

private:
    box boundary;
};

class SphereBoundary : public Boundary {
public:
    explicit SphereBoundary(const glm::vec3& c = glm::vec3(),
                            float radius       = 0,
                            const aligned::vector<Surface>& surfaces =
                                    aligned::vector<Surface>{Surface{}});
    bool inside(const glm::vec3& v) const override;
    box get_aabb() const override;

private:
    glm::vec3 c;
    float radius;
    box boundary;
};

template <typename T>
inline bool almost_equal(T x, T y, int ups) {
    return std::abs(x - y) <= std::numeric_limits<T>::epsilon() *
                                      std::max(std::abs(x), std::abs(y)) * ups;
}

class MeshBoundary : public Boundary {
public:
    MeshBoundary(const aligned::vector<Triangle>& triangles,
                 const aligned::vector<glm::vec3>& vertices,
                 const aligned::vector<Surface>& surfaces);
    explicit MeshBoundary(const CopyableSceneData& sd);
    bool inside(const glm::vec3& v) const override;
    box get_aabb() const override;

    aligned::vector<size_t> get_triangle_indices() const;

    using reference_store = aligned::vector<uint32_t>;

    glm::ivec3 hash_point(const glm::vec3& v) const;
    const reference_store& get_references(int x, int y) const;
    const reference_store& get_references(const glm::ivec3& i) const;

    static constexpr int DIVISIONS = 1024;

    const aligned::vector<Triangle>& get_triangles() const;
    const aligned::vector<glm::vec3>& get_vertices() const;
    const aligned::vector<Surface>& get_surfaces() const;
    glm::vec3 get_cell_size() const;

private:
    using hash_table = aligned::vector<aligned::vector<reference_store>>;

    hash_table compute_triangle_references() const;

    aligned::vector<Triangle> triangles;
    aligned::vector<glm::vec3> vertices;
    aligned::vector<Surface> surfaces;
    box boundary;
    glm::vec3 cell_size;
    hash_table triangle_references;

    static const reference_store empty_reference_store;
};
