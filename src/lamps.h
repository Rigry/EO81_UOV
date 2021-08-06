#pragma once

#include "pin.h"
#include "constant.h"
#include "timers.h"
#include <bitset>
#include <array>

class Lamps : TickSubscriber {
public:
    static Lamps& make(std::array<bool, 9>& state_lamps, uint16_t& bad_lamps, uint8_t& lamps_qty)
    {
        static auto result = Lamps{
              state_lamps
            , bad_lamps
            , lamps_qty
        };
        return result;
    }


private:
    const std::array<bool, 9>& state_lamps;
    uint16_t& bad_lamps;
    const uint8_t& lamps_qty;

    Lamps(
          std::array<bool, 9>& state_lamps
        , uint16_t& bad_lamps
        , uint8_t& lamps_qty
    ) : state_lamps {state_lamps}, bad_lamps {bad_lamps}, lamps_qty{lamps_qty}
    {
        tick_subscribe();
    }

    void notify() override {
        // заполняем только первые 10
        uint16_t tmp{0};
        // TODO сюда бы пинлист из 2-ой библиотеки
        for (auto i{0}; i < lamps_qty; i++) {
            tmp |= state_lamps[i] << i;
        }
        bad_lamps = tmp;
    }
};