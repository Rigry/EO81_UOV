#pragma once

#include <array>
#include "constant.h"
#include "timers.h"

struct Minutes {
    // std::array<uint32_t, glob::max_lamps> data {0};
    std::array<uint32_t, 10> data {0}; // пока без плат расширения
};

// считает время работы каждой лампы в минутах
struct Work_count : TickSubscriber {
    int tick_cnt {0};
    Minutes minutes;

    Minutes* get_data() { return &minutes; }
    void start() { tick_subscribe(); }
    void stop() { tick_unsubscribe(); }

    Work_count() {}

    void notify() override {
        if (++tick_cnt == 60000) {
            tick_cnt = 0;
            // если лампа включена, то инкрементировать минуты
            // если при этом перевалили за час, то обновить модбас
        }
    }
};