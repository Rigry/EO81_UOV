#pragma once

#include "screen_common.h"
#include <array>

constexpr auto models = std::array {
    "УОВ-ПВ-1",
    "УОВ-ПВ-5",
    "УОВ-ПВ-10",
    "УОВ-ПВ-15",
    "УОВ-ПВ-30",
    "УОВ-ПВ-50",
    "УОВ-ПВ-100",
    "УОВ-ПВ-150",
    "УОВ-ПВ-200",
    "УОВ-ПВ-300",
    "УОВ-ПВ-500",
    "УОВ-ПВ-700",
    "УОВ-ПВ-1000",
    "УОВ-ПВ-1500",
    "УОВ-СВ-5",
    "УОВ-СВ-10",
    "УОВ-СВ-15",
    "УОВ-СВ-30",
    "УОВ-СВ-50",
    "УОВ-СВ-100",
    "УОВ-СВ-150",
    "УОВ-СВ-200",
    "УОВ-СВ-300",
    "УОВ-СВ-500",
    "УОВ-СВ-700",
    "УОВ-СВ-1000",
    "УОВ-СВ-1500" 
};

constexpr std::string_view model_to_string(int i) {
    return models[i];
}

constexpr auto exist = std::array {
    "отсутствует",
    "присутствует"
};

constexpr std::string_view exist_to_string(int i) {
    return exist[i];
}

constexpr auto boudrate = std::array {
    "9600",
    "14400"
    "19200",
    "28800",
    "38400",
    "57600",
    "76800",
    "115200"
};

constexpr std::string_view boudrate_to_string(int i) {
    return boudrate[i];
}

constexpr auto parity = std::array {
    "нечетность",
    "четность  "  
};

constexpr std::string_view parity_to_string(int i) {
    return parity[i];
}



struct Main_screen : Screen {
    String_buffer& lcd;
    Eventer enter_event;
    Callback<> out_callback;
    int& model_n;
    uint16_t& temperatura;
    uint16_t& uv_level;
    uint8_t&  lamps_qty;

    Main_screen(
          String_buffer& lcd
        , Enter_event enter_event
        , Out_callback out_callback
        , int& model_n
        , uint16_t& temperatura
        , uint16_t& uv_level
        , uint8_t&  lamps_qty
    ) : lcd          {lcd}
      , enter_event  {enter_event.value}
      , out_callback {out_callback.value}
      , model_n      {model_n}
      , temperatura  {temperatura}
      , uv_level     {uv_level}
      , lamps_qty    {lamps_qty}
    {}

    void init() override {
        enter_event ([this]{ out_callback(); });
        lcd.clear();
        lcd.line(0).center() << models[model_n];

        lcd.line(1) << "Ламп:" << lamps_qty;
        lcd.line(2) << "Авария:никаких" << next_line;
        lcd         << "t"; 
        lcd.width(3) << temperatura << "C";
        lcd.line(3).cursor(8) << "УФ";
        lcd.line(3).cursor(14)<< "УЗ";
    }
    
    void deinit() override {
        enter_event (null_function);
    }

    void draw() override {
        lcd.line(3).cursor(3).width(3) << temperatura;
    }
};

struct Bad_lamps_screen : Screen {
    String_buffer& lcd;
    Eventer out_event;
    Callback<> out_callback;
    const Bit_set<glob::max_lamps>& bad_lamps;
    const uint8_t& qty_lamps;

    Bad_lamps_screen (
          String_buffer& lcd
        , Out_event    out_event
        , Out_callback out_callback
        , Bit_set<glob::max_lamps>& bad_lamps
        , uint8_t& qty_lamps
    ) : lcd          {lcd}
      , out_event    {out_event.value}
      , out_callback {out_callback.value}
      , bad_lamps    {bad_lamps}
      , qty_lamps    {qty_lamps}
    {}

    void init() override {
        out_event ([this]{ out_callback(); });
    }
    void deinit() override {
        out_event (null_function);
    }
    void draw() override {
        lcd.line(0);
        uint16_t constexpr max_on_screen {20};
        auto bad_qty {0};
        for (auto i {0}; i < qty_lamps ; i++) {
            if (bad_lamps[i]) {
                lcd.width(4) << i;
                bad_qty++;
            }
            if (bad_qty == max_on_screen) {
                break;
            }
        }
        lcd << clear_after;
    }
};

struct Work_time_screen : Screen {
    String_buffer& lcd;
    Buttons_events eventers;
    Callback<> out_callback;
    const std::array<uint16_t, glob::max_lamps>& hours;
    const uint8_t& qty_lamps;

    Work_time_screen (
          String_buffer& lcd
        , Buttons_events eventers
        , Out_callback out_callback
        , std::array<uint16_t, glob::max_lamps>& hours
        , uint8_t& qty_lamps
    ) : lcd          {lcd}
      , eventers     {eventers}
      , out_callback {out_callback.value}
      , hours        {hours}
      , qty_lamps    {qty_lamps}
    {}

    void init() override {
        eventers.up    ([this]{ up();    });
        eventers.down  ([this]{ down();  });
        eventers.out   ([this]{ out_callback(); });
        redraw();
    }
    void deinit() override {
        eventers.up    (null_function);
        eventers.down  (null_function);
        eventers.out   (null_function);
    }

    void draw() override {}

    void set_first_lamp (int v) {
        first_lamp = v;
        first_lamp -= first_lamp % 2;
    }

private:
    int first_lamp {0};

    void down() {
        if (first_lamp + 8 >= qty_lamps)
            return;
        first_lamp += 2;
        redraw();
    }
    
    void up() {
        if (first_lamp == 0) {
            return;
        }
        first_lamp -= 2;
        redraw();
    }

    void redraw() {
        lcd.line(0);
        uint16_t constexpr max_on_screen {8};
        auto cnt {0};
        for (auto i {first_lamp}; i < qty_lamps ; i++) {
            lcd.width(3) << i+1 << ':';
            lcd.width(5) << hours[i] << ' ';
            if (++cnt == max_on_screen) {
                break;
            }
        }
        lcd << clear_after;
    }
};

struct Config_screen : Screen {
    String_buffer& lcd;
    Buttons_events eventers;
    Callback<> out_callback;
    const Quantity& quantity;
    const Exsist& exist;

    Config_screen (
          String_buffer& lcd
        , Buttons_events eventers
        , Out_callback out_callback
        , Quantity&    quantity
        , Exsist&      exist
    ) : lcd           {lcd}
      , eventers      {eventers}
      , out_callback  {out_callback.value}
      , quantity      {quantity}
      , exist         {exist}
    {}

    void init() override {
        eventers.up    ([this]{ up();    });
        eventers.down  ([this]{ down();  });
        eventers.out   ([this]{ out_callback(); });
        redraw();
    }
    void deinit() override {
        eventers.up    (null_function);
        eventers.down  (null_function);
        eventers.out   (null_function);
    }

    void draw() override {}

    void redraw() {
        lcd.line(0);
        auto line_cnt {0};
        if (from_line == 0) {
            lcd << "Всего ламп: ";
            lcd.width(3) << quantity.lamps << next_line;
            line_cnt++;
        }
        if (from_line <= 1) {
            lcd << "Плат расширения: ";
            lcd.width(2) << quantity.extantions << next_line;
            line_cnt++;
        }
        if (from_line <= 2) {
            lcd << "Плата датчиков: " << (exist.board_sensor ? '+' : '-') << next_line;
            line_cnt++;
        }
        if (from_line <= 3) {
            lcd << "Датчик темп.: " << (exist.temp_sensor ? '+' : '-') << next_line;
            line_cnt++;
        }
        if (line_cnt == 4)
            return;
        if (from_line <= 4) {
            lcd << "Датчик УФ: " << (exist.uv_sensor ? '+' : '-') << next_line;
            line_cnt++;
        }
        // TODO версия
    }

private:
    int from_line {0};

    void up(){
        if (from_line > 0)
            from_line--;
        redraw();
    }
    void down(){
        if (from_line < 1)
            from_line++;
        redraw();
    }
};