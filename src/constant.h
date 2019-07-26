#pragma once


// флаги наличия в системе
struct Exsist {
    bool board_sensor :1;
    bool temp_sensor  :1;
    bool uv_sensor    :1;
    uint8_t           :5;
};

struct Quantity {
    uint8_t lamps;
    uint8_t extantions;
};

struct Count {
    uint16_t on;
    uint16_t reset_all;
    uint16_t reset_one;
    uint16_t reset_log;
};

namespace glob {
    auto constexpr max_lamps       {112};
    auto constexpr max_extantions  {max_lamps/16};
    auto constexpr password        {208};
    auto constexpr max_temperature {99};
}