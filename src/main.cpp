#define STM32F103xB
#define F_OSC   8000000UL
#define F_CPU   72000000UL
#include "stm32f103xb.h"
#include "periph_rcc.h"
#include "periph_flash.h"
#include "pin.h"
#include "buttons.h"
// #include "timers.h"
#include "delay.h"
#include "rus_string.h"
#include "string_buffer.h"
#include "hd44780.h"

using E  = mcu::PB3;
using RW = mcu::PD2;
using RS = mcu::PC12;
using DB4 = mcu::PB4;
using DB5 = mcu::PB5;
using DB6 = mcu::PB6;
using DB7 = mcu::PB7;


/// эта функция вызывается первой в startup файле
extern "C" void init_clock ()
{
   mcu::make_reference<mcu::Periph::FLASH>()
      .enable()
      .set(mcu::FLASH::Latency::_48_72MHz);
   

   mcu::make_reference<mcu::Periph::RCC>()
      .on_HSE()
      .wait_HSE_ready()
      .set      (mcu::RCC::AHBprescaler::AHBnotdiv)
      .set_APB1 (mcu::RCC::APBprescaler::APBdiv2)
      .set_APB2 (mcu::RCC::APBprescaler::APBnotdiv)
      .set      (mcu::RCC::PLLsource::HSE)
      .set      (mcu::RCC::PLLmultiplier::_9)
      .on_PLL()
      .wait_PLL_ready()
      .set      (mcu::RCC::SystemClock::CS_PLL);
}





int main()
{
   String_buffer lcd;
   HD44780::make<RS, RW, E, DB4, DB5, DB6, DB7>(lcd.get_buffer());
   lcd.center() << "Hello, World!" << next_line;
   lcd.center() << "Alex_Plus";
   
   
   
   while (1) {
      
   }

   
}
