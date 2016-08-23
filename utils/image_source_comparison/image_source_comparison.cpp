#include "raytracer/postprocess.h"
#include "raytracer/raytracer.h"

#include "common/azimuth_elevation.h"
#include "common/cl/common.h"
#include "common/dsp_vector_ops.h"
#include "common/progress_bar.h"
#include "common/range.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/string_builder.h"
#include "common/write_audio_file.h"

#include <iostream>

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

#ifndef OBJ_PATH_TUNNEL
#define OBJ_PATH_TUNNEL ""
#endif

#ifndef OBJ_PATH_BEDROOM
#define OBJ_PATH_BEDROOM ""
#endif

#ifndef OBJ_PATH_BAD_BOX
#define OBJ_PATH_BAD_BOX ""
#endif

void run_single(const compute_context& cc,
                double speed_of_sound,
                double acoustic_impedance,
                double sample_rate,
                const glm::vec3& source,
                const model::ReceiverSettings& receiver,
                const std::pair<std::string, std::string>& stage,
                const std::pair<std::string, surface>& surf) {
    scene_data scene{std::get<1>(stage)};
    scene.set_surfaces(std::get<1>(surf));
    const voxelised_scene_data voxelised{
            scene, 5, util::padded(scene.get_aabb(), glm::vec3{0.1})};

    constexpr std::atomic_bool keep_going{true};

    const auto reflections{100};
    progress_bar pb{std::cout, reflections};
    const auto results{raytracer::run(cc,
                                      voxelised,
                                      speed_of_sound,
                                      source,
                                      receiver.position,
                                      get_random_directions(100000),
                                      reflections,
                                      10,
                                      keep_going,
                                      [&](auto) { pb += 1; })};

    auto sig{raytracer::run_attenuation(
            cc, receiver, *results, sample_rate, acoustic_impedance)};

    normalize(sig);

    snd::write(build_string(std::get<0>(stage), "_", std::get<0>(surf), ".wav"),
               {sig},
               sample_rate,
               16);
}

int main() {
    const compute_context cc{};

    constexpr glm::vec3 source{0, 1, 0};

    const model::ReceiverSettings receiver{glm::vec3{0, 1, 1}};

    const aligned::vector<std::pair<std::string, surface>> surfaces{
            std::make_pair("0", make_surface(0.99, 0.01)),
            std::make_pair("1", make_surface(0.8, 0.2)),
            std::make_pair("2", make_surface(0.7, 0.3)),
            std::make_pair("3", make_surface(0.5, 0.5)),
            std::make_pair("4", make_surface(0.3, 0.7)),
            std::make_pair("5", make_surface(0.2, 0.8)),
            std::make_pair("6", make_surface(0.01, 0.99))};

    const aligned::vector<std::pair<std::string, std::string>> objects{
            std::make_pair("vault", OBJ_PATH),
            std::make_pair("tunnel", OBJ_PATH_TUNNEL),
            std::make_pair("bedroom", OBJ_PATH_BEDROOM),
            std::make_pair("bad_box", OBJ_PATH_BAD_BOX)};

    /*
    for (auto stage : objects) {
        for (auto surf : surfaces) {
            run_single(cc,
                       340,
                       400,
                       44100,
                       glm::vec3{0, 1, 0},
                       model::ReceiverSettings{glm::vec3{0, 1, 1}},
                       stage,
                       surf);
        }
    }
    */

    run_single(cc,
               340,
               400,
               44100,
               glm::vec3{0, 1, 0},
               model::ReceiverSettings{glm::vec3{0, 1, 1}},
               objects[0],
               surfaces[1]);
}
