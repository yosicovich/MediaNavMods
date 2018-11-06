#include <windows.h>

#include "SecureConnection.h"

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
CSecureConnection::CSecureConnection() :
m_hInstDivXLibrary(NULL),
m_pSecureConnection(NULL),
m_bConnected(FALSE),
m_bRequireSecureConnection(FALSE)
{
	UnloadDivXInterface();
	LoadDivXInterface();

	if (m_fnGetRandomBytes)
	{
		if ((*m_fnGetRandomBytes)(m_pbMySeed, RANDOM_SEED_LENGTH))
		{
			RETAILMSG(1, (TEXT("CSecureConnection: Failed to get random bytes\r\n")));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
CSecureConnection::~CSecureConnection()
{
	UnloadDivXInterface();
}

int CSecureConnection::LoadDivXInterface()
{
	if (m_hInstDivXLibrary)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("CSecureConnection: Library already loaded!!!!\r\n")));
		return -1;
	}

	m_hInstDivXLibrary = LoadLibrary(TEXT("DivXInterface.dll"));

	if (m_hInstDivXLibrary)
	{
		m_fnEncryptMessage = (fnEncryptMessage_t) GetProcAddress(m_hInstDivXLibrary, TEXT("EncryptMessage"));
		if (NULL == m_fnEncryptMessage)
		{
			DWORD dwError = GetLastError();

			DEBUGMSG(ZONE_ERROR, (TEXT("CDivXInterface: FAILED TO GET FUNCTION EncryptMessage %d\r\n"), dwError));
		}

		m_fnDecryptMessage = (fnDecryptMessage_t) GetProcAddress(m_hInstDivXLibrary, TEXT("DecryptMessage"));
		if (NULL == m_fnDecryptMessage)
		{
			DWORD dwError = GetLastError();

			DEBUGMSG(ZONE_ERROR, (TEXT("CDivXInterface: FAILED TO GET FUNCTION DecryptMessage %d\r\n"), dwError));
		}

		m_fnGetRandomBytes = (fnGetRandomBytes_t) GetProcAddress(m_hInstDivXLibrary, TEXT("GetRandomBytes"));
		if (NULL == m_fnGetRandomBytes)
		{
			DWORD dwError = GetLastError();

			DEBUGMSG(ZONE_ERROR, (TEXT("CDivXInterface: FAILED TO GET FUNCTION GetRandomBytes %d\r\n"), dwError));
		}
		
	}
	else
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("CSecureConnection: FAILED TO LOAD DIVX INTERFACE LIBRRARY\r\n")));
	}

	return 0;
}

int CSecureConnection::UnloadDivXInterface()
{
	if (m_hInstDivXLibrary)
	{
		FreeLibrary(m_hInstDivXLibrary);
		m_hInstDivXLibrary = NULL;
	}

	m_fnEncryptMessage = NULL;
	m_fnDecryptMessage = NULL;
	m_fnGetRandomBytes = NULL;

	return 0;
}

HRESULT CSecureConnection::MakeSecureConnection(IPin *pOtherPin)
{
	PIN_INFO	PinInfo;
	HRESULT		hResult;

	if (NULL == pOtherPin)
	{
		return E_FAIL;
	}

	if (NULL == m_hInstDivXLibrary)
	{
		return S_OK;
	}

	pOtherPin->QueryPinInfo(&PinInfo);

	if (NULL == PinInfo.pFilter)
	{
		// No filter associated with this pin?
		return S_OK;
	}

	hResult = pOtherPin->QueryInterface(IID_SecureConnection, reinterpret_cast<LPVOID*>(&m_pSecureConnection));
	
	PinInfo.pFilter->Release();

	if (FAILED(hResult))
	{
		// The secure interface isn't there so no secure connection required.
		return S_OK;
	}

	if (m_pSecureConnection->GetProtectionRequired())
	{
		PBYTE pbMessage = new BYTE[MAX_MESSAGE_LENGTH];
		DWORD	dwMessageLength = MAX_MESSAGE_LENGTH;

		SetProtectionRequired(TRUE);

		m_pSecureConnection->ExchangeSeeds(m_pbOtherSeed, RANDOM_SEED_LENGTH, m_pbMySeed, RANDOM_SEED_LENGTH);

		PrepareMessage(m_pbMySeed, RANDOM_SEED_LENGTH, m_pbOtherSeed, RANDOM_SEED_LENGTH, NULL, 0, pbMessage, &dwMessageLength);

		if (m_fnEncryptMessage)
		{
			if ((*m_fnEncryptMessage)(0, TRUE, pbMessage, &dwMessageLength, 256))
			{
				RETAILMSG(1, (TEXT("Failed to encrypt message\r\n")));
			}
		}

		m_pSecureConnection->CompleteSecureConnection(pbMessage, dwMessageLength);

		delete pbMessage;
	}
	else
	{
		SetProtectionRequired(FALSE);
	}

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CSecureConnection::CompleteSecureConnection(PBYTE pbMessage, DWORD dwMessageLength)
{
	if (NULL == pbMessage)
	{
		return E_FAIL;
	}

	if (m_fnDecryptMessage)
	{
		if ((*m_fnDecryptMessage)(0, TRUE, pbMessage, &dwMessageLength))
		{
			RETAILMSG(1, (TEXT("Failed to decrypt message\r\n")));
		}

		ValidateMessage(m_pbOtherSeed, RANDOM_SEED_LENGTH, m_pbMySeed, RANDOM_SEED_LENGTH, NULL, 0, pbMessage, dwMessageLength);

		m_bConnected = TRUE;
	}

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CSecureConnection::BreakSecureConnection()
{
	HRESULT hResult = S_OK;

	if (m_pSecureConnection)
	{
		hResult = m_pSecureConnection->BreakSecureConnection();
	}

	return hResult;

}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CSecureConnection::ExchangeSeeds(PBYTE pbMySeed, DWORD dwMySeedLength, PBYTE pbOtherSeed, DWORD dwOtherSeedLength)
{
	if ((RANDOM_SEED_LENGTH > dwMySeedLength) || (RANDOM_SEED_LENGTH > dwOtherSeedLength) ||
		 (NULL == pbMySeed) || (NULL == pbOtherSeed) )
	{
		return E_FAIL;
	}

	memcpy(pbMySeed, m_pbMySeed, RANDOM_SEED_LENGTH);
	memcpy(m_pbOtherSeed, pbOtherSeed, RANDOM_SEED_LENGTH);

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int CSecureConnection::CalculateIndex(PBYTE pbData, DWORD dwDataLen)
{
	int nIndex = 0;

	if (NULL == pbData)
	{
		return -1;
	}

	for (DWORD dwCurrentByte = 0 ; dwCurrentByte < dwDataLen ; dwCurrentByte++)
	{
		nIndex += pbData[dwCurrentByte];
	}

	return nIndex;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int CSecureConnection::PrepareMessage(PBYTE pbData1, DWORD dwData1Len, PBYTE pbData2, DWORD dwData2Len, PBYTE pbData3, DWORD dwData3Len, PBYTE pbMessage, DWORD *pdwMessageLen)
{
	int		nIndex = CalculateIndex(pbData1, dwData1Len);
	DWORD	dwBytesLeft = *pdwMessageLen;
	PBYTE	pbCurrentPosition = pbMessage;

	if ((dwData1Len < dwBytesLeft) && (dwData1Len > 0) )
	{
		if (pbData1)
		{
			dwBytesLeft -= dwData1Len;
			memcpy(pbCurrentPosition, pbData1, dwData1Len);
			pbCurrentPosition += dwData1Len;
		}
	}

	if ((dwData3Len < dwBytesLeft) && (dwData3Len > 0) )
	{
		if (pbData3)
		{
			dwBytesLeft -= dwData3Len;
			memcpy(pbCurrentPosition, pbData3, dwData1Len);
			pbCurrentPosition += dwData3Len;
		}
	}

	if ((dwData2Len < dwBytesLeft) && (dwData2Len > 0) )
	{
		if (pbData2)
		{
			dwBytesLeft -= dwData2Len;
			memcpy(pbCurrentPosition, pbData2, dwData1Len);
			pbCurrentPosition += dwData2Len;
		}
	}

	*pdwMessageLen -= dwBytesLeft;


	return 0;
}
///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int CSecureConnection::CompareBytes(PBYTE pbData1, DWORD dwData1Len, PBYTE pbData2, DWORD dwData2Len)
{
	DWORD	dwCurrentByte = 0;
	DWORD	dwLastByte;
	int		nDifference;

	if ( (NULL == pbData1) || ( NULL == pbData2) )
	{
		return -1;
	}

	if (dwData1Len > dwData2Len)
	{
		dwLastByte = dwData1Len;
	}
	else
	{
		dwLastByte = dwData2Len;
	}

	while (dwCurrentByte < dwLastByte)
	{
		nDifference = pbData1[dwCurrentByte] - pbData2[dwCurrentByte];

		if (nDifference)
			break;

		dwCurrentByte++;
	}

	return nDifference;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int CSecureConnection::ValidateMessage(PBYTE pbData1, DWORD dwData1Len, PBYTE pbData2, DWORD dwData2Len, PBYTE pbData3, DWORD dwData3Len, PBYTE pbMessage, DWORD dwMessageLen)
{
	DWORD	dwValidMessageLen = MAX_MESSAGE_LENGTH;
	BYTE	pbValidMessage[MAX_MESSAGE_LENGTH];

	int nIndex = CalculateIndex(pbData1, dwData1Len);

	// Determine what the message should be
	PrepareMessage(pbData1, dwData1Len, pbData2, dwData2Len, pbData3, dwData3Len, pbValidMessage, &dwValidMessageLen);

	// Now decrypt the message and compare it with what we came up with

	if (CompareBytes(pbMessage, dwMessageLen, pbValidMessage, dwValidMessageLen))
	{
		return -1;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
BOOL CSecureConnection::ConnectionOk()
{
	if (m_bRequireSecureConnection)
	{
		if (FALSE == m_bConnected)
		{
			RETAILMSG(1, (TEXT("Secure connection required, but not established!\r\n")));
			return FALSE;
		}
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP
CSecureConnection::QueryInterface(REFIID riid,void **ppv)
{
    CheckPointer(ppv, E_POINTER);
    
    /* Do we have this interface */
    
    if (riid == IID_SecureConnection || riid == IID_IUnknown) {
        return GetInterface((ISecureConnection *) this, ppv);
    } else {
        return E_NOINTERFACE;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG)
CSecureConnection::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG)
CSecureConnection::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0) {
        delete this;
    }
    return cRef;
}