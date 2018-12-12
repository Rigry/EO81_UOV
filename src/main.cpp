#define STM32F103xB
#define F_OSC   8000000UL
#define F_CPU   72000000UL
#include "rcc.h"
#include "periph_flash.h"
#include "pin.h"
#include "buttons.h"
#include "timers.h"


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
   auto down = mcu::Button::make<mcu::PB9>();

   auto led3 = mcu::Pin::make_new<mcu::PC7, mcu::PinMode::Output>();
   auto led2 = mcu::Pin::make_new<mcu::PC6, mcu::PinMode::Output>();
   auto led1 = mcu::Pin::make_new<mcu::PB12, mcu::PinMode::Output>();

   Timer timer {500};
   Timer timer1 {200};
   Timer timer2 {100};

   while(1) {
      if (up and down)
         led2 = true;
      if (up and down)
         led2 = false;
      if (timer.event())
         led3 ^= true;
      if (timer2.event())
         led1 ^= true;

   }
}
