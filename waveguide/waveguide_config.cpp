#include "waveguide_config.h"
#include "samplerate.h"

namespace config {

float Waveguide::get_max_frequency() const {
    return filter_frequency * oversample_ratio;
}

float Waveguide::get_waveguide_sample_rate() const {
    return get_max_frequency() * 4;
}

float Waveguide::get_divisions() const {
    return (SPEED_OF_SOUND * sqrt(3)) / get_waveguide_sample_rate();
}
}

std::vector<float> adjust_sampling_rate(std::vector<float>& w_results,
                                        const config::Waveguide& cc) {
    std::vector<float> out_signal(cc.sample_rate * w_results.size() /
                                  cc.get_waveguide_sample_rate());

    SRC_DATA sample_rate_info{
        w_results.data(),
        out_signal.data(),
        long(w_results.size()),
        long(out_signal.size()),
        0,
        0,
        0,
        cc.sample_rate / double(cc.get_waveguide_sample_rate())};

    src_simple(&sample_rate_info, SRC_SINC_BEST_QUALITY, 1);
    return out_signal;
}
