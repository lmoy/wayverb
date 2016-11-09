#include "combined/capsule_base.h"
#include "combined/engine.h"

#include "core/attenuator/hrtf.h"
#include "core/attenuator/microphone.h"

namespace wayverb {
namespace combined {

template <typename T>
class capsule final : public capsule_base {
public:
    capsule() = default;
    explicit capsule(const T& attenuator)
            : attenuator_{attenuator} {}

    std::unique_ptr<capsule_base> clone() const override {
        return std::make_unique<capsule>(*this);
    }

    util::aligned::vector<float> postprocess(
            const intermediate& intermediate,
            double sample_rate) const override {
        return intermediate.postprocess(attenuator_, sample_rate);
    }

    //glm::vec3 get_pointing() const override {
    //    return attenuator_.get_pointing();
    //}

private:
    T attenuator_;
};

std::unique_ptr<capsule_base> make_capsule_ptr(
        const core::attenuator::hrtf& attenuator) {
    return std::make_unique<capsule<core::attenuator::hrtf>>(attenuator);
}

std::unique_ptr<capsule_base> make_capsule_ptr(
        const core::attenuator::microphone& attenuator) {
    return std::make_unique<capsule<core::attenuator::microphone>>(attenuator);
}

}  // namespace combined
}  // namespace wayverb
