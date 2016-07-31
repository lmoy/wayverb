#pragma once

#include "WorkQueue.hpp"

#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/updatable.h"

#include "glm/glm.hpp"

#include "juce_opengl/juce_opengl.h"

class BaseContextLifetime : public mglu::drawable, public mglu::updatable {
public:
    void set_viewport(const glm::vec2& v);
    glm::vec2 get_viewport() const;
    float get_aspect() const;

    virtual void mouse_down(const glm::vec2& pos) = 0;
    virtual void mouse_drag(const glm::vec2& pos) = 0;
    virtual void mouse_up(const glm::vec2& pos)   = 0;
    virtual void mouse_wheel_move(float delta_y)  = 0;

private:
    virtual void viewport_changed(const glm::vec2& v);

    glm::vec2 viewport;
};

//----------------------------------------------------------------------------//

class BaseRenderer : public juce::OpenGLRenderer,
                     public juce::ChangeBroadcaster,
                     public juce::AsyncUpdater {
public:
    void set_viewport(const glm::vec2& u);

    template <typename T>
    void push_incoming(T&& t) {
        incoming_work_queue.push(std::forward<T>(t));
    }

    template <typename T>
    void push_outgoing(T&& t) {
        push_outgoing_impl(std::forward<T>(t));
    }

    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    void mouse_down(const glm::vec2& pos);
    void mouse_drag(const glm::vec2& pos);
    void mouse_up(const glm::vec2& pos);
    void mouse_wheel_move(float delta_y);

private:
    virtual BaseContextLifetime* get_context_lifetime() = 0;
    void handleAsyncUpdate() override;

    template <typename T>
    void push_outgoing_impl(T&& t) {
        outgoing_work_queue.push(std::forward<T>(t));
        triggerAsyncUpdate();
    }

    void notify_viewport_impl();

    glm::vec2 viewport;

    WorkQueue<> incoming_work_queue;
    WorkQueue<> outgoing_work_queue;
};

//----------------------------------------------------------------------------//

template <typename Renderer>
class BaseRendererComponent : public juce::Component {
public:
    template <typename... Ts>
    BaseRendererComponent(Ts&&... ts)
            : renderer(std::forward<Ts>(ts)...) {
        open_gl_context.setOpenGLVersionRequired(
                juce::OpenGLContext::openGL3_2);
        open_gl_context.setRenderer(&renderer);
        open_gl_context.setComponentPaintingEnabled(false);
        open_gl_context.setContinuousRepainting(true);
        open_gl_context.setMultisamplingEnabled(true);
        open_gl_context.attachTo(*this);
    }

    virtual ~BaseRendererComponent() noexcept { open_gl_context.detach(); }

    void resized() override {
        renderer.set_viewport(glm::vec2{getWidth(), getHeight()});
    }

    const Renderer& get_renderer() const { return renderer; }

    Renderer& get_renderer() { return renderer; }

    void mouseDown(const juce::MouseEvent& e) override {
        renderer.mouse_down(to_glm_vec2(e.getPosition()));
    }

    void mouseDrag(const juce::MouseEvent& e) override {
        renderer.mouse_drag(to_glm_vec2(e.getPosition()));
    }

    void mouseUp(const juce::MouseEvent& e) override {
        renderer.mouse_up(to_glm_vec2(e.getPosition()));
    }

    void mouseWheelMove(const juce::MouseEvent& event,
                        const juce::MouseWheelDetails& wheel) override {
        renderer.mouse_wheel_move(wheel.deltaY);
    }

protected:
    juce::OpenGLContext open_gl_context;
    Renderer renderer;

private:
    template <typename T>
    static glm::vec2 to_glm_vec2(const T& t) {
        return glm::vec2{t.x, t.y};
    }
};
