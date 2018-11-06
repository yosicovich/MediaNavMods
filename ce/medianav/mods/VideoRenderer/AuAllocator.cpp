/***********************************************************************
This product is the confidential property of NetLogic Microsystems Inc. 
(“NetLogic”), is provided under a non-disclosure agreement or license 
agreement, and is protected under applicable copyright, patent, and trade 
secret laws. 

Unauthorized use, reproduction, distribution or other dissemination without 
the prior written authorization from NetLogic is strictly prohibited.  

NetLogic disclaims all warranties of any nature, express or implied, 
including, without limitation, the warranties of fitness for a particular 
purpose, merchantability and/or non-infringement of third party rights. 

NetLogic assumes no liability for any error or omissions in this PRODUCT, 
or for the use of this PRODUCT. In no event shall NetLogic be liable to 
any other party for any special, PUNITIVE, incidental or consequential 
damages, whether based on breach of contract, tort, product liability, 
infringement of intellectual property rights or otherwise. NetLogic reserves 
the right to make changes to, or discontinue, its products At any time. 

Distribution of the product herein does not convey a license or any other right
in any patent, trademark, or other intellectual property of NetLogic.

Use of the product shall serve as acceptance of these terms and conditions.  If
you do not accept these terms, you should return or destroy the product and any 
other accompanying information immediately.

Copyright, 2009-20010, NetLogic Microsystems, Inc. All rights reserved.   
***************************_ NetLogic_3_******************************/
#include <streams.h>
#include "dshow\AuMedia.h"
#include "AuAllocator.h"
#include "os_api.h"

#define FUNC	1
#define ALLOC	1
#define ERR		1

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////	AuMediaSample
////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
CAuMediaSample::CAuMediaSample(TCHAR* pName, CBaseAllocator* pAllocator, HRESULT* phr, LPBYTE pBuffer, LONG length)
: CMediaSample(pName, pAllocator, phr, pBuffer, length)
{

}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAuMediaSample::GetPhysPointer(BYTE** ppBuffer)
{
	CheckPointer(ppBuffer, E_INVALIDARG);

	if (m_pMemPool == NULL)
		return  VFW_E_BUFFER_NOTSET;

	*ppBuffer = (BYTE *)m_pMemPool->pPhysical;

	
	return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAuMediaSample::GetVirtPointer(BYTE** ppBuffer)
{
	CheckPointer(ppBuffer, E_INVALIDARG);

	if (m_pMemPool == NULL)
		return  VFW_E_BUFFER_NOTSET;

	*ppBuffer = (BYTE *)m_pMemPool->pVirtual;
	
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAuMediaSample::QueryInterface(REFIID riid, void **ppv)
{
	CheckPointer(ppv, E_INVALIDARG);
	HRESULT hr = S_OK;



	if (IsEqualIID(riid, IID_AuMediaSample)) 
	{
		*ppv = static_cast<IAuMediaSample*>(this);
	} 
	else 
	{
		hr = CMediaSample::QueryInterface(riid,ppv);
	}

	return hr;

}

STDMETHODIMP_(ULONG)
CAuMediaSample::AddRef()
{
    return CMediaSample::AddRef();
}


STDMETHODIMP_(ULONG)
CAuMediaSample::Release()
{
    return CMediaSample::Release();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////	AuAllocator
////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CCritSec CAuAllocator::m_csMempool;

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
CAuAllocator::CAuAllocator(CBaseFilter *pFilter, TCHAR *pName, HRESULT *phr) :
CBaseAllocator(pName, NULL, phr, TRUE),
m_pFilter(pFilter)
{
	OS_Print((FUNC | ALLOC), "Creating CAuAllocator\r\n");
	m_hMem = mem_open_driver();

	*phr = S_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
CAuAllocator::~CAuAllocator()
{
	OS_Print((FUNC | ALLOC), "CAuAllocator destroyed\r\n");

	mem_close_driver(m_hMem);
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAuAllocator::Alloc(void)
{
	CAutoLock		lck(this);
	HRESULT			hr;
	CAuMediaSample	*pNewSample;

	OS_Print((FUNC | ALLOC), "CAuAllocator Alloc\r\n");
	RETAILMSG(1, (TEXT("CAuAllocator Alloc\r\n")));

	// TODO:
	// Remove single mempool ioctl.  Do one for each sample.
	
	// 1. Call the base class implementation, to determine whether the memory truly needs allocating.
	hr = CBaseAllocator::Alloc();

	if(NOERROR == hr) 
	{
		ULONG BufferSize;
		ULONG Remainder;

		// 2. Allocate memory.
		BufferSize = (m_lSize + m_lPrefix);

		Remainder = BufferSize % m_lAlignment;
		if (Remainder)
		{
			BufferSize += m_lAlignment - Remainder;
		}
	
		RETAILMSG(1, (TEXT("CAuAllocator::Alloc: m_lCount=%d, BufferSize=%d\r\n"), m_lCount, BufferSize));

		///////////////////////////////////////////////////////////////////////
		// Lock here and do not unlock until we've deallocated this.  This 
		// protects us if we have multiple renderers created at once during 
		// playlists.
		///////////////////////////////////////////////////////////////////////
		m_csMempool.Lock();
		
		for(LONG count = 0; count < m_lCount; count++)
		{
			// 3. Create  CMediaSample objects that contain chunks of memory from step 2.
			OS_Print((FUNC | ALLOC), "Creating a CAuMediaSample... %d\r\n", count);
			
			pNewSample = new CAuMediaSample(NAME("AuAllocator Sample"), this, &hr, NULL, 0);
			pNewSample->m_pMemPool = mem_alloc(m_hMem, BufferSize, REGION_ITE, 0);
			if (NULL == pNewSample->m_pMemPool)
			{
				RETAILMSG(1, (TEXT("FAILED TO ALLOCATE YUV BUFFERS\r\n")));
				Free();
				return E_FAIL;
			}
			pNewSample->SetPointer((BYTE *)pNewSample->m_pMemPool->pVirtual, BufferSize);

			OS_Print((FUNC | ALLOC), "pNewSample->m_pMemPool->pPhysical = 0x%x\r\n", pNewSample->m_pMemPool->pPhysical);

			// 4. Add each CMediaSample object to the list of free samples ( CBaseAllocator::m_lFree).
			m_lFree.Add(pNewSample);
		}

		m_lAllocated = m_lCount;
	}
	else
	{
		OS_Print((FUNC | ALLOC), "Allocating wasn't possible -- CAuAllocator\r\n");
	}

	m_bChanged = FALSE;
	return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
void CAuAllocator::Free(void)
{
	CAuMediaSample	*pSample;
	LONG			count;

	OS_Print((FUNC | ALLOC), "CAuAllocator Free (%d)\r\n", m_lCount);

	for(count = 0; count < m_lCount; count++)
	{
		pSample = (CAuMediaSample *)m_lFree.RemoveHead();
		if (pSample != NULL)
		{
			if(pSample->m_pMemPool)
					mem_free(m_hMem, pSample->m_pMemPool);

			delete pSample;
		}
		else
		{
			OS_Print(ERR, "CAuMediaSample allocated doesn't match number freed!!\r\n");
			break;
		}
	}

	m_csMempool.Unlock();

	m_lAllocated = m_lAllocated - count;

	// We have to do this because on Stop memory is freed, but they never SetProperties again? 
	m_bChanged = TRUE;
  
	return;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAuAllocator::SetProperties(ALLOCATOR_PROPERTIES *pRequest, 
									 ALLOCATOR_PROPERTIES *pActual)
{
	CheckPointer(pRequest, E_POINTER);
    CheckPointer(pActual, E_POINTER);
    ValidateReadWritePtr(pActual, sizeof(ALLOCATOR_PROPERTIES));
    CAutoLock cObjectLock(this);

	OS_Print((FUNC | ALLOC), "CAuAllocator SetProperties\r\n");

	ZeroMemory(pActual, sizeof(ALLOCATOR_PROPERTIES));

	ASSERT(pRequest->cbBuffer > 0);

    if( pRequest->cbBuffer == 0 )
    {
        OS_Print(ERR, "The number of buffers requested is null\r\n");
        return E_INVALIDARG;
    }
/*
	// Check the alignment requested is 32 bit aligned 
    if (pRequest->cbAlign % 4) {
        OS_Print(ERR, "Alignment requested was 0x%x, not 32 bit aligned\r\n", pRequest->cbAlign);
        return VFW_E_BADALIGN;
    }
*/

	// Can't do this if already committed, there is an argument that says we
    // should not reject the SetProperties call if there are buffers still
    // active. However this is called by the source filter, which is the same
    // person who is holding the samples. Therefore it is not unreasonable
    // for them to free all their samples before changing the requirements
    if (m_bCommitted) 
	{
        return VFW_E_ALREADY_COMMITTED;
    }
    
    // Must be no outstanding buffers
    if (m_lAllocated != m_lFree.GetCount()) 
	{
        return VFW_E_BUFFERS_OUTSTANDING;
    }

	m_lCount		= pRequest->cBuffers;		// Number of buffers to provide.
	m_lSize			= pRequest->cbBuffer;		// Size of each buffer.
	m_lAlignment	= pRequest->cbAlign;		// Alignment of each buffer.
	m_lPrefix		= pRequest->cbPrefix;		// Prefix of each buffer.

	// We need at least 2 buffers in case we want to cache frames for resize, etc.. on pause.
	if (m_lCount < RENDERER_MIN_SAMPLES)
	{
		m_lCount = RENDERER_MIN_SAMPLES;
	}

	GetProperties(pActual);

	m_bChanged = TRUE;

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
//	These functions aren't implemented by us.  They're just here so we can see
//	when they are called if we want.
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAuAllocator::Commit()
{
	OS_Print((FUNC | ALLOC), "CAuAllocator Commit\r\n");

	return CBaseAllocator::Commit();
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAuAllocator::Decommit()
{
	OS_Print((FUNC | ALLOC), "CAuAllocator Decommit\r\n");
	
	return CBaseAllocator::Decommit();
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAuAllocator::GetBuffer(IMediaSample **ppBuffer,
								 REFERENCE_TIME *pStartTime,
								 REFERENCE_TIME *pEndTime,
								 DWORD dwFlags)
{
	OS_Print((FUNC | ALLOC), "CAuAllocator GetBuffer\r\n");

	return CBaseAllocator::GetBuffer(ppBuffer, pStartTime, pEndTime, dwFlags);
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAuAllocator::ReleaseBuffer(IMediaSample *pSample)
{
	OS_Print((FUNC | ALLOC), "CAuAllocator ReleaseBuffer\r\n");
	
	return CBaseAllocator::ReleaseBuffer(pSample);
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
void CAuAllocator::NotifyMediaType(CMediaType *pMediaType)
{
    m_pMediaType = pMediaType;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
BYTE* CAuAllocator::GetPhysicalAddress(BYTE *pVirtual)
{
  unsigned long pPhysical = 0;

  OS_Print(1, "BROKEN GETPHYSICALADDRESS\r\n");
#ifdef NO_MAP_VIRTUAL_YUV
  return (BYTE*)pVirtual;
#endif

#ifdef UNDER_CE
  pPhysical = (ULONG)m_pMemPool->pPhysical + ((ULONG)pVirtual - (ULONG)m_pMemPool->pVirtual);
  return (BYTE*)pPhysical;
#else
  // No Physical Address available
  return NULL;
#endif
}