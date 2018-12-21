#define STM32F103xB
#define F_OSC   8000000UL
#define F_CPU   72000000UL
#include "stm32f103xb.h"
#include "rcc.h"
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

// auto& PB = mcu::make_reference<mcu::Periph::GPIOB>();
// volatile auto& b = PB.like_CMSIS();

int main()
{
   // auto up = mcu::Button::make<mcu::PB8>();
   // auto down = mcu::Button::make<mcu::PB9>();
   // auto delay = mcu::Delay::make<mcu::Periph::TIM4>(); 
   auto& led3 = mcu::Pin::make_new<mcu::PC7, mcu::PinMode::Output>();
   auto& led2 = mcu::Pin::make_new<mcu::PC6, mcu::PinMode::Output>();
   // // auto led5 = mcu::Pin::make_new<mcu::PC9, mcu::PinMode::Output>();
   auto& led1 = mcu::Pin::make_new<mcu::PB12, mcu::PinMode::Output>();
   auto& lcd = mcu::LCD::make<mcu::PC12, mcu::PD2, mcu::PB3, mcu::PB4, mcu::PB5, mcu::PB6, mcu::PB7, mcu::PB8>();
   // // auto lcd = mcu::LCD::make<mcu::PC12, mcu::PD2, mcu::PA12, mcu::PB12, mcu::PC6, mcu::PC7, mcu::PC9>();
   // auto& pb4 = mcu::Pin::make_new<mcu::PB4, mcu::PinMode::Output>();
   // auto& portC = mcu::make_reference<mcu::Periph::GPIOC>();
   // auto& portB = mcu::make_reference<mcu::Periph::GPIOB>();
   // lcd.clear();
   // lcd.send();

  

   // Timer timer {10};
   // Timer timer1 {200};
   // Timer timer2 {100};

   // while(1){
      
      // delay<100000>();
      // led1 ^= true;

      // int n = 1000;
      // while(n){
         // delay<10>();
         // n--;
      // }

      // volatile GPIO_TypeDef* b = (GPIO_TypeDef*) GPIOB_BASE;

      // RCC->APB2ENR |= RCC_APB2ENR_IOPBEN_Msk;
      // while ((RCC->APB2ENR & RCC_APB2ENR_IOPBEN_Msk) == 0) {}
      // b->CRL = 0x11111444;

	   // b->BSRR = 1 << 4+16;

	   // b->BSRR = 1 << 4;

      // pb4 ^= true;

      // delay<100>();
      // b.ODR = 0;
      
      // delay<100>();
      // b.ODR = 1 << 4;

      
      // if(timer.event())
      //    led2 ^= true;
      

   // }


   while (1) { 
      led3 = true;
      delay<50000>();
      led3 = false; 
      delay<50000>();
      led2 = true;
      delay<50000>();
      led2 = false;
      delay<50000>();
      led1 = true;
      delay<150000>();
      led1 = false;
      delay<150000>();
      led2 = true;
      delay<150000>();
      led2 = false;
      delay<200000>();
   

   }

   
}
