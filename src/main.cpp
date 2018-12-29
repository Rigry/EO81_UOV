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
#include "lcd.h"


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
   auto up = mcu::Button::make<mcu::PB8>();
   // auto down = mcu::Button::make<mcu::PB9>();
   // auto delay = mcu::Delay::make<mcu::Periph::TIM4>(); 
   // auto& led3 = Pin::make<mcu::PC7, mcu::PinMode::Output>();
   // auto& led2 = Pin::make<mcu::PC6, mcu::PinMode::Output>();
   // // auto led5 = mcu::Pin::make_new<mcu::PC9, mcu::PinMode::Output>();
   // auto& led1 = Pin::make<mcu::PB12, mcu::PinMode::Output>();
   auto& lcd = mcu::LCD::make<mcu::PC12, mcu::PD2, mcu::PB3, mcu::PB4, mcu::PB5, mcu::PB6, mcu::PB7>();
   
   lcd.set_line(1).central() << "Alex_Plus Ultrasound company";
   lcd.set_line(2).central() << string;
   lcd.set_line(0).set_cursor(5) << "Alex_Plus";
   // lcd << "Hello, World! Alex_Plus Ultrasound company";
//    lcd << "It was on the corner of the street that he noticed the first sign of\
// something peculiar - a cat reading a map. For a second, Mr. Dursley\
// didn't realize what he had seen -- then he jerked his head around to\
// look again.";
   // lcd.set_line(2).central() << "Hello, World!";
   lcd.set_line(3).central() << 587;
   
   // lcd << "Alex_";
   //  lcd.set_cursor(2) << "Hello, World!";
   int p {0};
   // lcd << p;
   // lcd << "Hello, World! Alex_Plus - Ultrasound company";

   while (1) { 
      // led3 = true;
      // delay<50000>();
      // led3 = false; 
      // delay<50000>();
      // led2 = true;
      // delay<50000>();
      // led2 = false;
      // delay<50000>();
      // led1 = true;
      // delay<150000>();
      // led1 = false;
      // delay<150000>();
      // led2 = true;
      // delay<150000>();
      // led2 = false;
      // delay<200000>();
      // lcd << "Hello, World!";
      // lcd << "Alex_Plus";
      // if (up)
      //    lcd << " - Ultrasound company";
      // lcd << p;
      // p++;
      // if (p > 9) p = 0;
   }

   
}
