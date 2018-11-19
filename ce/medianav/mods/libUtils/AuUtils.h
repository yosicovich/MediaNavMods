
#pragma once

#include "utils.h"
#include "db13xx.h"
#include "oemioctl.h"


namespace Utils {
namespace AU {
    
    bool getSystemID(SYSTEMID& systemID);
    bool getAUOTP(OTP& otp);

}; // AU
}; // Utils