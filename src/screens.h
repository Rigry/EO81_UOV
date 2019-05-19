#pragma once

#include "screen_common.h"
#include <array>

constexpr auto models = std::array{
    "УОВ-ПВ-1   ",
    "УОВ-ПВ-5   ",
    "УОВ-ПВ-10  ",
    "УОВ-ПВ-15  ",
    "УОВ-ПВ-30  ",
    "УОВ-ПВ-50  ",
    "УОВ-ПВ-100 ",
    "УОВ-ПВ-150 ",
    "УОВ-ПВ-200 ",
    "УОВ-ПВ-300 ",
    "УОВ-ПВ-500 ",
    "УОВ-ПВ-700 ",
    "УОВ-ПВ-1000",
    "УОВ-ПВ-1500",
    "УОВ-СВ-5   ",
    "УОВ-СВ-10  ",
    "УОВ-СВ-15  ",
    "УОВ-СВ-30  ",
    "УОВ-СВ-50  ",
    "УОВ-СВ-100 ",
    "УОВ-СВ-150 ",
    "УОВ-СВ-200 ",
    "УОВ-СВ-300 ",
    "УОВ-СВ-500 ",
    "УОВ-СВ-700 ",
    "УОВ-СВ-1000",
    "УОВ-СВ-1500" 
};

constexpr std::string_view model_to_string(int i) {
    return models[i];
}


struct Main_screen : Screen {
    String_buffer& lcd;
    Eventer enter_event;
    Callback<> out_callback;
    int& model_n;

    Main_screen(
          String_buffer& lcd
        , Enter_event enter_event
        , Out_callback out_callback
        , int& model_n
    ) : lcd          {lcd}
      , enter_event  {enter_event.value}
      , out_callback {out_callback.value}
      , model_n      {model_n}
    {}
    void init() override {
        enter_event ([this]{ out_callback(); });
        lcd.clear();
        lcd.line(0).center() << models[model_n];
        lcd << "счет: " << next_line;
    }
    void deinit() override {
        enter_event (null_function);
    }

    int placeholder {0};
    void draw() override {
        lcd.line(2) << placeholder++;
    }
};