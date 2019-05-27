#pragma once
#include "stdafx.h"
#include "medianav.h"
#include <map>
#include <vector>
namespace MediaNav
{
    class AccMeter
    {
    public:
        typedef std::map<int, int> TAccMeasureResult;
        typedef std::vector<TAccMeasureResult> TAccMeasureResults;
        typedef std::vector<int> TAccMeasureEdges;

    public:
        AccMeter();
        virtual ~AccMeter();
        void reset();
        TAccMeasureResults getResults() const;
        TAccMeasureResult getLastResult() const;
        bool startMeasure(const TAccMeasureEdges& accMeasureEdges);
        void stopMeasure();
        bool isMeasuring() const;
    private:
        bool measureTick();
        static void checkThread(LPVOID pvParams);
        static DWORD GetThreadTick(FILETIME* a, FILETIME* b);
    private:
        bool    m_threadsCreated;
        HANDLE  m_hCheckThread;
        DWORD   m_dwStartTime;
        int     m_prevSpeed;
        HANDLE  m_hExitEvent;

        TAccMeasureResult m_lastResult;
        TAccMeasureResults m_results;
        TAccMeasureEdges m_measureEdges;
        
        MediaNav::CSharedMemory m_micomInfo;
        mutable Utils::CLock m_resultsLock;
    };
};

