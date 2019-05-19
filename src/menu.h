#pragma once

#include "string_buffer.h"
#include "hd44780.h"
#include "select_screen.h"
#include "set_screen.h"
#include "screens.h"
#include "button.h"

#include "ring_buffer.h"
RingBuffer<16> tmp;



template<class Pins, class Flash_data>
struct Menu : TickSubscriber {
    String_buffer lcd {};
    HD44780& _ { HD44780::make(Pins{}, lcd.get_buffer()) };
    Button_event& up;
    Button_event& down;
    Button_event& enter;
    Flash_data&   flash;
    Screen* current_screen {&main_screen};
    size_t tick_count{0};

    Menu (Pins pins, Button_event& up, Button_event& down, Button_event& enter,  Flash_data& flash)
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
        , Out_callback             { [this]{ change_screen(main_select);  }}
        , Line {"Просмотр конф.-ии" ,[]{}}
        , Line {"Тревога УФ"        ,[this]{ change_screen(min_UV_set);   }}
        , Line {"Настройка сети"    ,[this]{ change_screen(rs485_select); }}
        , Line {"Тех. настройки"    ,[this]{ change_screen(tech_select);  }}
    };
    Select_screen<4> rs485_select {
          lcd
        , Up_event    { [this](auto c){   up.set_click_callback(c);}     }
        , Down_event  { [this](auto c){ down.set_click_callback(c);}     }
        , Enter_event { [this](auto c){enter.set_click_callback(c);}     }
        , Out_event   { [this](auto c){enter.set_long_push_callback(c);} }
        , Out_callback    { [this]{ change_screen(config_select);  }}
        , Line {"Адрес"       ,[]{}}
        , Line {"Скорость"    ,[]{}}
        , Line {"Проверка"    ,[]{}}
        , Line {"Стоп биты"   ,[]{}}
    };
    Select_screen<4> tech_select {
          lcd
        , Up_event    { [this](auto c){   up.set_click_callback(c);}     }
        , Down_event  { [this](auto c){ down.set_click_callback(c);}     }
        , Enter_event { [this](auto c){enter.set_click_callback(c);}     }
        , Out_event   { [this](auto c){enter.set_long_push_callback(c);} }
        , Out_callback    { [this]{ change_screen(config_select);  }}
        , Line {"Наименование" ,[this]{ change_screen(name_set);   }}
        , Line {"туду"   ,[]{}}
        , Line {"туду"   ,[]{}}
        , Line {"туду"   ,[]{}}
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
        , 1, 100
    };
    Set_screen<int, models_to_string> name_set {
          lcd
        , Up_event    { [this](auto c){   up.set_click_callback(c);}     }
        , Down_event  { [this](auto c){ down.set_click_callback(c);}     }
        , Enter_event { [this](auto c){enter.set_click_callback(c);}     }
        , Out_event   { [this](auto c){enter.set_long_push_callback(c);} }
        , Out_callback    { [this]{ change_screen(config_select);  }}
        , "Наименование установки"
        , flash.model_number
        , 0, models.size()
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