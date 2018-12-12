#define STM32F405xx
#define F_CPU   8000000UL

// #include "RCC.h"
#include "pin.h"
#include "buttons.h"


extern "C" void init_clock (){
   
}

using Up   = mcu::PB8;
using Down = mcu::PB9;
using Us   = mcu::PB0;
using Uv   = mcu::PB1;

using Alarm  = mcu::PC7;
using Led_us = mcu::PC6;
using Led_uv = mcu::PB12;

using Relay_uv = mcu::PB10;
using Relay_us = mcu::PB11;

int main()
{
   auto buttons = mcu::Buttons::make<mcu::PA1, mcu::PA2>();
}