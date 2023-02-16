#pragma once


// флаги наличия в системе
struct Exsist {
    bool temp_sensor  :1;
    bool uv_sensor    :1;
    bool dry_contacts :1;
    uint8_t           :5;
};

struct Control {
    bool control_us : 1;
    bool control_uv : 1;
    bool distance   : 1;
    uint16_t        : 5;
    bool us_on      : 1;
    bool uv_on      : 1;
    bool alarm      : 1;
    uint16_t        : 5;
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

struct Flags {
    bool us_on        : 1;
    bool uv_on        : 1;
    bool uv_low_level : 1;
    bool overheat     : 1;
    bool us_started   : 1;
    bool uv_started   : 1;
    bool bad_lamps    : 1; 
    bool distance     : 1;
    bool mode         : 1;
    bool not_flow     : 1;
    uint16_t          : 6;
    bool is_alarm() { return bad_lamps or overheat or uv_low_level; }
};


namespace glob {
    auto constexpr max_lamps       {110};
    auto constexpr max_extantions  {10};
    auto constexpr password        {1207};
    auto constexpr max_temperature {99};
}