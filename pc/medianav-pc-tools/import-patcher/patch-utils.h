#pragma once
#include <stdint.h>
#include <memory>

namespace PatchUtils {
   
    class MIPSTableCall
    {
    public:
        MIPSTableCall(uint32_t tableVA, uint32_t tableIndex)
        {
            createCall(tableVA + tableIndex * sizeof(uint32_t));
        }

        MIPSTableCall(uint32_t targetAddr)
        {
            createCall(targetAddr);
        }

        size_t getDataSize() const
        {
            return sizeof(dataBuf);
        }

        const uint8_t * getData() const
        {
            return reinterpret_cast<const uint8_t *>(dataBuf);
        }
    private:
        void createCall(uint32_t targetAddr)
        {
            uint16_t hiAddr = (targetAddr >> 16);
            uint16_t loAddr = targetAddr & 0xFFFF;
            if (loAddr >= 0x8000)
            {
                hiAddr += 1;
            }
            dataBuf[0] = 0x3C080000 + hiAddr; // lui $t0, hiAddr
            dataBuf[1] = 0x8D080000 + loAddr; // lw  $t0, loAddr($t0)
            dataBuf[2] = 0x0100F809; //jalr $t0
        }

        uint32_t dataBuf[3];
    };

    class MIPSNop
    {
    public:
        MIPSNop(size_t instrCount)
            :bufSize(instrCount * sizeof(uint32_t)), dataBuf(new uint8_t[bufSize])
        {
            std::memset(dataBuf.get(), 0, bufSize);
        }

        MIPSNop(uint32_t startVA, uint32_t stopVA)
            :bufSize(stopVA - startVA), dataBuf(new uint8_t[bufSize])
        {
            std::memset(dataBuf.get(), 0, bufSize);
        }

        size_t getDataSize() const
        {
            return bufSize;
        }

        const uint8_t * getData() const
        {
            return reinterpret_cast<const uint8_t *>(dataBuf.get());
        }
    protected:
        size_t bufSize;
        std::unique_ptr<uint8_t> dataBuf;
    };

    class MIPSTableCallWithPadding : public MIPSNop
    {
    public:
        MIPSTableCallWithPadding(uint32_t startVA, uint32_t stopVA, uint32_t targetAddr)
            :MIPSNop(startVA, stopVA)
        {
            MIPSTableCall call(targetAddr);
            if (getDataSize() < call.getDataSize())
                throw std::exception("The area is too small to inplace call pattern");

            memcpy(&(dataBuf.get()[bufSize - call.getDataSize()]), call.getData(), call.getDataSize());
        }
    };


}; //namespace PatchUtils
