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
    template<class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10>
    static Lamps& make(std::bitset<glob::max_lamps>& lamps, uint8_t& lamps_qty)
    {
        auto pins = make_pins<
            mcu::PinMode::Input,
            P1,P2,P3,P4,P5,P6,P7,P8,P9,P10
        >();
        static auto result = Lamps{
              from_tuple(pins)
            , lamps
            , lamps_qty
        };
        return result;
    }


private:
    std::array<Pin*, 10> pins;
    std::bitset<glob::max_lamps>& lamps;
    const uint8_t& lamps_qty;

    Lamps(
          std::array<Pin*, 10> pins
        , std::bitset<glob::max_lamps>& lamps
        , uint8_t& lamps_qty
    ) : pins {pins}, lamps {lamps}, lamps_qty{lamps_qty}
    {
        tick_subscribe();
    }

    void notify() override {
        // заполняем только первые 10
        // с конца в бит сете, чтобы первые 16 были в первом регистре
        for (auto i{0}; i < lamps_qty; i++) {
            lamps[lamps.size() - i] = *pins[i];
        }
    }
};