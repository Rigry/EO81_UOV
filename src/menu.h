#pragma once

#include "string_buffer.h"
#include "hd44780.h"
#include "select_screen.h"
#include "set_screen.h"
#include "screens.h"
#include "button.h"

#include "ring_buffer.h"
RingBuffer<16> tmp;



template<class Pins, class Flash_data, class Modbus_regs>
struct Menu : TickSubscriber {
    String_buffer lcd {};
    HD44780& hd44780 { HD44780::make(Pins{}, lcd.get_buffer()) };
    Button_event& up;
    Button_event& down;
    Button_event& enter;
    Flash_data&   flash;
    Modbus_regs&  modbus;

    Screen* current_screen {&main_screen};
    size_t tick_count{0};

    Buttons_events buttons_events {
          Up_event    { [this](auto c){   up.set_click_callback(c);}     }
        , Down_event  { [this](auto c){ down.set_click_callback(c);}     }
        , Enter_event { [this](auto c){enter.set_click_callback(c);}     }
        , Out_event   { [this](auto c){enter.set_long_push_callback(c);} }
    };


    Menu (
        Pins pins
        , Button_event& up
        , Button_event& down
        , Button_event& enter
        , Flash_data&   flash
        , Modbus_regs&  modbus
    ) : up{up}, down{down}, enter{enter}, flash{flash}, modbus{modbus}
    {
        tick_subscribe();
        current_screen->init();
    }
    
    Main_screen main_screen {
          lcd
        , Enter_event  { [this](auto c){enter.set_click_callback(c);}     }
        , Out_callback { [this]{ change_screen(main_select); }}
        , flash.model_number
        , modbus.temperature
        , modbus.uv_level
        , modbus.quantity.lamps
    };

    Select_screen<4> main_select {
          lcd, buttons_events
        , Out_callback       { [this]{ change_screen(main_screen);  }}
        , Line {"Аварии"      ,[this]{ change_screen(alarm_select); }}
        , Line {"Наработка"   ,[this]{ change_screen(work_select); }}
        , Line {"Конфигурация",[this]{ change_screen(config_select); }}
        , Line {"Лог работы"  ,[]{}}
    };

    Select_screen<3> alarm_select {
          lcd, buttons_events
        , Out_callback    { [this]{ change_screen(main_select);  }}
        , Line {"Нерабочие лампы"    ,[this]{ change_screen(bad_lamps_screen);}}
        , Line {"Ошибки линии RS485" ,[]{}}
        , Line {"Сбросить аварии"    ,[]{}}
    };

    Bad_lamps_screen bad_lamps_screen {
          lcd, Out_event{buttons_events.out}
        , Out_callback       { [this]{ change_screen(alarm_select);  }}
        , modbus.lamp, modbus.quantity.lamps
    };

    Select_screen<3> work_select {
          lcd, buttons_events
        , Out_callback             { [this]{ change_screen(main_select);  }}
        , Line {"Просмотр наработки",[this]{ change_screen(work_time_screen);}}
        , Line {"Сброс всех ламп   ",[this]{
            for(auto& hour : modbus.hours) // TODO ещё во flash
                hour = 0;
            change_screen(work_time_screen); // чтоб увидеть действие
        }}
        , Line {"Сброс одной лампы",[this]{ 
            reset_n_set.max = modbus.quantity.lamps;
            change_screen(reset_n_set);
        }}

    };

    Work_time_screen work_time_screen {
          lcd, buttons_events
        , Out_callback       { [this]{ change_screen(work_select);  }}
        , modbus.hours, modbus.quantity.lamps
    };

    int reset_n {1};
    Set_screen<int> reset_n_set {
          lcd, buttons_events
        , "N лампы для сброса"
        , reset_n
        , Min<int>{1}, Max<int>{1}
        , Out_callback      { [this]{ change_screen(work_select);  }}
        , Enter_callback    { [this]{
            modbus.hours[reset_n-1] = 0; // TODO во флэше тоже
            work_time_screen.set_first_lamp(reset_n-1);
            change_screen(work_time_screen);
        }}
    };

    Select_screen<4> config_select {
          lcd, buttons_events
        , Out_callback             { [this]{ change_screen(main_select);  }}
        , Line {"Просмотр конф.-ии" ,[this]{ change_screen(config_screen);}}
        , Line {"Тревога УФ"        ,[this]{ change_screen(min_UV_set);   }}
        , Line {"Настройка сети"    ,[this]{ change_screen(rs485_select); }}
        , Line {"Тех. настройки"    ,[this]{ change_screen(tech_select);  }}
    };

    Config_screen config_screen {
          lcd, buttons_events
        , Out_callback       { [this]{ change_screen(config_select);  }}
        , modbus.quantity, flash.exist
    };

    Select_screen<5> rs485_select {
          lcd, buttons_events
        , Out_callback       { [this]{ change_screen(config_select);  }}
        , Line {"Адрес"       ,[this]{ change_screen(address_set);    }}
        , Line {"Скорость"    ,[this]{ change_screen(boudrate_set);   }}
        , Line {"Проверка"    ,[this]{ change_screen(parity_en_set);  }}
        , Line {"Тип проверки",[this]{ change_screen(parity_set);     }}
        , Line {"Стоп биты"   ,[this]{ change_screen(stop_bits_set);  }}
    };

    Select_screen<4> tech_select {
          lcd, buttons_events
        , Out_callback        { [this]{ change_screen(config_select);  }}
        , Line {"Наименование" ,[this]{ change_screen(name_set);   }}
        , Line {"туду"   ,[]{}}
        , Line {"туду"   ,[]{}}
        , Line {"туду"   ,[]{}}
    };

    Set_screen<int> min_UV_set {
          lcd, buttons_events
        , "уровень УФ"
        , flash.uv_level_min
        , Min<int>{1}, Max<int>{100}
        , Out_callback    { [this]{ change_screen(config_select);  }}
        , Enter_callback  {nullptr}
    };

    Set_screen<uint8_t> address_set {
          lcd, buttons_events
        , "Адрес modbus"
        , flash.modbus_address
        , Min<uint8_t>{1}, Max<uint8_t>{255}
        , Out_callback    { [this]{ change_screen(rs485_select);  }}
        , Enter_callback  {nullptr}
    };

    // приходится сохранять, так как нельзя сделать ссылку на член битового поля
    uint8_t boudrate_ {flash.uart_set.baudrate};
    Set_screen<uint8_t, boudrate_to_string> boudrate_set {
          lcd, buttons_events
        , "Скорость в бодах"
        , boudrate_
        , Min<uint8_t>{0}, Max<uint8_t>{::boudrate.size() - 1}
        , Out_callback    { [this]{ 
            flash.uart_set.baudrate = USART::Baudrate(boudrate_);
            change_screen(rs485_select);
        }} // FIX its enter callback
        , Enter_callback  {nullptr}
    };

    uint8_t parity_enable_ {flash.uart_set.parity_enable};
    Set_screen<uint8_t, exist_to_string> parity_en_set {
          lcd, buttons_events
        , "Проверка на чет/нечет"
        , parity_enable_
        , Min<uint8_t>{0}, Max<uint8_t>{exist.size() - 1}
        , Out_callback    { [this]{ 
            flash.uart_set.parity_enable = bool(parity_enable_);
            change_screen(rs485_select);
        }} // FIX its enter callback
        , Enter_callback  {nullptr}
    };

    uint8_t parity_ {flash.uart_set.parity};
    Set_screen<uint8_t, parity_to_string> parity_set {
          lcd, buttons_events
        , "Проверка на"
        , parity_
        , Min<uint8_t>{0}, Max<uint8_t>{parity.size() - 1}
        , Out_callback    { [this]{ 
            flash.uart_set.parity = USART::Parity(parity_);
            change_screen(rs485_select);
        }} // FIX its enter callback
        , Enter_callback  {nullptr}
    };

    int stop_bits {flash.uart_set.stop_bits == USART::StopBits::_1 ? 1 : 2};
    Set_screen<int> stop_bits_set {
          lcd, buttons_events
        , "Количество стоп бит"
        , stop_bits
        , Min<int>{1}, Max<int>{2}
        , Out_callback    { [this]{ 
            flash.uart_set.stop_bits = 
                stop_bits == 1 
                ? USART::StopBits::_1 
                : USART::StopBits::_2
            ;
            change_screen(rs485_select);
        }} // FIX its enter callback
        , Enter_callback  {nullptr}
    };

    Set_screen<int, model_to_string> name_set {
          lcd, buttons_events
        , "Наименование уст."
        , flash.model_number
        , Min<int>{0}, Max<int>{models.size() - 1}
        , Out_callback    { [this]{ change_screen(tech_select);  }}
        , Enter_callback  {nullptr}
    };
    

    void notify() override {
        every_qty_cnt_call(tick_count, 50, [this]{
            current_screen->draw();
        });
    }

    void change_screen(Screen& new_screen) {
        current_screen->deinit();
        current_screen = &new_screen;
        current_screen->init();
    }
};