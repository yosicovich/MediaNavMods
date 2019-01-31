//
// AviDemultiplexor.h

#pragma once

#include "Avi.h"
#include "ElemType.h"
#pragma once

#include "DShowDemuxFilter.h"
//
// Partially derived from GDCL sample MPEG-4 parser filter
// available in source form at http://www.gdcl.co.uk/articles
//

// Declaration of AVI Demultiplexor Filter
//
// The filter has an IAsyncReader input pin and creates an output pin
// for each valid stream in the input pin.

class DECLSPEC_UUID("D24C840C-C469-4368-A363-0913B44AEF5C") // avidmx
AviDemultiplexor: public DShowDemultiplexor
{
public:
    // constructor method used by class factory
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT* phr);

protected:
    virtual MoviePtr createMovie(const AtomReaderPtr& pRoot);

private:
    // construct only via class factory
    AviDemultiplexor(LPUNKNOWN pUnk, HRESULT* phr);
};



