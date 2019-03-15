#pragma once
#include <utility>
#include "buttons.h"
#include "function.h"
#include "string_buffer.h"

template <int qty>
class Select_screen
{
   using Lines = std::pair<std::string_view, Function<void()>>;
   
   size_t line{1};
   size_t qty_screen{0};

   Button& up;
   Button& down;
   String_buffer& lcd;
   std::array<Lines, qty> lines;

public:

   template <class ... Lines>
   Select_screen( Button& up
                , Button& down
                , String_buffer& lcd
                , Lines ... lines
                ) 
                : up   {up}
                , down {down}
                , lcd  {lcd}
                , lines{lines...}
   {}

   void operator()()
   {
      for (auto i = 0; i < qty; i++) {
         lcd.line(i) << lines[i];
      }
   }


   void set_screen(size_t qty_) {qty_screen = qty_;}
   void set_position(size_t n) {line = n;}
   bool exit(){return (up and down).push_long();}
   bool menu(){return exit();}
   bool next(){return (up and down).click();}
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

// template <class ... Lines> 
// Select_screen(Button&, Button&, String_buffer&, Lines... l) 
// 	-> Select_screen<sizeof... (Lines)>;