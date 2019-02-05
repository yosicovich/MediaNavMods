#pragma once
#include <stdint.h>
#include <memory>
#include "PEFile.h"

namespace PatchUtils {
   
    class MIPSTableCall : public CodePatchCase
    {
    public:
        MIPSTableCall(uint32_t applyVA, uint32_t tableVA, uint32_t tableIndex)
            :MIPSTableCall(applyVA, tableVA + tableIndex * sizeof(uint32_t))
        {
        }

        MIPSTableCall(uint32_t applyVA, uint32_t targetAddr)
            :CodePatchCase(applyVA, 3 * sizeof(uint32_t))
        {
            createCall(targetAddr);
        }

    private:
        void createCall(uint32_t targetAddr)
        {
            uint32_t * patchBuf = reinterpret_cast<uint32_t *>(dataBuf.get());
            uint16_t hiAddr = (targetAddr >> 16);
            uint16_t loAddr = targetAddr & 0xFFFF;
            if (loAddr >= 0x8000)
            {
                hiAddr += 1;
            }
            patchBuf[0] = 0x3C080000 + hiAddr; // lui $t0, hiAddr
            patchBuf[1] = 0x8D080000 + loAddr; // lw  $t0, loAddr($t0)
            patchBuf[2] = 0x0100F809; //jalr $t0
            
            DWORD applyVA = getApplyVA();

            addRelocation(applyVA, RelocationTypePtr(new RelocationTypeHighAdj(loAddr)));
            applyVA += sizeof(DWORD);
            addRelocation(applyVA, RelocationTypePtr(new RelocationTypeGeneric(IMAGE_REL_BASED_LOW)));
        }

    };

    class MIPSNop : public CodePatchCase
    {
    public:
        MIPSNop(uint32_t startVA, uint32_t stopVA)
            :CodePatchCase(startVA, stopVA - startVA)
        {
            std::memset(dataBuf.get(), 0, bufSize);
        }
    };

    class MIPSTableCallWithPadding : public MIPSNop
    {
    public:
        MIPSTableCallWithPadding(uint32_t startVA, uint32_t stopVA, uint32_t targetAddr, bool bottomPlacement = true)
            :MIPSNop(startVA, stopVA)
        {
            MIPSTableCall call(startVA, targetAddr);
            if (getDataSize() < call.getDataSize())
                throw std::exception("The area is too small to inplace call pattern");

            if (bottomPlacement)
            {
                call = MIPSTableCall(startVA + getDataSize() - call.getDataSize(), targetAddr);
                memcpy(&(dataBuf.get()[bufSize - call.getDataSize()]), call.getData(), call.getDataSize());
            }
            else
            {
                memcpy(dataBuf.get(), call.getData(), call.getDataSize());
            }

            clearRelocations(call.getDeleteRelocations());
            if (!addRelocations(call.getNewRelocations()))
                throw std::exception("Relocation already defined!");
        }
    };


}; //namespace PatchUtils
