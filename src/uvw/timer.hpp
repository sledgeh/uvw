#pragma once


#include <utility>
#include <memory>
#include <chrono>
#include <uv.h>
#include "error.h"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


class Timer final: public Handle<Timer> {
    static void startCallback(Timer &timer, uv_timer_t *) {
        timer.publish(TimerEvent{});
    }

    explicit Timer(std::shared_ptr<Loop> ref)
        : Handle{ResourceType<uv_timer_t>{}, std::move(ref)}
    {
        initialized = (uv_timer_init(parent(), get<uv_timer_t>()) == 0);
    }

public:
    using Time = std::chrono::milliseconds;

    template<typename... Args>
    static std::shared_ptr<Timer> create(Args&&... args) {
        return std::shared_ptr<Timer>{new Timer{std::forward<Args>(args)...}};
    }

    void start(Time timeout, Time repeat) {
        using CBF = CallbackFactory<void(uv_timer_t *)>;
        auto func = CBF::create<&Timer::startCallback>(*this);
        auto err = uv_timer_start(get<uv_timer_t>(), func, timeout.count(), repeat.count());
        if(err) publish(ErrorEvent{err});
    }

    void stop() {
        auto err = uv_timer_stop(get<uv_timer_t>());
        if(err) publish(ErrorEvent{err});
    }

    void again() {
        auto err = uv_timer_again(get<uv_timer_t>());
        if(err) publish(ErrorEvent{err});
    }

    void repeat(Time repeat) {
        uv_timer_set_repeat(get<uv_timer_t>(), repeat.count());
    }

    Time repeat() {
        return Time{uv_timer_get_repeat(get<uv_timer_t>())};
    }

    explicit operator bool() const noexcept { return initialized; }

private:
    bool initialized;
};


}
