#include "AuUtils.h"
#include "pkfuncs.h"

namespace Utils {
namespace AU {

    bool getSystemID(SYSTEMID& systemID)
    {
        ULONG        ResultSize;
        /*
        * Use the kernel IOCTL to retrieve PRID/BRDID/SCRATCH/GUID
        */
        if(!KernelIoControl(IOCTL_OEM_GET_SYSTEMID, NULL, 0, &systemID, sizeof(SYSTEMID), &ResultSize))
            return false;
        return true;
    }

    bool getAUOTP(OTP& otp)
    {
        SYSTEMID systemID;
        
        if(!getSystemID(systemID))
            return false;
        memcpy((void *)&otp, systemID.guid, sizeof(otp));

        return true;
    }

}; // AU
}; // Utils