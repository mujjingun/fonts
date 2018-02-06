#include "otftable.hpp"

#include <cmath>

namespace fontutils {

uint32_t calculate_checksum (Buffer &dis, uint32_t length)
{
   uint32_t checksum = 0;
   uint32_t l;

   l = (length % 4 > 0) ? length / 4 + 1 : length / 4;

   for (uint32_t i = 0; i < l; i++) {
       checksum += dis.read<uint32_t>();
   }

   return checksum;
}

int le_pow2(int num)
{
    return 1 << std::ilogb(num);
}

}
