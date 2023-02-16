#pragma once

#include "screen_common.h"
#include <array>
#include <bitset>

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

constexpr auto reassign = std::array {
    "не переназначать",
    "переназначить"
};

constexpr std::string_view reassign_to_string(int i) {
    return reassign[i];
}

constexpr auto confirm = std::array {
    "нет",
    "да"
};

constexpr std::string_view confirm_to_string(int i) {
    return confirm[i];
}

constexpr auto boudrate = std::array {
    "9600",
    "14400",
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

constexpr auto mode = std::array {
    "Ручной",
    "Авто"
};

constexpr std::string_view mode_to_string(int i) {
    return mode[i];
}



struct Main_screen : Screen {
    String_buffer& lcd;
    Eventer enter_event;
    Callback<> out_callback;
    uint8_t&  model_n;
    uint16_t& temperatura;
    uint16_t& uv_level;
    uint8_t&  lamps_qty;
    Flags&    flags;
    Exsist&   exist;
    int _ {0}; // без этой фигни оптимизатор чудил

    Main_screen(
          String_buffer& lcd
        , Enter_event enter_event
        , Out_callback out_callback
        , uint8_t& model_n
        , uint16_t& temperatura
        , uint16_t& uv_level
        , uint8_t&  lamps_qty
        , Flags&    flags
        , Exsist&   exist
    ) : lcd          {lcd}
      , enter_event  {enter_event.value}
      , out_callback {out_callback.value}
      , model_n      {model_n}
      , temperatura  {temperatura}
      , uv_level     {uv_level}
      , lamps_qty    {lamps_qty}
      , flags        {flags}
      , exist        {exist}
    {}

    void init() override {
        enter_event ([this]{ out_callback(); });
        lcd.clear();
        lcd.line(0).center() << models[model_n];

        lcd.line(1) << "Ламп: " << lamps_qty;
        lcd.line(2) << "Авария: " << next_line;
    }

    void deinit() override {
        enter_event (nullptr);
    }

    void draw() override {
        lcd.line(1).cursor(6) << lamps_qty << next_line;
        if (exist.temp_sensor) {
            lcd.line(3) << "t ";
            lcd.width(2) << temperatura << "C  ";
        }
        
        lcd.line(3).cursor(7)  << (flags.uv_on ? "УФ" : "  ");
        if (exist.uv_sensor) {
            if (flags.uv_on)
                lcd.cursor(10).width(3) << uv_level << "%";
            else
                lcd.cursor(10) << "    ";
        }

        lcd.cursor(16) << (flags.us_on ? "УЗ" : "  ");

        lcd.cursor(19) << (flags.distance ? "Д" : " ");

        lcd.line(2).cursor(8);
        if (not flags.is_alarm() ) {
            lcd << "нет" << next_line;
            return;
        }
        if (flags.overheat) {
            lcd << "ПЕРЕГРЕВ" << next_line;
            return;
        }
        if (flags.bad_lamps) {
            lcd << "ЛАМПЫ" << next_line;
            return;
        }
        if (flags.uv_low_level) {
            lcd << "УРОВЕНЬ" << next_line;
            return;
        }
    }
};

struct Bad_lamps_screen : Screen {
    String_buffer& lcd;
    Buttons_events eventers;
    Callback<> out_callback;
    const std::array<uint16_t, glob::max_extantions+1>& bad_lamps;
    const uint8_t& qty_uov_lamps;
    const uint8_t& qty_ext_lamps_1;
    const uint8_t& qty_ext_lamps_2;
    const Flags& flags;

    Bad_lamps_screen (
          String_buffer& lcd
        , Buttons_events eventers
        , Out_callback out_callback
        , std::array<uint16_t, glob::max_extantions+1>& bad_lamps
        , uint8_t& qty_uov_lamps
        , uint8_t& qty_ext_lamps_1
        , uint8_t& qty_ext_lamps_2
        , Flags& flags
    ) : lcd             {lcd}
      , eventers        {eventers}
      , out_callback    {out_callback.value}
      , bad_lamps       {bad_lamps}
      , qty_uov_lamps   {qty_uov_lamps}
      , qty_ext_lamps_1 {qty_ext_lamps_1}
      , qty_ext_lamps_2 {qty_ext_lamps_2}
      , flags           {flags}
    {}

    void init() override {
        eventers.up    ([this]{ up();    });
        eventers.down  ([this]{ down();  });
        eventers.out   ([this]{ out_callback(); });
        redraw();
    }
    void deinit() override {
        eventers.up    (nullptr);
        eventers.down  (nullptr);
        eventers.out   (nullptr);
    }

    void draw() override {redraw();}

private:

    int first_lamp {0};
    std::array<uint16_t, glob::max_lamps> lamp_is_bad{0};


    void down() {
        if (first_lamp + 20 >= qty_uov_lamps + qty_ext_lamps_1 + qty_ext_lamps_2)
            return;
        first_lamp += 5;

        redraw();
    }

    void up() {
        if (first_lamp == 0) {
            return;
        }
        first_lamp -= 5;

        redraw();
    }

    void redraw() {

        lcd.line(0);

        if (not flags.uv_on) {
            lcd.center() << "Не включено";
            lcd << clear_after;
            return;
        }

        if (bad_lamps[0] == 0 and bad_lamps[1] == 0 and bad_lamps[2] == 0) {
            lcd.center() << "Отсуствуют";
            lcd << clear_after;
            return;
        }

        uint16_t constexpr max_on_screen {20};
        auto bad_qty {0};

            for (auto i {0}; i < qty_uov_lamps; i++) {
              if ((bad_lamps[0] >> i) & 0b1) {
                   lamp_is_bad[bad_qty++] = i + 1;
                }
            }

            for (auto i {0}; i < qty_ext_lamps_1; i++) {
              if ((bad_lamps[1] >> i) & 0b1) {
                   lamp_is_bad[bad_qty++] = i + 11;
                }
            }

            for (auto i {0}; i < qty_ext_lamps_2; i++) {
              if ((bad_lamps[2] >> i) & 0b1) {
                   lamp_is_bad[bad_qty++] = i + 21;
                }
            }

            for (auto j {0}; j < (bad_qty - first_lamp); j++) {

                if (j == max_on_screen) {
                    break;
                }

                lcd.width(4) << lamp_is_bad[first_lamp + j];
            }
        if ((bad_qty - first_lamp) >= max_on_screen) {
            return;
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
        eventers.up    (nullptr);
        eventers.down  (nullptr);
        eventers.out   (nullptr);
    }

    void draw() override {redraw();}

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
        eventers.up    (nullptr);
        eventers.down  (nullptr);
        eventers.out   (nullptr);
    }

    void draw() override {}



private:
    int from_line {0};

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
            lcd << "Плата датчиков: " << (exist.temp_sensor or exist.uv_sensor ? '+' : '-') << next_line;
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
        if (line_cnt == 4)
            return;
        if (from_line <= 5) {
            lcd << "Vers.:" << format(VERSION) << next_line;
            line_cnt++;
        }
        if (line_cnt == 4)
            return;
        if (from_line <= 5) {
            lcd << "Libr.:" << format(MCULIB_VERSION) << next_line;
            line_cnt++;
        }
    }

    // TODO вырезать количество коммитов (проблема с constexpr)
    // вот неверная попытка https://wandbox.org/permlink/v5cRmBtse0h7ksxr
    // возвращение временного массива
    // возможное решение 
    // https://github.com/boostcon/cppnow_presentations_2012/blob/master/wed/schurr_cpp11_tools_for_class_authors.pdf?raw=true
    constexpr std::string_view format(std::string_view in) {
        auto begin = in[0] == 'v' ? 1 : 0;
        size_t max_length = 12;
        auto end = std::min(max_length, in.length()) + begin;
        return in.substr(begin, end);
    }

    void up(){
        if (from_line > 0)
            from_line--;
        redraw();
    }
    void down(){
        if (from_line < 3)
            from_line++;
        redraw();
    }
};

using Password_callback = Construct_wrapper<Callback<int>>;

struct Password_screen : Screen {
    String_buffer& lcd;
    Buttons_events eventers;
    Callback<>     out_callback;
    Callback<int>  password_callback;

    Password_screen (
          String_buffer&    lcd
        , Buttons_events    eventers
        , Out_callback      out_callback
        , Password_callback password_callback
    ) : lcd               {lcd}
      , eventers          {eventers}
      , out_callback      {out_callback.value}
      , password_callback {password_callback.value}
    {}

    void init() override {
        eventers.up    ([this]{ up();    });
        eventers.down  ([this]{ down();  });
        eventers.out   ([this]{ out_callback(); });
        eventers.enter ([this]{ enter();  });
        lcd.line(0).center() << "Введите пароль" << clear_after;
        std::fill(std::begin(digits), std::end(digits), 0);
        index = 3;
        redraw();
    }
    void deinit() override {
        eventers.up    (nullptr);
        eventers.down  (nullptr);
        eventers.out   (nullptr);
        eventers.enter (nullptr);
    }

    void draw() override {}


private:
    std::array<int,4> digits {0};
    int index {3};

    void down() {
        digits[index]--;
        if (digits[index] < 0) {
            digits[index] = 9;
        }
        redraw();
    }
    
    void up() {
        digits[index]++;
        digits[index] %= 10;
        redraw();
    }

    void enter() {
        if (--index < 0) {
            int password {0};
            for (auto n : digits)
                (password *= 10) += n;
            password_callback(password);
            return;
        }
        redraw();
    }

    void redraw() {
        lcd.line(2).cursor(8);
        for (auto n : digits)
            lcd << n;
        lcd.line(3).cursor(8+index) << char(0x5E) << next_line;
    }
};

struct Log_screen : Screen {
    String_buffer& lcd;
    Eventer out_event;
    Callback<> out_callback;
    const Count& count;

    Log_screen (
          String_buffer& lcd
        , Out_event      out_event
        , Out_callback   out_callback
        , Count&         count
    ) : lcd          {lcd}
      , out_event    {out_event.value}
      , out_callback {out_callback.value}
      , count        {count}
    {}

    void init() override {
        out_event ([this]{ out_callback(); });
        lcd.line(0);
        lcd << "Включений:     ";
        lcd.width(5) << count.on;
        lcd << "Сбросов полных:";
        lcd.width(5) << count.reset_all;
        lcd << "Сбросов один.: ";
        lcd.width(5) << count.reset_one;
        lcd << "Сбросов логов: ";
        lcd.width(5) << count.reset_log;
    }
    void deinit() override {
        out_event (nullptr);
    }
    void draw() override {}
};