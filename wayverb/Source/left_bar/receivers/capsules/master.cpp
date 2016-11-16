#include "master.h"
#include "PolarPatternDisplay.h"

#include "../../azimuth_elevation_property.h"
#include "../../slider_property.h"

#include "combined/model/capsule.h"

#include "core/az_el.h"
#include "core/orientation.h"

namespace left_bar {
namespace receivers {
namespace capsules {

template <typename T>
class generic_property_component final : public PropertyComponent {
public:
    template <typename... Ts>
    generic_property_component(const String& name, int height, Ts&&... ts)
            : PropertyComponent{name, height}
            , content{std::forward<Ts>(ts)...} {
        addAndMakeVisible(content);
    }

    void refresh() override {}

    T content;
};

class ear_picker final : public Component, public TextButton::Listener {
public:
    ear_picker() {
        l_button_.setRadioGroupId(1);
        r_button_.setRadioGroupId(1);

        addAndMakeVisible(l_button_);
        addAndMakeVisible(r_button_);
    }

    void set(wayverb::core::attenuator::hrtf::channel channel) {
        switch (channel) {
            case wayverb::core::attenuator::hrtf::channel::left:
                l_button_.setToggleState(true, dontSendNotification);
                break;
            case wayverb::core::attenuator::hrtf::channel::right:
                r_button_.setToggleState(true, dontSendNotification);
                break;
        }
    }

    void buttonClicked(Button* b) override {
        if (b == &l_button_) {
            on_change_(*this, wayverb::core::attenuator::hrtf::channel::left);
        } else if (b == &r_button_) {
            on_change_(*this, wayverb::core::attenuator::hrtf::channel::right);
        }
    }

    using on_change = util::event<ear_picker&,
                                  wayverb::core::attenuator::hrtf::channel>;
    on_change::connection connect_on_change(on_change::callback_type callback) {
        return on_change_.connect(std::move(callback));
    }

    void resized() override {
        const auto bounds = getLocalBounds();
        l_button_.setBounds(bounds.withTrimmedRight(getWidth() / 2 + 1));
        r_button_.setBounds(bounds.withTrimmedLeft(getWidth() / 2 + 1));
    }

private:
    TextButton l_button_{"left"};
    model::Connector<TextButton> l_button_connector_{&l_button_, this};
    
    TextButton r_button_{"right"};
    model::Connector<TextButton> r_button_connector_{&r_button_, this};

    on_change on_change_;
};

class hrtf_properties final : public PropertyPanel {
public:
    hrtf_properties(wayverb::combined::model::hrtf& model)
            : model_{model} {
        auto orientation =
                std::make_unique<azimuth_elevation_property>("orientation");
        auto channel = std::make_unique<generic_property_component<ear_picker>>("ear", 25);

        auto update_from_hrtf =
                [ this, o = orientation.get(), c = channel.get() ](auto& hrtf) {
            o->set(wayverb::core::compute_azimuth_elevation(
                    hrtf.get().orientation.get_pointing()));
            c->content.set(hrtf.get().get_channel());
        };

        update_from_hrtf(model_);

        connection_ = wayverb::combined::model::hrtf::scoped_connection{
                model_.connect(update_from_hrtf)};

        orientation->connect_on_change([this](auto&, auto orientation) {
            model_.set_orientation(
                    wayverb::core::orientation{compute_pointing(orientation)});
        });

        channel->content.connect_on_change(
                [this](auto&, auto channel) { model_.set_channel(channel); });

        addProperties({orientation.release()});
        addProperties({channel.release()});
    }

private:
    wayverb::combined::model::hrtf& model_;
    wayverb::combined::model::hrtf::scoped_connection connection_;
};

////////////////////////////////////////////////////////////////////////////////

class microphone_properties final : public PropertyPanel {
public:
    microphone_properties(wayverb::combined::model::microphone& model)
            : model_{model} {
        auto orientation =
                std::make_unique<azimuth_elevation_property>("orientation");
        auto shape = std::make_unique<slider_property>("shape", 0, 1);
        auto display = std::make_unique<PolarPatternProperty>("shape display", 80);

        auto update_from_microphone =
                [ this, o = orientation.get(), s = shape.get(), d = display.get() ](auto& mic) {
            o->set(wayverb::core::compute_azimuth_elevation(
                    mic.get().orientation.get_pointing()));
            s->set(mic.get().get_shape());
            d->set_shape(mic.get().get_shape());
        };

        update_from_microphone(model_);

        connection_ = wayverb::combined::model::microphone::scoped_connection{
                model_.connect(update_from_microphone)};

        orientation->connect_on_change([this](auto&, auto orientation) {
            model_.set_orientation(
                    wayverb::core::orientation{compute_pointing(orientation)});
        });

        shape->connect_on_change(
                [this](auto&, auto shape) { model_.set_shape(shape); });

        addProperties({orientation.release()});
        addProperties({shape.release()});
        addProperties({display.release()});
    }

private:
    wayverb::combined::model::microphone& model_;
    wayverb::combined::model::microphone::scoped_connection connection_;
};

class capsule_editor final : public TabbedComponent {
public:
    capsule_editor(wayverb::combined::model::capsule& model)
            : TabbedComponent{TabbedButtonBar::Orientation::TabsAtTop}
            , model_{model} {
        addTab("microphone",
               Colours::darkgrey,
               new microphone_properties{model.microphone()},
               true);
        addTab("hrtf", Colours::darkgrey, new hrtf_properties{model.hrtf()}, true);

        auto update_from_capsule = [this](auto& m) {
            switch (m.get_mode()) {
                case wayverb::combined::model::capsule::mode::microphone:
                    setCurrentTabIndex(0, dontSendNotification);
                    break;
                case wayverb::combined::model::capsule::mode::hrtf:
                    setCurrentTabIndex(1, dontSendNotification);
                    break;
            }
        };

        update_from_capsule(model_);
        
        connection_ = wayverb::combined::model::capsule::scoped_connection{model_.connect(update_from_capsule)};

        initializing_ = false;
    }

    void currentTabChanged(int newCurrentTabIndex, const String& name) override {
        if (! initializing_) {
            switch (newCurrentTabIndex) {
                case 0:
                    model_.set_mode(wayverb::combined::model::capsule::mode::microphone);
                    break;

                case 1:
                    model_.set_mode(wayverb::combined::model::capsule::mode::hrtf);
                    break;
            }
        }
    }

private:
    wayverb::combined::model::capsule& model_;
    wayverb::combined::model::capsule::scoped_connection connection_;
    bool initializing_ = true;
};

////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Component> capsule_config_item::get_callout_component(
        wayverb::combined::model::capsule& model) {
    auto ret = std::make_unique<capsule_editor>(model);
    ret->setSize(300, 200);
    return std::move(ret);
}

////////////////////////////////////////////////////////////////////////////////

master::master(
        wayverb::combined::model::vector<wayverb::combined::model::capsule, 1>&
                model)
        : list_box_{model} {
    list_box_.setRowHeight(30);
    addAndMakeVisible(list_box_);
}

void master::resized() { list_box_.setBounds(getLocalBounds()); }

}  // namespace capsules
}  // namespace receivers
}  // namespace left_bar
