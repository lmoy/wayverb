#include "box/img_src.h"
#include "box/waveguide.h"

#include "common/attenuator/hrtf.h"
#include "common/attenuator/microphone.h"
#include "common/cl/iterator.h"
#include "common/reverb_time.h"

#include "utilities/aligned/map.h"
#include "utilities/string_builder.h"

#include "audio_file/audio_file.h"

#include <iostream>

//  TODO test raytracer diffuse output to compenstate for fast img-src decay
//
//  TODO bidirectional mic outputs modal responses don't match

template <typename It>
void normalize(It begin, It end) {
    const auto max_mags{
            map_to_vector(begin, end, [](auto i) { return max_mag(i); })};
    const auto mag{*std::max_element(max_mags.begin(), max_mags.end())};
    std::for_each(begin, end, [=](auto& vec) {
        for (auto& samp : vec) {
            samp /= mag;
        }
    });
}

int main(int argc, char** argv) {
    //  constants ------------------------------------------------------------//

    const geo::box box{glm::vec3{0}, glm::vec3{5.56, 3.97, 2.81}};
    constexpr auto sample_rate{16000.0};
    constexpr auto speed_of_sound{340.0};
    constexpr auto acoustic_impedance{400.0f};

    constexpr glm::vec3 source{2.09, 2.12, 2.12};
    constexpr glm::vec3 receiver{2.09, 3.08, 0.96};
    const glm::vec3 pointing{0, 0, 1};
    const glm::vec3 up{0, 1, 0};

    constexpr auto reflectance{0.95};
    const auto absorption{1 - pow(reflectance, 2)};

    const aligned::map<std::string, float> polar_pattern_map{
            {"omnidirectional", 0.0f},
            {"cardioid", 0.5f},
            {"bidirectional", 1.0f}};

    const auto eyring{
            eyring_reverb_time(geo::get_scene_data(box, absorption), 0)};

    std::cerr << "expected reverb time: " << eyring << '\n';

    //  tests ----------------------------------------------------------------//

    const auto waveguide{run_waveguide(box,
                                       absorption,
                                       source,
                                       receiver,
                                       speed_of_sound,
                                       acoustic_impedance,
                                       sample_rate,
                                       eyring)};

    const auto exact_img_src{run_exact_img_src(
            box, absorption, source, receiver, speed_of_sound, eyring)};

    const auto fast_img_src{
            run_fast_img_src(box, absorption, source, receiver)};

    //  postprocessing -------------------------------------------------------//

    const auto postprocess{[&](auto prefix, auto attenuator) {
        auto waveguide_p{postprocess_waveguide(begin(waveguide),
                                               end(waveguide),
                                               attenuator,
                                               sample_rate,
                                               acoustic_impedance)};

        const auto impulses_p{[&](auto begin, auto end) {
            return postprocess_impulses(begin,
                                        end,
                                        receiver,
                                        attenuator,
                                        speed_of_sound,
                                        acoustic_impedance,
                                        sample_rate);
        }};

        auto exact_img_src_p{
                impulses_p(begin(exact_img_src), end(exact_img_src))};
        auto fast_img_src_p{impulses_p(begin(fast_img_src), end(fast_img_src))};

        const auto outputs = {&waveguide_p, &exact_img_src_p, &fast_img_src_p};

        const auto make_iterator{[](auto i) {
            return make_mapping_iterator_adapter(
                    std::move(i), [](auto& i) -> auto& { return *i; });
        }};
        normalize(make_iterator(outputs.begin()), make_iterator(outputs.end()));

        write(build_string(prefix, ".waveguide.wav"),
              audio_file::make_audio_file(waveguide_p, sample_rate),
              16);
        write(build_string(prefix, ".exact_img_src.wav"),
              audio_file::make_audio_file(exact_img_src_p, sample_rate),
              16);
        write(build_string(prefix, ".fast_img_src.wav"),
              audio_file::make_audio_file(fast_img_src_p, sample_rate),
              16);
    }};

    //  simulations ----------------------------------------------------------//


    for (const auto& pair : polar_pattern_map) {
        postprocess(pair.first, microphone{pointing, pair.second});
    }

    postprocess("hrtf_l", hrtf{pointing, up, hrtf::channel::left});
    postprocess("hrtf_r", hrtf{pointing, up, hrtf::channel::right});

    return EXIT_SUCCESS;
}
