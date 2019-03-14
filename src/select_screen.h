#pragma once
#include "buttons.h"
#include "string_buffer.h"

class Select_screen
{
   size_t line{1};
   size_t qty_screen{0};

   Button& up;
   Button& down;
   String_buffer& lcd;

public:

   Select_screen(Button& up,
                 Button& down,
                 String_buffer& lcd) 
                 : up   {up}
                 , down {down}
                 , lcd  {lcd} {};

   void set_screen(size_t qty) {qty_screen = qty;}
   void set_position(size_t n) {line = n;}
   void position() 
   {
      if (up.push()) { 
         line -= 1;
         if (line < 1) line = qty_screen;
      }
      if (down.push()) {
         line += 1;
         if (line > qty_screen) line = 1;
      } 
      for (size_t i = 1; i <= qty_screen; i++)
      {
         lcd.line(i-1).cursor(19) << " ";
         if (i == line)
            lcd.line(i-1).cursor(19) << "~";
      } 
   }

   auto screen(){return line;}
};