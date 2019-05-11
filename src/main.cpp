#define STM32F103xB
#define F_OSC   8000000UL
#define F_CPU   72000000UL

#define HEAP_SIZE 1024

#include "init_clock.h"
#include "main.h"
#include "timers.h"
#include "literals.h"
#include "string_buffer.h"
#include "hd44780.h"
#include "select_screen.h"
#include "set_screen.h"
#include "screens.h"
#include "button.h"

#include "ring_buffer.h"
RingBuffer<16> tmp;



/// эта функция вызывается первой в startup файле
extern "C" void init_clock() { init_clock<F_OSC, F_CPU>(); }

using E     = mcu::PB3;       
using RW    = mcu::PD2;       
using RS    = mcu::PC12;      
using DB4   = mcu::PB4;       
using DB5   = mcu::PB5;
using DB6   = mcu::PB6;    
using DB7   = mcu::PB7;
using Up    = mcu::PB8;
using Down  = mcu::PB9;
using US    = mcu::PB0; 

struct Flash_data {
    uint16_t factory_number = 0;
    UART::Settings uart_set = {
        .parity_enable  = false,
        .parity         = USART::Parity::even,
        .data_bits      = USART::DataBits::_8,
        .stop_bits      = USART::StopBits::_1,
        .baudrate       = USART::Baudrate::BR9600,
        .res            = 0
    };
    uint8_t  modbus_address        = 1;
    uint16_t max_temperature       = 55;
    int uv_level_min               = 40;  
    uint16_t qty_uv_lamps          = 5;
    uint16_t uv_level_highest      = 0x0100;
    int      model_number          = 0;
    uint16_t temperature_recovery  = 20; 
} flash;




struct Menu : TickSubscriber {
    String_buffer lcd {};
    HD44780& _ { HD44780::make<RS, RW, E, DB4, DB5, DB6, DB7>(lcd.get_buffer()) };
    Button_event& up;
    Button_event& down;
    Button_event& enter;
    Flash_data&   flash;
    Screen* current_screen {&main_screen};
    size_t tick_count{0};

    Menu (Button_event& up, Button_event& down, Button_event& enter,  Flash_data& flash)
        : up{up}, down{down}, enter{enter}, flash{flash}
    {
        tick_subscribe();
        current_screen->init();
    }
    
    Main_screen main_screen {
          lcd
        , Enter_event  { [this](auto c){enter.set_click_callback(c);}     }
        , Out_callback { [this]{ change_screen(main_select); }}
        , flash.model_number
    };
    Select_screen<4> main_select {
          lcd
        , Up_event    { [this](auto c){   up.set_click_callback(c);}     }
        , Down_event  { [this](auto c){ down.set_click_callback(c);}     }
        , Enter_event { [this](auto c){enter.set_click_callback(c);}     }
        , Out_event   { [this](auto c){enter.set_long_push_callback(c);} }
        , Out_callback       { [this]{ change_screen(main_screen);  }}
        , Line {"Аварии"      ,[this]{ change_screen(alarm_select); }}
        , Line {"Наработка"   ,[]{}}
        , Line {"Конфигурация",[this]{ change_screen(config_select); }}
        , Line {"Лог работы"  ,[]{}}
    };
    Select_screen<3> alarm_select {
          lcd
        , Up_event    { [this](auto c){   up.set_click_callback(c);}     }
        , Down_event  { [this](auto c){ down.set_click_callback(c);}     }
        , Enter_event { [this](auto c){enter.set_click_callback(c);}     }
        , Out_event   { [this](auto c){enter.set_long_push_callback(c);} }
        , Out_callback    { [this]{ change_screen(main_select);  }}
        , Line {"Нерабочие лампы"    ,[]{}}
        , Line {"Ошибки линии RS485" ,[]{}}
        , Line {"Сбросить аварии"    ,[]{}}
    };
    Select_screen<4> config_select {
          lcd
        , Up_event    { [this](auto c){   up.set_click_callback(c);}     }
        , Down_event  { [this](auto c){ down.set_click_callback(c);}     }
        , Enter_event { [this](auto c){enter.set_click_callback(c);}     }
        , Out_event   { [this](auto c){enter.set_long_push_callback(c);} }
        , Out_callback    { [this]{ change_screen(main_select);  }}
        , Line {"Просмотр конф.-ии" ,[]{}}
        , Line {"Тревога УФ"        ,[this]{ change_screen(min_UV_set);   }}
        , Line {"Настройка сети"    ,[this]{ change_screen(rs485_select); }}
        , Line {"Тех. настройки"    ,[]{}}
    };
    Select_screen<4> rs485_select {
          lcd
        , Up_event    { [this](auto c){   up.set_click_callback(c);}     }
        , Down_event  { [this](auto c){ down.set_click_callback(c);}     }
        , Enter_event { [this](auto c){enter.set_click_callback(c);}     }
        , Out_event   { [this](auto c){enter.set_long_push_callback(c);} }
        , Out_callback    { [this]{ change_screen(config_select);  }}
        , Line {"Адрес:     255"    ,[]{}}
        , Line {"Скорость:  115200"     ,[]{}}
        , Line {"Проверка:  отсутсв."   ,[]{}}
        , Line {"Стоп биты: 1"         ,[]{}}
    };
    Set_screen<int> min_UV_set {
          lcd
        , Up_event    { [this](auto c){   up.set_click_callback(c);}     }
        , Down_event  { [this](auto c){ down.set_click_callback(c);}     }
        , Enter_event { [this](auto c){enter.set_click_callback(c);}     }
        , Out_event   { [this](auto c){enter.set_long_push_callback(c);} }
        , Out_callback    { [this]{ change_screen(config_select);  }}
        , "уровень УФ"
        , flash.uv_level_min
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
        auto i = new int;
        tmp.push(uint32_t(i));
        delete i;
    }
};


int main()
{
    auto up    = Button<Up>();
    auto down  = Button<Down>();
    auto enter = Tied_buttons(up, down);
    [[maybe_unused]] auto _ = Menu(up, down, enter, flash);

    while (1) {
        __WFI();
    }

}
