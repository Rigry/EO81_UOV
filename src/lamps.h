#pragma once

#include "pin.h"
#include "constant.h"
#include "timers.h"
#include <bitset>

// TODO знаю что плохо, но спешил
template<class T>
auto from_tuple(T in) {
    return std::array{
        &std::get<0>(in),
        &std::get<1>(in),
        &std::get<2>(in),
        &std::get<3>(in),
        &std::get<4>(in),
        &std::get<5>(in),
        &std::get<6>(in),
        &std::get<7>(in),
        &std::get<8>(in),
        &std::get<9>(in)
    };
}




class Lamps : TickSubscriber {
public:
    static Lamps& make(uint16_t& bad_lamps, uint8_t& lamps_qty)
    {
        static auto result = Lamps{
              from_tuple()
            , bad_lamps
            , lamps_qty
        };
        return result;
    }


private:
    uint16_t& bad_lamps;
    const uint8_t& lamps_qty;

    Lamps(
          uint16_t& bad_lamps
        , uint8_t& lamps_qty
    ) : bad_lamps {bad_lamps}, lamps_qty{lamps_qty}
    {
        tick_subscribe();
    }

    void notify() override {
        // заполняем только первые 10
        uint16_t tmp{0};
        // TODO сюда бы пинлист из 2-ой библиотеки
        for (auto i{0}; i < lamps_qty; i++) {
            tmp |= *[i] << i;
        }
        bad_lamps = tmp;
    }
};