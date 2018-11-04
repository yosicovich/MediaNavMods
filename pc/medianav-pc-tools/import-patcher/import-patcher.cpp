// import-patcher.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <PEFile.h>
#include "patch-utils.h"

using namespace PatchUtils;

void patchAppMain(const std::string& srcFile, const std::string& dstFile);
void patchMgrUsb(const std::string& srcFile, const std::string& dstFile);
void patchRVC(const std::string& srcFile, const std::string& dstFile);
void patchMicomManager(const std::string& srcFile, const std::string& dstFile);


int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("Usage: import-patcher.exe <source path> <destination path>\r\n");
        return -1;
    }

    patchAppMain(std::string(argv[1]) + "\\AppMain.exe", std::string(argv[2]) + "\\AppMain.exe");
    patchMgrUsb(std::string(argv[1]) + "\\MgrUSB.exe", std::string(argv[2]) + "\\MgrUSB.exe");
    patchRVC(std::string(argv[1]) + "\\RVC.dll", std::string(argv[2]) + "\\RVC.dll");
    patchMicomManager(std::string(argv[1]) + "\\MicomManager.exe", std::string(argv[2]) + "\\MicomManager.exe");
    return 0;

    PEFile pe(argv[1]);
    
    // Add import
    std::vector<std::string> functions = {
        "extCheckMediaFilesExtList"
    };
    pe.addImport("player_helper.dll", functions);

    //pe.setNoSectionTruncate(true);
    DWORD xxx = 0;

    //pe.patchSection(0x0001D398, &xxx, 4);

    // Add "MessageBoxA" & "ShowWindow" functions to the import table

    //pe.
    // Add a new section named ".at4re" with size "0x1000" byte
    //pe.addSection(".at4re", 0x1000, false);

    // Save the modified file
    pe.saveToFile(argv[1]);
    return 0;
}


void patchAppMain(const std::string& srcFile, const std::string& dstFile)
{
    PEFile pe(srcFile.c_str());

    // Add import
    std::vector<std::string> functions = {
        "extCheckMediaFilesExtList"
    };
    pe.addImport("player_helper.dll", functions);

    // Save import and reload the file for patching
    pe.saveAndReload(dstFile);

    // Hooks to extend file extensions support
    {
        {
            MIPSTableCallWithPadding call(0x0001D398, 0x0001D3BC, pe.getImportFunctionVA("player_helper.dll", "extCheckMediaFilesExtList"));
            pe.patchSection(0x0001D398, call.getData(), call.getDataSize());
        }

        {
            MIPSTableCallWithPadding call(0x00021BF0, 0x00021C14, pe.getImportFunctionVA("player_helper.dll", "extCheckMediaFilesExtList"));
            pe.patchSection(0x00021BF0, call.getData(), call.getDataSize());
        }

        {
            MIPSTableCallWithPadding call(0x000492A0, 0x000492CC, pe.getImportFunctionVA("player_helper.dll", "extCheckMediaFilesExtList"));
            pe.patchSection(0x000492A0, call.getData(), call.getDataSize());
        }

        {
            MIPSTableCallWithPadding call(0x0004C01C, 0x0004C040, pe.getImportFunctionVA("player_helper.dll", "extCheckMediaFilesExtList"));
            pe.patchSection(0x0004C01C, call.getData(), call.getDataSize());
        }
    }

    // Fix Total consumption display
    {
        BYTE patch = 0x11;
        pe.patchSection(0x0010211B, &patch, sizeof(BYTE));
        pe.patchSection(0x00102FC3, &patch, sizeof(BYTE));
        pe.patchSection(0x0010320F, &patch, sizeof(BYTE));
        pe.patchSection(0x001036F7, &patch, sizeof(BYTE));
        pe.patchSection(0x00103897, &patch, sizeof(BYTE));
        pe.patchSection(0x00103D1F, &patch, sizeof(BYTE));
        pe.patchSection(0x001047C7, &patch, sizeof(BYTE));
        pe.patchSection(0x00104B0B, &patch, sizeof(BYTE));
    }


    pe.saveToFile(dstFile.c_str());
}

void patchMgrUsb(const std::string& srcFile, const std::string& dstFile)
{
    PEFile pe(srcFile.c_str());

    // Add import
    std::vector<std::string> functions = {
        "extCheckMediaFileMatch",
        "extCheckMediaFileMatch2"
    };
    pe.addImport("player_helper.dll", functions);

    // Save import and reload the file for patching
    pe.saveAndReload(dstFile);

    // Hooks to extend file extensions support
    {
        {
            MIPSTableCallWithPadding call(0x000164C0, 0x000164CC, pe.getImportFunctionVA("player_helper.dll", "extCheckMediaFileMatch"));
            pe.patchSection(0x000164C0, call.getData(), call.getDataSize());

            DWORD cmd = 0x02002025; // move $a0, $s0
            pe.patchSection(0x000164CC, &cmd, sizeof(DWORD));
        }

        {
            MIPSTableCallWithPadding call(0x00013C38, 0x00013C48, pe.getImportFunctionVA("player_helper.dll", "extCheckMediaFileMatch2"));
            pe.patchSection(0x00013C38, call.getData(), call.getDataSize());

            DWORD cmd = 0x27A40038; // addiu   $a0, $sp, 0x270+var_238
            pe.patchSection(0x00013C48, &cmd, sizeof(DWORD));
        }
    }

    pe.saveToFile(dstFile.c_str());
}

void patchRVC(const std::string& srcFile, const std::string& dstFile)
{
    PEFile pe(srcFile.c_str());

    // Reduce number of ITE buffers 6->3
    {
        BYTE patch = 0x03;
        pe.patchSection(0x1000B724, &patch, sizeof(BYTE));
        pe.patchSection(0x1000B960, &patch, sizeof(BYTE));
        pe.patchSection(0x1000BE04, &patch, sizeof(BYTE));

        patch = 0x0C;
        pe.patchSection(0x1000C3F8, &patch, sizeof(BYTE));
        pe.patchSection(0x1000BBD8, &patch, sizeof(BYTE));
    }

    pe.saveToFile(dstFile.c_str());
}

void patchMicomManager(const std::string& srcFile, const std::string& dstFile)
{
    PEFile pe(srcFile.c_str());

    // Increase send buffer size
    {
        BYTE patch = 0x20;
        pe.patchSection(0x0002BDC8, &patch, sizeof(BYTE));

        patch = 0xDC;
        pe.patchSection(0x0002BDCC, &patch, sizeof(BYTE));

        patch = 0xD8;
        pe.patchSection(0x0002BDD0, &patch, sizeof(BYTE));

        patch = 0xD4;
        pe.patchSection(0x0002BDDC, &patch, sizeof(BYTE));

        patch = 0xF4;
        pe.patchSection(0x0002BDE0, &patch, sizeof(BYTE));

        patch = 0xF0;
        pe.patchSection(0x0002BE20, &patch, sizeof(BYTE));

        patch = 0xD4;
        pe.patchSection(0x0002BE64, &patch, sizeof(BYTE));

        patch = 0xD8;
        pe.patchSection(0x0002BE6C, &patch, sizeof(BYTE));

        patch = 0xDC;
        pe.patchSection(0x0002BE70, &patch, sizeof(BYTE));

        patch = 0xE0;
        pe.patchSection(0x0002BE78, &patch, sizeof(BYTE));
    }

    /*
    // Increase ACC_OFF timeout to 1 hour
    {
        DWORD timeoutMin = 1;// 60;
        timeoutMin *= 60000;
        ++timeoutMin; // Add one millisecond
        WORD lo = timeoutMin & 0xFFFF;
        WORD hi = timeoutMin >> 16;

        pe.patchSection(0x00022254, &hi, sizeof(WORD));
        pe.patchSection(0x0002225C, &lo, sizeof(WORD));
    }
    */
    pe.saveToFile(dstFile.c_str());
}
