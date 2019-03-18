#pragma once
#include <utility>
#include "buttons.h"
#include "function.h"
#include "string_buffer.h"

struct Line {
      std::string_view name;
      Function<void()> callback;
      // Lines(std::string_view name, int callback)
      //    : name {name}
      //    , callback {callback}
      // {}
   };

template <int qty>
class Screen
{
public:
   template <class...Line>
   Screen(Button& up, Button& down, String_buffer& lcd
         , Line ... lines
         ) 
         : up   {up}
         , down {down}
         , lcd  {lcd}
         , lines {lines...}
   {}

   void operator()(Function<void()> f)
   {
      lcd.clear();
      for (int i = 0; i < qty; i++) {
         lcd.line(i) << lines[i].name;
      }
      if (next())
         lines[carriage].callback();
      if (back()) {
         f();
         carriage = 0;
      }

      if (up.push()) { 
         carriage -= 1;
         if (carriage < 0) carriage = qty - 1;
      }
      if (down.push()) {
         carriage += 1;
         if (carriage >= qty) carriage = 0;
      } 
      for (int i = 0; i < qty; i++)
      {
         lcd.line(i).cursor(19) << " ";
         if (i == carriage)
            lcd.line(i).cursor(19) << "~";
      }

   }
private:

//    using Lines = std::pair<std::string_view, Function<void()>>;

   Button& up;
   Button& down;
   String_buffer& lcd;
   std::array<Line, qty> lines;
   bool back(){return (up and down).push_long();}
   bool next(){return (up and down).click();}
   int carriage{0};

};
