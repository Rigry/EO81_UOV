#pragma once

#include <array>
#include "constant.h"
#include "timers.h"

namespace {
    constexpr auto remember_every        {5000};
    constexpr auto update_every_remember {1};
}

// каждая плата расширения будет считать свои лампы
// поэтому тут только 10
struct Minutes {
    std::array<uint32_t, 10> data {0};
};

// считает время работы каждой лампы в минутах
struct Work_count : TickSubscriber {
    int tick_cnt {0};
    Minutes minutes;
    const uint16_t& bad_lamps;
    std::array<uint16_t, glob::max_lamps>& hours;

    Work_count (
          uint16_t& bad_lamps
        , std::array<uint16_t, glob::max_lamps>& hours
    ) : bad_lamps {bad_lamps}, hours {hours}
    {}

    Minutes* get_data() { return &minutes; }
    void start() { tick_subscribe(); }
    void stop() { tick_unsubscribe(); }

    void notify() override {
        if (++tick_cnt == remember_every) {
            tick_cnt = 0;
            for (auto i {0}; i < 10; i++) {
                minutes.data[i] += not ((bad_lamps >> i) & 0b1);
                hours[i] = get_hours(i);
            }
        }
    }

    uint16_t get_hours (int i) { return minutes.data[i] / update_every_remember; }
};