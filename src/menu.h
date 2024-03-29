#pragma once

#include "string_buffer.h"
#include "hd44780.h"
#include "select_screen.h"
#include "set_screen.h"
#include "screens.h"
#include "button.h"
#include "work_count.h"



template<class Pins, class Flash_data, class Modbus_regs, class Reg>
struct Menu : TickSubscriber {
    String_buffer lcd {};
    HD44780& hd44780 { HD44780::make(Pins{}, lcd.get_buffer()) };
    Button_event& up;
    Button_event& down;
    Button_event& enter;
    Flash_data&   flash;
    Modbus_regs&  modbus;
    Work_count&   work_count;
    Reg& master_regs;

    Screen* current_screen {&main_screen};
    size_t tick_count{0};

    Buttons_events buttons_events {
          Up_event    { [this](auto c){   up.set_click_callback(c);}     }
        , Down_event  { [this](auto c){ down.set_click_callback(c);}     }
        , Enter_event { [this](auto c){enter.set_click_callback(c);}     }
        , Out_event   { [this](auto c){enter.set_long_push_callback(c);} }
        , Increment_up_event   { [this](auto c){  up.set_increment_callback(c);} }
        , Increment_down_event { [this](auto c){down.set_increment_callback(c);} }
    };


    Menu (
        Pins pins
        , Button_event& up
        , Button_event& down
        , Button_event& enter
        , Flash_data&   flash
        , Modbus_regs&  modbus
        , Work_count&   work_count
        , Reg&          master_regs
    ) : up{up}, down{down}, enter{enter}
      , flash{flash}, modbus{modbus}, work_count{work_count}, master_regs {master_regs}
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
        , modbus.work_flags
        , flash.exist
    };

    Select_screen<5> main_select {
          lcd, buttons_events
        , Out_callback       { [this]{ change_screen(main_screen);  }}
        , Line {"Аварии"      ,[this]{ change_screen(alarm_select); }}
        , Line {"Наработка"   ,[this]{ change_screen(work_select);  }}
        , Line {"Конфигурация",[this]{ change_screen(config_select);}}
        , Line {"Лог работы"  ,[this]{ change_screen(log_screen);   }}
        , Line {"Режим работы",[this]{ change_screen(mode_screen);  }}
    };

    int mode {flash.automatic};
    Set_screen<int, mode_to_string> mode_screen {
          lcd, buttons_events
        , "Режим работы"
        , mode
        , Min<int>{0}, Max<int>{1}
        , Out_callback    { [this]{ change_screen(main_select); }}
        , Enter_callback  { [this]{ 
            flash.automatic = modbus.work_flags.mode = mode;
            change_screen(main_select);
        }}
    };

    Select_screen<3> alarm_select {
          lcd, buttons_events
        , Out_callback    { [this]{ change_screen(main_select);  }}
        , Line {"Нерабочие лампы"    ,[this]{ change_screen(bad_lamps_screen);}}
        , Line {"Ошибки линии RS485" ,[]{}}
        , Line {"Сбросить аварии"    ,[]{}}
    };

    Bad_lamps_screen bad_lamps_screen {
          lcd, buttons_events
        , Out_callback       { [this]{ change_screen(alarm_select);  }}
        , modbus.bad_lamps, flash.qty_lamps_uov, flash.qty_lamps_ext_1, flash.qty_lamps_ext_2, modbus.work_flags
    };

    Select_screen<3> work_select {
          lcd, buttons_events
        , Out_callback             { [this]{ change_screen(main_select);  }}
        , Line {"Просмотр наработки",[this]{ change_screen(work_time_screen);}}
        , Line {"Сброс всех ламп   ",[this]{
            work_count.reset();
            master_regs.reset_hours_1 = 1023; // 10 bits
            master_regs.reset_hours_2 = 1023; // 10 bits
            flash.count.reset_all++;
            change_screen(work_time_screen); // чтоб увидеть действие
        }}
        , Line {"Сброс одной лампы",[this]{ 
            reset_n_set.max = modbus.quantity.lamps;
            change_screen(reset_n_set);
        }}

    };

    Work_time_screen work_time_screen {
          lcd, buttons_events
        , Out_callback       { [this]{ change_screen(work_select); master_regs.reset_hours_1 = 0; 
                                                                   master_regs.reset_hours_2 = 0; }}
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
            if ((reset_n-1) < flash.qty_lamps_uov) {
                work_count.reset(reset_n-1);
            } else if ((reset_n-1) < (flash.qty_lamps_ext_1 + flash.qty_lamps_uov)) {
                master_regs.reset_hours_1 = 1 <<  (reset_n-1-flash.qty_lamps_uov);
            } else if ((reset_n-1) < (flash.qty_lamps_ext_2 + flash.qty_lamps_ext_1 + flash.qty_lamps_uov))  {
                master_regs.reset_hours_2 = 1 <<  (reset_n-1-flash.qty_lamps_uov-flash.qty_lamps_ext_1);
            }
            
            flash.count.reset_one++;
            work_time_screen.set_first_lamp(reset_n-1);
            change_screen(work_time_screen);
        }}
    };

    Select_screen<4> config_select {
          lcd, buttons_events
        , Out_callback             { [this]{ change_screen(main_select);     }}
        , Line {"Просмотр конф.-ии" ,[this]{ change_screen(config_screen);   }}
        , Line {"Тревога УФ"        ,[this]{ change_screen(min_UV_set);      }}
        , Line {"Настройки modbus"  ,[this]{ change_screen(modbus_select);   }}
        , Line {"Тех. настройки"    ,[this]{ change_screen(password_screen); }}
    };

    Config_screen config_screen {
          lcd, buttons_events
        , Out_callback       { [this]{ change_screen(config_select);  }}
        , modbus.quantity, flash.exist
    };

    Set_screen<uint8_t> min_UV_set {
          lcd, buttons_events
        , "уровень УФ"
        , flash.uv_level_min
        , Min<uint8_t>{1}, Max<uint8_t>{100}
        , Out_callback    { [this]{ change_screen(config_select);  }}
    };

    Select_screen<5> modbus_select {
          lcd, buttons_events
        , Out_callback             { [this]{ change_screen(config_select);   }}
        , Line {"Адрес"             ,[this]{ change_screen(address_set);     }}
        , Line {"Скорость"          ,[this]{ change_screen(boudrate_set);    }}
        , Line {"Проверка"          ,[this]{ change_screen(parity_en_set);   }}
        , Line {"Тип проверки"      ,[this]{ change_screen(parity_set);      }}
        , Line {"Стоп биты"         ,[this]{ change_screen(stop_bits_set);   }}
    };

    Set_screen<uint8_t> address_set {
          lcd, buttons_events
        , "Адрес modbus"
        , flash.modbus_address
        , Min<uint8_t>{1}, Max<uint8_t>{255}
        , Out_callback    { [this]{ change_screen(modbus_select);  }}
    };

    // приходится сохранять, так как нельзя сделать ссылку на член битового поля
    uint8_t boudrate_ {flash.uart_set.baudrate};
    Set_screen<uint8_t, boudrate_to_string> boudrate_set {
          lcd, buttons_events
        , "Скорость в бодах"
        , boudrate_
        , Min<uint8_t>{0}, Max<uint8_t>{::boudrate.size() - 1}
        , Out_callback    { [this]{ change_screen(modbus_select); }}
        , Enter_callback  { [this]{ 
            flash.uart_set.baudrate = USART::Baudrate(boudrate_);
            change_screen(modbus_select);
        }}
    };

    uint8_t parity_enable_ {flash.uart_set.parity_enable};
    Set_screen<uint8_t, exist_to_string> parity_en_set {
          lcd, buttons_events
        , "Проверка на чет/нечет"
        , parity_enable_
        , Min<uint8_t>{0}, Max<uint8_t>{exist.size() - 1}
        , Out_callback    { [this]{ change_screen(modbus_select); }}
        , Enter_callback  { [this]{ 
            flash.uart_set.parity_enable = bool(parity_enable_);
            change_screen(modbus_select);
        }}
    };

    uint8_t parity_ {flash.uart_set.parity};
    Set_screen<uint8_t, parity_to_string> parity_set {
          lcd, buttons_events
        , "Проверка на"
        , parity_
        , Min<uint8_t>{0}, Max<uint8_t>{parity.size() - 1}
        , Out_callback    { [this]{ change_screen(modbus_select); }}
        , Enter_callback  { [this]{ 
            flash.uart_set.parity = USART::Parity(parity_);
            change_screen(modbus_select);
        }}
    };

    int stop_bits {flash.uart_set.stop_bits == USART::StopBits::_1 ? 1 : 2};
    Set_screen<int> stop_bits_set {
          lcd, buttons_events
        , "Количество стоп бит"
        , stop_bits
        , Min<int>{1}, Max<int>{2}
        , Out_callback    { [this]{ change_screen(modbus_select); }}
        , Enter_callback  { [this]{ 
            flash.uart_set.stop_bits = 
                stop_bits == 1 
                    ? USART::StopBits::_1
                    : USART::StopBits::_2
            ;
            change_screen(modbus_select);
        }}
    };

    Password_screen password_screen {
          lcd, buttons_events
        , Out_callback      { [this]{ change_screen(config_select); }}
        , Password_callback { [this](int password){ 
            if (password == glob::password)
                change_screen(tech_select);
            else
                change_screen(config_select);
        }}
    };

    int confirm;
    Select_screen<12> tech_select {
          lcd, buttons_events
        , Out_callback        { [this]{ change_screen(config_select);           }}
        , Line {"Наименование"       ,[this]{ change_screen(name_set);          }}
        , Line {"Максимум УФ"        ,[this]{ change_screen(max_uv_set);        }}
        , Line {"Сброс максимума УФ" ,[this]{ confirm = 0; 
                                              change_screen(max_uv_reset);      }}
        , Line {"Кол-во ламп тут"    ,[this]{ change_screen(qty_lamps_set);     }}
        , Line {"Кол-во расширений"  ,[this]{ change_screen(qty_extentions_set);}}
        , Line {"Макс. температура"  ,[this]{ change_screen(max_temp_set);      }}
        , Line {"Темп. восстановл."  ,[this]{ change_screen(recovery_temp_set); }}
        , Line {"Сброс лога"         ,[this]{
            confirm = 0;
            change_screen(log_reset);
        }}
        , Line {"Датчик температуры" ,[this]{ change_screen(sens_temp_set);   }}
        , Line {"Датчик УФ"          ,[this]{ change_screen(sens_uv_set);     }}
        , Line {"Сухие контакты"     ,[this]{ change_screen(dry_contacts_set);}}
        , Line {"10 выход"           ,[this]{ change_screen(outputs);         }}
    };

    Set_screen<uint8_t, model_to_string> name_set {
          lcd, buttons_events
        , "Наименование уст."
        , flash.model_number
        , Min<uint8_t>{0}, Max<uint8_t>{models.size() - 1}
        , Out_callback    { [this]{ change_screen(tech_select);  }}
    };

    Set_screen<uint16_t> max_uv_set {
          lcd, buttons_events
        , "Максимум УФ"
        , flash.uv_level_highest
        , Min<uint16_t>{0}, Max<uint16_t>{0xFFFF}
        , Out_callback    { [this]{ change_screen(tech_select);  }}
    };

    Set_screen<int, confirm_to_string> max_uv_reset {
          lcd, buttons_events
        , "Сбросить макс УФ?"
        , confirm
        , Min<int>{0}, Max<int>{1}
        , Out_callback    { [this]{ change_screen(tech_select);  }}
        , Enter_callback  { [this]{
            if (confirm) {
                flash.uv_level_highest = 0;
            }
            change_screen(tech_select);
        }}
    };

    Set_screen<uint8_t> qty_lamps_set {
          lcd, buttons_events
        , "Кол-во ламп тут"
        , flash.qty_lamps_uov
        , Min<uint8_t>{0}, Max<uint8_t>{10}
        , Out_callback    { [this]{ change_screen(tech_select);  }}
        , Enter_callback  { [this]{
            modbus.quantity = flash.quantity;
            change_screen(tech_select);
        }}
    };

    Set_screen<uint8_t> qty_extentions_set {
          lcd, buttons_events
        , "Кол-во расширений"
        , flash.quantity.extantions
        , Min<uint8_t>{0}, Max<uint8_t>{glob::max_extantions}
        , Out_callback    { [this]{ change_screen(tech_select);  }}
        , Enter_callback  { [this]{
            modbus.quantity = flash.quantity;
            change_screen(tech_select);
        }}
    };

    Set_screen<uint8_t> max_temp_set {
          lcd, buttons_events
        , "Макс. температура"
        , flash.max_temperature
        , Min<uint8_t>{20}, Max<uint8_t>{glob::max_temperature}
        , Out_callback    { [this]{ change_screen(tech_select);  }}
    };

    Set_screen<uint8_t> recovery_temp_set {
          lcd, buttons_events
        , "Темп. восстановл."
        , flash.temperature_recovery
        , Min<uint8_t>{5}, Max<uint8_t>{glob::max_temperature}
        , Out_callback    { [this]{ change_screen(tech_select);  }}
    };

    Set_screen<uint8_t> low_uv_set {
          lcd, buttons_events
        , "Уровень низк. УФ"
        , flash.uv_level_min
        , Min<uint8_t>{1}, Max<uint8_t>{100}
        , Out_callback    { [this]{ change_screen(tech_select);  }}
    };

    Set_screen<int, confirm_to_string> log_reset {
          lcd, buttons_events
        , "Сбросить лог?"
        , confirm
        , Min<int>{0}, Max<int>{1}
        , Out_callback    { [this]{ change_screen(tech_select);  }}
        , Enter_callback  { [this]{
            if (confirm) {
                flash.count.on        = 0;
                flash.count.reset_all = 0;
                flash.count.reset_one = 0;
                flash.count.reset_log++;
            }
            change_screen(tech_select);
        }}
    };

    int temp_sensor {flash.exist.temp_sensor};
    Set_screen<int, exist_to_string> sens_temp_set {
          lcd, buttons_events
        , "Датчик температуры"
        , temp_sensor
        , Min<int>{0}, Max<int>{1}
        , Out_callback    { [this]{ change_screen(tech_select); }}
        , Enter_callback  { [this]{ 
            flash.exist.temp_sensor = bool(temp_sensor);
            change_screen(tech_select);
        }}
    };

    int uv_sensor {flash.exist.uv_sensor};
    Set_screen<int, exist_to_string> sens_uv_set {
          lcd, buttons_events
        , "Датчик УФ"
        , uv_sensor
        , Min<int>{0}, Max<int>{1}
        , Out_callback    { [this]{ change_screen(tech_select); }}
        , Enter_callback  { [this]{ 
            flash.exist.uv_sensor = bool(uv_sensor);
            change_screen(tech_select);
        }}
    };

    int dry_contacts {flash.exist.dry_contacts};
    Set_screen<int, exist_to_string> dry_contacts_set {
          lcd, buttons_events
        , "ЭО-101"
        , dry_contacts
        , Min<int>{0}, Max<int>{1}
        , Out_callback    { [this]{ change_screen(tech_select); }}
        , Enter_callback  { [this]{ 
            flash.exist.dry_contacts = bool(dry_contacts);
            change_screen(tech_select);
        }}
    };

    int reassignment {flash.reassignment};
    Set_screen<int, reassign_to_string> outputs {
        lcd, buttons_events
        , "Переназначение"
        , reassignment
        , Min<int>{0}, Max<int>{1}
        , Out_callback    { [this]{ change_screen(tech_select);  }}
        , Enter_callback  { [this]{
            flash.reassignment = bool(reassignment);
            change_screen(tech_select);
        }}
    };

    Log_screen log_screen {
          lcd, Out_event{buttons_events.out}
        , Out_callback       { [this]{ change_screen(main_select); }}
        , flash.count
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