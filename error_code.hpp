#ifndef APM_SERIAL_LOG_ERROR_CODE_HPP_INCLUDED
#define APM_SERIAL_LOG_ERROR_CODE_HPP_INCLUDED

#include "arduino_compat.h"

struct error_code{
   static constexpr byte sd_init = 3;
   static constexpr byte card_init = 6;
   static constexpr byte volume_init = 7;
   static constexpr byte root_init = 8;
   static constexpr byte file_open = 9;
};

#endif // APM_SERIAL_LOG_ERROR_CODE_HPP_INCLUDED
