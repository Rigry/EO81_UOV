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

namespace glob {
    auto constexpr max_lamps       {112};
    auto constexpr max_extantions  {max_lamps/16};
    auto constexpr password        {208};
    auto constexpr max_temperature {99};
}