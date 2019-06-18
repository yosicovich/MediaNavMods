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
void patchNavitel(const std::string& srcFile, const std::string& dstFile);


int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("Usage: import-patcher.exe <source path> <destination path>\r\n");
        return -1;
    }

#if 1
	patchAppMain(std::string(argv[1]) + "\\AppMain.exe", std::string(argv[2]) + "\\AppMain.exe");
    patchMgrUsb(std::string(argv[1]) + "\\MgrUSB.exe", std::string(argv[2]) + "\\MgrUSB.exe");
    //patchRVC(std::string(argv[1]) + "\\RVC.dll", std::string(argv[2]) + "\\RVC.dll");
    patchMicomManager(std::string(argv[1]) + "\\MicomManager.exe", std::string(argv[2]) + "\\MicomManager.exe");
#endif
#if 0
//	patchNavitel(std::string(argv[1]) + "\\MobileNavigator.exe", std::string(argv[2]) + "\\MobileNavigator.exe");
	patchNavitel(std::string(argv[1]) + "\\nngnavi.exe", std::string(argv[2]) + "\\nngnavi.exe");
#endif

    printf("Successfully patched!\r\n");
    return 0;

    PEFile pe(argv[1]);
    
    // Add import
    PE_IMPORT_ENTRIES functions = {
		{"extCheckMediaFilesExtList", 0}
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
	PE_IMPORT_ENTRIES functions = {
		{"extCheckMediaFilesExtList", 0}
    };
    pe.addImport("player_helper.dll", functions);

    // Save import and reload the file for patching
    pe.saveAndReload(dstFile);

    // Hooks to extend file extensions support
    {
        {
            MIPSTableCallWithPadding call(0x0001D398, 0x0001D3BC, pe.getImportFunctionVA("player_helper.dll", "extCheckMediaFilesExtList"));
            pe.patch(call);
        }

        {
            MIPSTableCallWithPadding call(0x00021BF0, 0x00021C14, pe.getImportFunctionVA("player_helper.dll", "extCheckMediaFilesExtList"));
            pe.patch(call);
        }

        {
            MIPSTableCallWithPadding call(0x000492A0, 0x000492CC, pe.getImportFunctionVA("player_helper.dll", "extCheckMediaFilesExtList"));
            pe.patch(call);
        }

        {
            MIPSTableCallWithPadding call(0x0004C01C, 0x0004C040, pe.getImportFunctionVA("player_helper.dll", "extCheckMediaFilesExtList"));
            pe.patch(call);
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

    // GUI test
 /*   {
        BYTE patch = 0xB4;
        pe.patchSection(0x22860, &patch, sizeof(BYTE));
        pe.patchSection(0x00023176, &patch, sizeof(BYTE));
        WORD wPatch = 550;
        pe.patchSection(0x000228F4, &patch, sizeof(WORD));
        

    }*/
    // TEST2
    {
//        BYTE patch = 0x16;
//        pe.patchSection(0x0012F80B, &patch, sizeof(BYTE));

    }

	// DEBUG
#ifdef _DEBUG
	{
		// Disable starup argument checking.
		{
			MIPSPCRelJumpWithPadding call(0x0012A6F4, 0x0012A72C, 0x0012A730, false);
			pe.patch(call);
		}

		{
			wchar_t* langDllRus = L"\\MD\\VTEST\\gui\\DATA\\LangDllRus.dll\0";
			pe.patchSection(0x00151D64, langDllRus, (wcslen(langDllRus) + 1) * 2);
		}

	}
#endif
	// GUI hack
	{
		pe.saveAndReload(dstFile);
		// Add import
		PE_IMPORT_ENTRIES functions = {
//			{"?singleton@?$CGUIFixedEmptyDlg@VCDevMaketDlg@@@@SAPAVCDevMaketDlg@@XZ", 4},
			{"?singleton@CResManagerExt@@SAPAV1@XZ", 2},
			{"?singleton@?$CGUIFixedEmptyDlg@VCVehicleDlg@@@@SAPAVCVehicleDlg@@XZ", 3},
		};
		pe.addImport("appmaingui.dll", functions);
		pe.saveAndReload(dstFile);

		/*{
			MIPSTableCallWithPadding call(0x0001EDE4, 0x0001EE24, pe.getImportFunctionVA("appmaingui.dll", "?singleton@?$CGUIFixedEmptyDlg@VCDevMaketDlg@@@@SAPAVCDevMaketDlg@@XZ"), false);
			pe.patch(call);
		}*/

		{
			MIPSTableCallWithPadding call(0x0001F4C0, 0x0001F500, pe.getImportFunctionVA("appmaingui.dll", "?singleton@CResManagerExt@@SAPAV1@XZ"), false);
			pe.patch(call);
		}

		{
			MIPSTableCallWithPadding call(0x0002593C, 0x000259C0, pe.getImportFunctionVA("appmaingui.dll", "?singleton@?$CGUIFixedEmptyDlg@VCVehicleDlg@@@@SAPAVCVehicleDlg@@XZ"), false);
			pe.patch(call);
		}

		// Always show vehicle menu
		{
			DWORD cmd = 0xAE294A38; // sw  $t1, 0x4A38($s1)
			pe.patchSection(0x0002237C, &cmd, sizeof(DWORD));
			cmd = 0x24080001; //li  $t0, 1
			pe.patchSection(0x00022350, &cmd, sizeof(DWORD));
			//cmd = 0xAE224A30; // sw      $v0, 0x4A30($s1)
			//pe.patchSection(0x00022358, &cmd, sizeof(DWORD));
		}
	}
    pe.saveToFile(dstFile.c_str());
}

void patchMgrUsb(const std::string& srcFile, const std::string& dstFile)
{
    PEFile pe(srcFile.c_str());

    // Add import
	PE_IMPORT_ENTRIES functions = {
		{"extCheckMediaFileMatch", 0}
		,{"extCheckMediaFileMatch2", 0}
    };
    pe.addImport("player_helper.dll", functions);

    // Save import and reload the file for patching
    pe.saveAndReload(dstFile);

    // Hooks to extend file extensions support
    {
        {
            MIPSTableCallWithPadding call(0x000164C0, 0x000164CC, pe.getImportFunctionVA("player_helper.dll", "extCheckMediaFileMatch"));
            pe.patch(call);

            DWORD cmd = 0x02002025; // move $a0, $s0
            pe.patchSection(0x000164CC, &cmd, sizeof(DWORD));
        }

        {
            MIPSTableCallWithPadding call(0x00013C38, 0x00013C48, pe.getImportFunctionVA("player_helper.dll", "extCheckMediaFileMatch2"));
            pe.patch(call);

            DWORD cmd = 0x27A40038; // addiu   $a0, $sp, 0x270+var_238
            pe.patchSection(0x00013C48, &cmd, sizeof(DWORD));
        }
    }

    
    {

        // Fix faulty device removal detection on EC_COMPELTE param2 not null basis
        MIPSNop nopFill(0x00011C5C, 0x00011C64);
        pe.patch(nopFill);
    }


    // Enable full debug output
    /*{
        BYTE patch = 0x00;
        pe.patchSection(0x00023D4A, &patch, sizeof(BYTE));
        pe.patchSection(0x000235C6, &patch, sizeof(BYTE));

    }*/
    
    // Redirect messages to USBTags
    {
        wchar_t* usbTags = L"MgrTag\0";
        // Patch IpcPostMsg table
		pe.patchSection(0x0002F48C, usbTags, (wcslen(usbTags) + 1) * 2);
		// Path SendMessageTimeout
		pe.patchSection(0x0002A5FC, usbTags, (wcslen(usbTags) + 1) * 2);
    }

	// set USBTags mutex name
	{
		wchar_t* usbTags = L"ShmMxMgrUsbTagMgr_";
		pe.patchSection(0x0002C58C, usbTags, wcslen(usbTags) * 2);
	}

	// set USBTags shared mem name
	{
		wchar_t* usbTags = L"ShmFmMgrUsbTagMgr_";
		pe.patchSection(0x0002C794, usbTags, wcslen(usbTags) * 2);
	}
	pe.saveToFile(dstFile.c_str());
}

void patchRVC(const std::string& srcFile, const std::string& dstFile)
{
    PEFile pe(srcFile.c_str());

    // Reduce number of ITE buffers 6->3
    BYTE numBuffers = 3;
    if(numBuffers != 6) // 6 is a default
    {
       
        pe.patchSection(0x1000B724, &numBuffers, sizeof(BYTE));
        pe.patchSection(0x1000B960, &numBuffers, sizeof(BYTE));
        pe.patchSection(0x1000BE04, &numBuffers, sizeof(BYTE));

        BYTE patch = numBuffers * sizeof(DWORD);
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

#ifdef _DEBUG
	// No CodeChecker
	{
		WORD val = 0x1000; // b xxxxx
		pe.patchSection(0x00022006, &val, sizeof(WORD));
	}

#endif

#if 0
	// Volume reset
	{
		BYTE defVolume = 20;
		BYTE tresholdVolume = defVolume + 1;

		pe.patchSection(0x00013140, &tresholdVolume, sizeof(BYTE));
		pe.patchSection(0x0001314C, &defVolume, sizeof(BYTE));

		pe.patchSection(0x00013BE0, &tresholdVolume, sizeof(BYTE));
		pe.patchSection(0x00013BEC, &defVolume, sizeof(BYTE));

		pe.patchSection(0x000123DC, &defVolume, sizeof(BYTE));
		pe.patchSection(0x000123EC, &tresholdVolume, sizeof(BYTE));
		pe.patchSection(0x0001240C, &tresholdVolume, sizeof(BYTE));
		pe.patchSection(0x00012430, &tresholdVolume, sizeof(BYTE));
		pe.patchSection(0x00012458, &tresholdVolume, sizeof(BYTE));
		pe.patchSection(0x00012480, &tresholdVolume, sizeof(BYTE));
	}
#endif

    pe.saveToFile(dstFile.c_str());
}

void patchNavitel(const std::string& srcFile, const std::string& dstFile)
{
	PEFile pe(srcFile.c_str());

	// Add import
	PE_IMPORT_ENTRIES functions = {
		{ "getVersion", 0 }
	};
	pe.addImport("compat.dll", functions);

	pe.saveToFile(dstFile.c_str());
}
