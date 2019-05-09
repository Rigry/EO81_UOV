#define STM32F103xB
#define F_OSC   8000000UL
#define F_CPU   72000000UL

#include "stm32f103xb.h"
#include "init_clock.h"
#include "main.h"
#include "timers.h"
#include "literals.h"
#include "string_buffer.h"
#include "hd44780.h"
#include "select_screen.h"
#include "button.h"


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

struct Main_screen : Screen {
    String_buffer& lcd;
    Function<void(Callback<>)> enter_publisher;
    Callback<> out_callback; 

    Main_screen(String_buffer& lcd, Enter_publisher enter_publisher, Out_callback out_callback)
        : lcd             {lcd}
        , enter_publisher {enter_publisher.value}
        , out_callback    {out_callback.value}
    {}
    void init() override {
        enter_publisher ([this]{ out_callback(); });
        lcd.clear();
        lcd.line(0).center() << "Главный экран";
        lcd << "счет: " << next_line;
    }
    void deinit() override {
        enter_publisher ([]{});
    }

    int placeholder {0};
    void draw() override {
        lcd.line(2) << placeholder++;
    }

};


struct Menu : TickSubscriber {
    String_buffer lcd {};
    HD44780& _ { HD44780::make<RS, RW, E, DB4, DB5, DB6, DB7>(lcd.get_buffer()) };
    Button_event& up;
    Button_event& down;
    Button_event& enter;
    size_t tick_count{0};

    Menu (Button_event& up, Button_event& down, Button_event& enter)
        : up{up}, down{down}, enter{enter} 
    {
        tick_subscribe();
        current_screen->init();
    }
    
    Main_screen main_screen {
          lcd
        , Enter_publisher { [this](auto c){enter.set_click_callback(c);}     }
        , Out_callback    { [this]{ change_screen(main_select); }}
    };
    Select_screen<5> main_select {
          lcd
        , Up_publisher    { [this](auto c){   up.set_click_callback(c);}     }
        , Down_publisher  { [this](auto c){ down.set_click_callback(c);}     }
        , Enter_publisher { [this](auto c){enter.set_click_callback(c);}     }
        , Out_publisher   { [this](auto c){enter.set_long_push_callback(c);} }
        , Out_callback       { [this]{ change_screen(main_screen);  }}
        , Line {"Аварии"      ,[this]{ change_screen(alarm_select); }}
        , Line {"Наработка"   ,[]{}}
        , Line {"Конфигурация",[]{}}
        , Line {"Лог работы"  ,[]{}}
        , Line {"Настройки"   ,[]{}}
    };
    Select_screen<3> alarm_select {
          lcd
        , Up_publisher    { [this](auto c){   up.set_click_callback(c);}     }
        , Down_publisher  { [this](auto c){ down.set_click_callback(c);}     }
        , Enter_publisher { [this](auto c){enter.set_click_callback(c);}     }
        , Out_publisher   { [this](auto c){enter.set_long_push_callback(c);} }
        , Out_callback    { [this]{ change_screen(main_select);  }}
        , Line {"Нерабочие лампы"    ,[]{}}
        , Line {"Ошибки линии RS485" ,[]{}}
        , Line {"Сбросить аварии"    ,[]{}}
    };
    Screen* current_screen {&main_screen};

    void notify() override {
        every_qty_cnt_call(tick_count, 50, [this]{
            current_screen->draw();
        });
    }

    void change_screen(Screen& news) {
        current_screen->deinit();
        current_screen = &news;
        current_screen->init();
    }
};




int main()
{
    auto up    = Button<Up>();
    auto down  = Button<Down>();
    auto enter = Buttons(up, down);
    [[maybe_unused]] auto _ = Menu(up, down, enter);

    while (1) {
        __WFI();
    }

}
