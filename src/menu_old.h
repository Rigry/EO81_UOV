#pragma once

// #include "rus_string.h"
#include "delay.h"
#include "string_buffer.h"
#include "hd44780.h"
#include "select_screen.h"

template <class Up, class Down>
class Menu 
{
public:

   Menu(String_buffer& lcd)
      : up   {mcu::Button::make<Up>()}
      , down {mcu::Button::make<Down>()}
      , lcd  {lcd}
   {}

   void operator() () {

      switch (screen) {
         case Select::main:
            lcd.clear();
            lcd.line(0).center() << make_model[0] << model_type[3];
            lcd.line(1) << "Ламп:" << modbus_qty_lamp;
            lcd.line(2) << "Авария:никаких";
            lcd.line(3).cursor(0) << "t"; lcd.cursor(1).width(3) << modbus_temp << "C";
            lcd.line(3).cursor(8) << "УФ";
            lcd.line(3).cursor(14)<< "УЗ";
            if ((up and down).push_long())
               screen = Select::select;
         break;
         case Select::select:
            select_screen([&]{screen = Select::main;});
         break;
         case Select::emergency:
            emergency_screen([&]{screen = Select::select;});
         break;
         case Select::time_lamp:
            time_lamp_screen([&]{screen = Select::select;});
         break;
         case Select::configuration:
            config_screen([&](){screen = Select::select;});
         break;
         case Select::logs:
            log_screen([&](){screen = Select::select;});
         break;

      }
   }

private:
   enum class Select {
      main, select, emergency, time_lamp, configuration, logs
   } screen {Select::main};

   std::array<int, 14> model_type {   1,   5,  10,  15,  30,  50,   100,
                                    150, 200, 300, 500, 700, 1000, 1500};
   std::array<std::string_view, 2> make_model {"УОВ-ПВ-", "УОВ-СВ-"};
 
   Button up;
   Button down;
   String_buffer& lcd;
   
   Select_screen<6>select_screen    {up, down, lcd
                            ,Line {"Аварии"      ,[&]{screen = Select::emergency;    }}
                            ,Line {"Наработка"   ,[&]{screen = Select::time_lamp;    }}
                            ,Line {"Конфигурация",[&]{screen = Select::configuration;}}
                            ,Line {"Лог работы"  ,[&]{screen = Select::logs;         }}
                            ,Line {"Настройки"   ,[&]{screen = Select::configuration;}}
                            ,Line {"Наработка"   ,[&]{screen = Select::time_lamp    ;}} };
   Select_screen<3>emergency_screen{up, down, lcd
                            ,Line {"Нерабочие лампы"   ,[&](){}}
                            ,Line {"Ошибки линии RS485",[&](){}}
                            ,Line {"Сбросить аварии"   ,[&](){}} };
   Select_screen<2>time_lamp_screen{up, down, lcd
                            ,Line {"Просмотр"       ,[&](){}}
                            ,Line {"Сброс наработки",[&](){}} };
   Select_screen<4>config_screen   {up, down, lcd
                            ,Line {"Просмотр конф-ции" ,[&](){}}
                            ,Line {"Настройки"         ,[&](){}}
                            ,Line {"Настройки конф-ции",[&](){}}
                            ,Line {"Настройки сети"    ,[&](){}} };
   Select_screen<2>log_screen      {up, down, lcd
                            ,Line {"Просмотреть лог",[&](){}}
                            ,Line {"Сбросить лог"   ,[&](){}} };

   size_t modbus_qty_lamp {9};
   size_t modbus_temp {25};

};