#pragma once

#include "rus_string.h"
#include "delay.h"
#include "string_buffer.h"
#include "hd44780.h"

class Menu 
{
   enum class Screen {
      main, select, emergency, time_lamp, configuration, logs
   }
   
   Button& up;
   Button& down;
   String_buffer& lcd;
   Menu(Button& up, Button& down, String_buffer& lcd) 
      : up   {up}
      , down {down}
      , lcd  {lcd}
   {}

public:
   
   template <class UP, class DOWN>
   static auto& make(String_buffer& lcd)
   {
      static auto menu = Menu
      {
         mcu::Button::make<UP>(),
         mcu::Button::make<DOWN>(),
         lcd
      };
      // HD44780::make<RS, RW, E, DB4, DB5, DB6, DB7>(lcd.get_buffer());
      // вывести главный экран
      return menu;
   }

   void operator() () {
      
      // if (up.push())
      //    lcd.line(0).center()  << "Hello, World!" << next_line;
      // else if (down.push())
      //    lcd.line(0).center() << "Alex_Plus";
   }

};