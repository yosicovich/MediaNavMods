#include "accmeter.h"

namespace MediaNav {

    using namespace MediaNav::MicomManager;
    using namespace Utils;

    AccMeter::AccMeter()
        :m_hCheckThread(NULL)
        ,m_dwStartTime(0)
        ,m_micomInfo(cMicomMemMutexName, cMicomMemName, true, 0)
    {
        m_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if(!m_hExitEvent)
            return;
    }

    AccMeter::~AccMeter()
    {
        if(m_hCheckThread)
        {
            stopMeasure();
        }

        if(m_hExitEvent)
        {
            CloseHandle(m_hExitEvent);
        }
    }

    bool AccMeter::startMeasure(const TAccMeasureEdges& accMeasureEdges)
    {
        stopMeasure();

        m_measureEdges = accMeasureEdges;
        
        if(m_measureEdges.empty())
            return false;

        if(!m_hExitEvent)
            return false;
        
        DWORD ThreadID;
        m_hCheckThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AccMeter::checkThread, (LPVOID)this, CREATE_SUSPENDED, &ThreadID);

        if( m_hCheckThread == NULL )
        {
            return false;
        }
        ResetEvent(m_hExitEvent);
        m_dwStartTime = 0;
        ResumeThread(m_hCheckThread);

        return true;
    }

    void AccMeter::stopMeasure()
    {
        if(!m_hExitEvent)
            return;

        SetEvent(m_hExitEvent);

        if(m_hCheckThread)
        {
            WaitForSingleObject(m_hCheckThread, 3000);
            CloseHandle(m_hCheckThread);
            m_hCheckThread = NULL;
        }
    }

    bool AccMeter::isMeasuring() const
    {
        return m_hCheckThread != NULL && WaitForSingleObject(m_hCheckThread, 0) == WAIT_TIMEOUT;
    }

    void AccMeter::reset()
    {
        stopMeasure();
        CLockHolder<CLock> lock(m_resultsLock);
        m_results.clear();
    }

    AccMeter::TAccMeasureResults AccMeter::getResults() const
    {
        CLockHolder<CLock> lock(m_resultsLock);
        return m_results;
    }

    AccMeter::TAccMeasureResult AccMeter::getLastResult() const
    {
        CLockHolder<CLock> lock(m_resultsLock);
        return m_lastResult;
    }

    
#define SPEED_EMU
    bool AccMeter::measureTick()
    {
        static const int cStartDetectEdge = 55;// 2 km/h
        
#ifdef SPEED_EMU
        static int speed;
        if(m_dwStartTime == 0)
            speed = 55;
        else
            speed += 28;
#else
        int speed;
        MicomManagerInfo info;
        m_micomInfo.read(&info, 0, sizeof(info));
        speed = info.m_speedCm_s;
#endif
        
        if(speed < cStartDetectEdge)
            return false;

        if(m_dwStartTime == 0)
        {
            m_dwStartTime = GetTickCount();
            m_prevSpeed = speed;
        }
        
        if(speed < m_prevSpeed)
            return true; // Slow down condition. The measurement is definetely over.

        m_prevSpeed = speed;

        for(TAccMeasureEdges::iterator it = m_measureEdges.begin(); it != m_measureEdges.end(); ++it)
        {
            if(speed >= *it)
            {
                CLockHolder<CLock> lock(m_resultsLock);
                m_lastResult.insert(std::make_pair(*it, GetTickCount() - m_dwStartTime));
                it = m_measureEdges.erase(it);
                if(it == m_measureEdges.end())
                    break;
            }
        }

        if(m_measureEdges.empty())
            return true;

        return false;
    }

    void AccMeter::checkThread(LPVOID pvParams)
    {
        AccMeter* pSelf = reinterpret_cast<AccMeter*>(pvParams);

        while(WaitForSingleObject(pSelf->m_hExitEvent, 100) == WAIT_TIMEOUT )
        {
            if(pSelf->measureTick())
                break;
        }

        {
            CLockHolder<CLock> lock(pSelf->m_resultsLock);
            pSelf->m_results.push_back(pSelf->m_lastResult);
        }
    }
}; //namespace Medianav
