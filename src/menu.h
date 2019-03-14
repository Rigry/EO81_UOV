#pragma once

// #include "rus_string.h"
#include "delay.h"
#include "string_buffer.h"
#include "hd44780.h"
#include "select_screen.h"

class Menu 
{
   enum class Screen {
      main, select, emergency, time_lamp, configuration, logs
   } screen {Screen::main};

   std::array<std::string_view, 27> model {
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
   
   Button& up;
   Button& down;
   String_buffer& lcd;
   Select_screen select_screen {up, down, lcd}; 
   Menu(Button& up, Button& down, String_buffer& lcd)
      : up   {up}
      , down {down}
      , lcd  {lcd}
   {}

public:
   
   template <class Up, class Down>
   static auto& make(String_buffer& lcd)
   {
      static auto menu = Menu
      {
         mcu::Button::make<Up>(),
         mcu::Button::make<Down>(),
         lcd
      };
      // HD44780::make<RS, RW, E, DB4, DB5, DB6, DB7>(lcd.get_buffer());
      // вывести главный экран
      return menu;
   }

   void operator() () {
      
      switch (screen)
      {
         case Screen::main:
            lcd.line(0).center() << "Контроллер УОВ";
            if ((up and down).push_long()) {
               lcd.clear();
               screen = Screen::select;
            }
         break;
         case Screen::select:
            lcd.line(0) << "Аварии";
            lcd.line(1) << "Наработка";
            lcd.line(2) << "Конфигурация";
            lcd.line(3) << "Лог работы";
            select_screen.set_screen(4);
            
            if ((up and down).push_long()) {
               lcd.clear();
               screen = Screen::main;
            } else if (select_screen.screen() == 1 and (up and down).click()) {
               lcd.clear();
               select_screen.set_position(1);
               screen = Screen::emergency;
            } else if (select_screen.screen() == 2 and (up and down).click()) {
               lcd.clear();
               select_screen.set_position(1);
               screen = Screen::time_lamp;
            } else if (select_screen.screen() == 3 and (up and down).click()) {
               lcd.clear();
               select_screen.set_position(1);
               screen = Screen::configuration;
            } else if (select_screen.screen() == 4 and (up and down).click()) {
               lcd.clear();
               select_screen.set_position(1);
               screen = Screen::logs;
            }
            // select_screen.position();
         break;
         case Screen::emergency:
            lcd.line(0) << "Нерабочие лампы";
            lcd.line(1) << "Ошибки линии RS485";
            lcd.line(2) << "Сбросить аварии";
            select_screen.set_screen(3);
            if ((up and down).push_long()) {
               lcd.clear();
               select_screen.set_position(1);
               screen = Screen::select;
            }
         break;
         case Screen::time_lamp:
            lcd.line(0) << "Просмотр";
            lcd.line(1) << "Сброс наработки";
            select_screen.set_screen(2);
            if ((up and down).push_long()) {
               lcd.clear();
               select_screen.set_position(2);
               screen = Screen::select;
            }
         break;
         case Screen::configuration:
            lcd.line(0) << "Просмотр конф-ции";
            lcd.line(1) << "Настройки";
            lcd.line(2) << "Настройки конф-ции";
            lcd.line(3) << "Настройки сети";
            select_screen.set_screen(4);
            if ((up and down).push_long()) {
               lcd.clear();
               select_screen.set_position(3);
               screen = Screen::select;
            }
         break;
         case Screen::logs:
            lcd.line(0) << "Просмотреть лог";
            lcd.line(1) << "Сбросить лог";
            select_screen.set_screen(2);
            if ((up and down).push_long()) {
               lcd.clear();
               select_screen.set_position(4);
               screen = Screen::select;
            }
         break;
      
      }
      
      if (screen != Screen::main)
         select_screen.position();
      // if (up.push())
      //    lcd.line(0).center()  << "Hello, World!" << next_line;
      // else if (down.push())
      //    lcd.line(0).center() << "Alex_Plus";
   }

};