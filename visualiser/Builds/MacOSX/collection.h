#pragma once

#include "value_wrapper.h"

#include <vector>
#include <cassert>

namespace model {

template <typename It, typename... Ts, typename Func>
void for_each(Func&& f, It first, It last, Ts&&... ts) {
    while (first != last) {
        f(*first++, (*ts++)...);
    }
}

template <typename T>
class ValueWrapper<std::vector<T>> : public NestedValueWrapper<std::vector<T>> {
public:
    ValueWrapper(ModelMember* parent, std::vector<T>& t)
            : NestedValueWrapper<std::vector<T>>(parent, t)
            , wrappers(t.size()) {
        this->reseat(t);
    }

    void set(const std::vector<T>& u, bool do_notify = true) override {
        *(this->t) = u;
        reseat_and_notify(do_notify);
    }

    ValueWrapper<T>& at(size_t pos) {
        return *wrappers.at(pos);
    }
    const ValueWrapper<T>& at(size_t pos) const {
        return *wrappers.at(pos);
    }

    ValueWrapper<T>& operator[](size_t pos) {
        return *wrappers[pos];
    }
    const ValueWrapper<T>& operator[](size_t pos) const {
        return *wrappers[pos];
    }

    ValueWrapper<T>& front() {
        return *wrappers.front();
    }
    const ValueWrapper<T>& front() const {
        return *wrappers.front();
    }

    ValueWrapper<T>& back() {
        return *wrappers.back();
    }
    const ValueWrapper<T>& back() const {
        return *wrappers.back();
    }

    bool empty() const {
        return wrappers.empty();
    }
    size_t size() const {
        return wrappers.size();
    }

    auto begin() {
        return wrappers.begin();
    }

    auto begin() const {
        return wrappers.begin();
    }

    auto cbegin() const {
        return wrappers.cbegin();
    }

    auto end() {
        return wrappers.end();
    }

    auto end() const {
        return wrappers.end();
    }

    auto cend() const {
        return wrappers.cend();
    }

    //  modifiers
    void clear(bool do_notify = true) {
        wrappers.clear();
        this->t->clear();
        reseat_and_notify(do_notify);
    }

    void insert(size_t pos, const T& value, bool do_notify = true) {
        this->t->insert(this->t->begin() + pos, value);
        wrappers.insert(wrappers.begin() + pos, nullptr);
        reseat_and_notify(do_notify);
    }

    void erase(size_t pos, bool do_notify = true) {
        this->t->erase(this->t->begin() + pos);
        wrappers.erase(wrappers.begin() + pos);
        reseat_and_notify(do_notify);
    }

    void push_back(const T& value, bool do_notify = true) {
        this->t->push_back(value);
        wrappers.push_back(nullptr);
        reseat_and_notify(do_notify);
    }

    void pop_back(bool do_notify = true) {
        this->t->pop_back();
        wrappers.pop_back();
        reseat_and_notify(do_notify);
    }

    void resize(size_t num, const T& value = T(), bool do_notify = true) {
        this->t->resize(num, value);
        wrappers.resize(num, nullptr);
        reseat_and_notify(do_notify);
    }

    void reseat(std::vector<T>& u) override {
        assert(u.size() == wrappers.size());
        for_each(
            [this](auto& i, auto& j) {
                if (j) {
                    j->reseat(i);
                } else {
                    j = std::make_unique<ValueWrapper<T>>(this, i);
                }
            },
            u.begin(),
            u.end(),
            wrappers.begin());
    }

private:
    void reseat_and_notify(bool do_notify) {
        this->reseat(*(this->t));
        this->notify(do_notify);
    }

    std::vector<std::unique_ptr<ValueWrapper<T>>> wrappers;
};
};
