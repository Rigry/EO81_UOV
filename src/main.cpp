#define STM32F103xB
#define F_OSC   8000000UL
#define F_CPU   72000000UL

#include "stm32f103xb.h"
#include "periph_rcc.h"
#include "periph_flash.h"
#include "main.h"
#include "menu.h"


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
   Menu<BTN_UP, BTN_DOWN> menu (lcd);
   // auto& led2 = Pin::make<mcu::PC6, mcu::PinMode::Output>();
   // auto up = mcu::Button::make<mcu::PB8>();
   // auto down = mcu::Button::make<mcu::PB9>();
   
   while (1) {

      menu();
      // if (up.click())
         // led2 ^= up.click();
      // if (down.push_long())
         // led2 = true;
      // process();
      // us_led = us ^= us_on;
      // uv_led = uv ^= uv_on;
      __WFI();
   }

}
