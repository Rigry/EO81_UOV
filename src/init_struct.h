#pragma once

#include <cstdint>
#include <bitset>
#include <array>
#include "uart.h"




// fuction 05
struct Control_reg {
   
   bool us; // on/off ultrasound
   bool uv; // on/off ultraviolet

};

struct Exsist {
   bool Board   :1;
   bool Temp    :1;
   bool UV      :1;
};