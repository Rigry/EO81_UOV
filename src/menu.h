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
        , modbus.qty_uv_lamps
    };

    Select_screen<4> main_select {
          lcd, buttons_events
        , Out_callback       { [this]{ change_screen(main_screen);  }}
        , Line {"Аварии"      ,[this]{ change_screen(alarm_select); }}
        , Line {"Наработка"   ,[]{}}
        , Line {"Конфигурация",[this]{ change_screen(config_select); }}
        , Line {"Лог работы"  ,[]{}}
    };

    Select_screen<3> alarm_select {
          lcd, buttons_events
        , Out_callback    { [this]{ change_screen(main_select);  }}
        , Line {"Нерабочие лампы"    ,[]{}}
        , Line {"Ошибки линии RS485" ,[]{}}
        , Line {"Сбросить аварии"    ,[]{}}
    };

    Select_screen<4> config_select {
          lcd, buttons_events
        , Out_callback             { [this]{ change_screen(main_select);  }}
        , Line {"Просмотр конф.-ии" ,[]{}}
        , Line {"Тревога УФ"        ,[this]{ change_screen(min_UV_set);   }}
        , Line {"Настройка сети"    ,[this]{ change_screen(rs485_select); }}
        , Line {"Тех. настройки"    ,[this]{ change_screen(tech_select);  }}
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
        , Out_callback    { [this]{ change_screen(config_select);  }}
        , "уровень УФ"
        , flash.uv_level_min
        , Min<int>{1}, Max<int>{100}
    };

    Set_screen<uint8_t> address_set {
          lcd, buttons_events
        , Out_callback    { [this]{ change_screen(rs485_select);  }}
        , "Адрес modbus"
        , flash.modbus_address
        , Min<uint8_t>{1}, Max<uint8_t>{255}
    };

    // приходится сохранять, так как нельзя сделать ссылку на член битового поля
    uint8_t boudrate_ {flash.uart_set.baudrate};
    Set_screen<uint8_t, boudrate_to_string> boudrate_set {
          lcd, buttons_events
        , Out_callback    { [this]{ 
            flash.uart_set.baudrate = USART::Baudrate(boudrate_);
            change_screen(rs485_select);
        }}
        , "Скорость в бодах"
        , boudrate_
        , Min<uint8_t>{0}, Max<uint8_t>{::boudrate.size() - 1}
    };

    uint8_t parity_enable_ {flash.uart_set.parity_enable};
    Set_screen<uint8_t, exist_to_string> parity_en_set {
          lcd, buttons_events
        , Out_callback    { [this]{ 
            flash.uart_set.parity_enable = bool(parity_enable_);
            change_screen(rs485_select);
        }}
        , "Проверка на чет/нечет"
        , parity_enable_
        , Min<uint8_t>{0}, Max<uint8_t>{exist.size() - 1}
    };

    uint8_t parity_ {flash.uart_set.parity};
    Set_screen<uint8_t, parity_to_string> parity_set {
          lcd, buttons_events
        , Out_callback    { [this]{ 
            flash.uart_set.parity = USART::Parity(parity_);
            change_screen(rs485_select);
        }}
        , "Проверка на"
        , parity_
        , Min<uint8_t>{0}, Max<uint8_t>{parity.size() - 1}
    };

    int stop_bits {flash.uart_set.stop_bits == USART::StopBits::_1 ? 1 : 2};
    Set_screen<int> stop_bits_set {
          lcd, buttons_events
        , Out_callback    { [this]{ 
            flash.uart_set.stop_bits = 
                stop_bits == 1 
                ? USART::StopBits::_1 
                : USART::StopBits::_2
            ;
            change_screen(rs485_select);
        }}
        , "Количество стоп бит"
        , stop_bits
        , Min<int>{1}, Max<int>{2}
    };

    Set_screen<int, model_to_string> name_set {
          lcd, buttons_events
        , Out_callback    { [this]{ change_screen(tech_select);  }}
        , "Наименование уст."
        , flash.model_number
        , Min<int>{0}, Max<int>{models.size() - 1}
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