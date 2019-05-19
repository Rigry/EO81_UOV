#pragma once


// #include "bit_set.h"
// #include "pin.h"
// #include "button.h"
// #include "modbus_slave.h"
// #include "modbus_master.h"
// #include "hysteresis.h"


// using EPRA1  = mcu::PA0;        using E   = mcu::PB3;       
// using EPRA2  = mcu::PA1;        using RW  = mcu::PD2;       
// using EPRA3  = mcu::PA2;        using RS  = mcu::PC12;      
// using EPRA4  = mcu::PA3;        using DB4 = mcu::PB4;       
// using EPRA5  = mcu::PA4;        using DB5 = mcu::PB5;
// using EPRA6  = mcu::PA5;        using DB6 = mcu::PB6;    
// using EPRA7  = mcu::PA6;        using DB7 = mcu::PB7;
// using EPRA8  = mcu::PA7;
// using EPRA9  = mcu::PC4;
// using EPRA10 = mcu::PC5;

// using BTN_UP   = mcu::PB8;      using LED1 = mcu::PB12;
// using BTN_DOWN = mcu::PB9;      using LED2 = mcu::PC6;
// using BTN_UF   = mcu::PB1;      using LED3 = mcu::PC7;
// using BTN_UZG  = mcu::PB0;      using LED4 = mcu::PA12;
// using R_UF     = mcu::PB10;     using LED5 = mcu::PC9;
// using R_UZG    = mcu::PB11;



// void process()
// {
//    


//    // slave reaction
//    slave ([&](uint16_t registrAddress) {
//          static bool unblock = false;
//       switch (registrAddress) {
//          case ADR(uart_set):
//             flash.uart_set
//                = slave.outRegs.uart_set
//                = slave.inRegs.uart_set;
//          break;
//          case ADR(modbus_address):
//             flash.modbus_address 
//                = slave.outRegs.modbus_address
//                = slave.inRegs.modbus_address;
//          break;
//          case ADR(password):
//             unblock = slave.inRegs.password == 208;
//          break;
//          case ADR(factory_number):
//             if (unblock) {
//                unblock = false;
//                flash.factory_number 
//                   = slave.outRegs.factory_number
//                   = slave.inRegs.factory_number;
//             }
//             unblock = true;
//          break;
//          case ADR (max_temperature):
//             flash.max_temperature
//             = slave.outRegs.max_temperature
//             = slave.inRegs.max_temperature;
//          break;
//          case ADR (uv_level_min):
//             flash.uv_level_min
//             = slave.outRegs.uv_level_min
//             = slave.inRegs.uv_level_min;
//          break;
//          case ADR (qty_uv_lamps):
//             flash.qty_uv_lamps
//             = slave.outRegs.qty_uv_lamps
//             = slave.inRegs.qty_uv_lamps;
//          break;
//          case ADR (uv_level_highest):
//             flash.uv_level_highest
//             = slave.outRegs.uv_level_highest
//             = slave.inRegs.uv_level_highest;
//          break;
//          case ADR (hours):

//          break;
//       }

//    });

   // auto& lamp_1  = Pin::make<EPRA1,  PinMode::Input>();
   // auto& lamp_2  = Pin::make<EPRA2,  PinMode::Input>();
   // auto& lamp_3  = Pin::make<EPRA3,  PinMode::Input>();
   // auto& lamp_4  = Pin::make<EPRA4,  PinMode::Input>();
   // auto& lamp_5  = Pin::make<EPRA5,  PinMode::Input>();
   // auto& lamp_6  = Pin::make<EPRA6,  PinMode::Input>();
   // auto& lamp_7  = Pin::make<EPRA7,  PinMode::Input>();
   // auto& lamp_8  = Pin::make<EPRA8,  PinMode::Input>();
   // auto& lamp_9  = Pin::make<EPRA9,  PinMode::Input>();
   // auto& lamp_10 = Pin::make<EPRA10, PinMode::Input>();
   
   // auto& us = Pin::make<R_UZG, PinMode::Output>();
   // auto& uv = Pin::make<R_UF,  PinMode::Output>();

   // auto& up    = Button::make<BTN_UP  >();
   // auto& down  = Button::make<BTN_DOWN>();
   // auto& us_on = Button::make<BTN_UZG>(); 
   // auto& uv_on = Button::make<BTN_UF >();

   // auto& uv_led = Pin::make<LED1, PinMode::Output>();
   // auto& us_led = Pin::make<LED2, PinMode::Output>();
   // auto& alarm  = Pin::make<LED3, PinMode::Output>();

   // Hysteresis<uint16_t> heat;
// }
