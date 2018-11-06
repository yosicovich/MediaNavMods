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

#ifndef __AUALLOCATOR_H__
#define __AUALLOCATOR_H__

#include "os_api.h"

#include "dshow\AuMedia.h"

#define RENDERER_MIN_SAMPLES 3

class CAuMediaSample : public CMediaSample 
					 , public IAuMediaSample
{
private:
	
public:
	MEM_IOCTL			*m_pMemPool;

	CAuMediaSample(
        TCHAR *pName,
        CBaseAllocator *pAllocator,
        HRESULT *phr,
        LPBYTE pBuffer = NULL,
        LONG length = 0);

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	void SetPhysPointer(BYTE* ptr);
	HRESULT GetPhysPointer(BYTE** ppBuffer);
	HRESULT GetVirtPointer(BYTE** ppBuffer);
};



class CAuAllocator : public CBaseAllocator
{
public:
	CAuAllocator(CBaseFilter *pFilter, TCHAR *pName, HRESULT *phr);
	~CAuAllocator();
	STDMETHODIMP SetProperties(ALLOCATOR_PROPERTIES *pRequest, 
										 ALLOCATOR_PROPERTIES *pActual);
	STDMETHODIMP Commit();
	STDMETHODIMP Decommit();
	STDMETHODIMP GetBuffer(IMediaSample **ppBuffer,
									 REFERENCE_TIME *pStartTime,
									 REFERENCE_TIME *pEndTime,
									 DWORD dwFlags);
	STDMETHODIMP ReleaseBuffer(IMediaSample *pSample);
	void NotifyMediaType(CMediaType *pMediaType);


	//////////////////////////////////////////////////////////////////////////////
	//
	//
	//
	//////////////////////////////////////////////////////////////////////////////
	BYTE* GetPhysicalAddress(BYTE *pVirtual);
  
protected:
	HRESULT Alloc(void);
	void Free(void);

private:
	// TODO:
	HANDLE			m_hMem;
	PMEM_IOCTL		m_pMemPool;
	CBaseFilter		*m_pFilter;
	CMediaType		*m_pMediaType;				// Pointer to the current format
	static CCritSec	m_csMempool;
};

#endif // __AUALLOCATOR_H__