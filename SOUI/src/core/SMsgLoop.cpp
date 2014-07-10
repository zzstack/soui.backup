#include "souistd.h"
#include "core/SMsgLoop.h"

namespace SOUI
{
    SMessageLoop* SMessageLoop::s_pCurMsgLoop =0;


    int SMessageLoop::Run()
    {
        BOOL bDoIdle = TRUE;
        int nIdleCount = 0;
        BOOL bRet;

        for(;;)
        {
            while(bDoIdle && !::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE))
            {
                if(!OnIdle(nIdleCount++))
                    bDoIdle = FALSE;
            }

            bRet = ::GetMessage(&m_msg, NULL, 0, 0);

            if(bRet == -1)
            {
                STRACE(_T("::GetMessage returned -1 (error)"));
                continue;   // error, don't process
            }
            else if(!bRet)
            {
                STRACE(_T("SMessageLoop::Run - exiting"));
                break;   // WM_QUIT, exit message loop
            }

            if(!PreTranslateMessage(&m_msg))
            {
                ::TranslateMessage(&m_msg);
                ::DispatchMessage(&m_msg);
            }

            if(IsIdleMessage(&m_msg))
            {
                bDoIdle = TRUE;
                nIdleCount = 0;
            }
        }

        return (int)m_msg.wParam;
    }

    BOOL SMessageLoop::OnIdle( int /*nIdleCount*/ )
    {
        for(size_t i = 0; i < m_aIdleHandler.GetCount(); i++)
        {
            IIdleHandler* pIdleHandler = m_aIdleHandler[i];
            if(pIdleHandler != NULL)
                pIdleHandler->OnIdle();
        }
        return FALSE;   // don't continue
    }

    BOOL SMessageLoop::PreTranslateMessage( MSG* pMsg )
    {
        // loop backwards
        for(int i = (int)m_aMsgFilter.GetCount() - 1; i >= 0; i--)
        {
            IMessageFilter* pMessageFilter = m_aMsgFilter[i];
            if(pMessageFilter != NULL && pMessageFilter->PreTranslateMessage(pMsg))
                return TRUE;
        }
        return FALSE;   // not translated
    }

    BOOL SMessageLoop::IsIdleMessage( MSG* pMsg )
    {
        // These messages should NOT cause idle processing
        switch(pMsg->message)
        {
        case WM_MOUSEMOVE:
        case WM_NCMOUSEMOVE:
        case WM_PAINT:
        case WM_SYSTIMER:	// WM_SYSTIMER (caret blink)
            return FALSE;
        }

        return TRUE;
    }

    BOOL SMessageLoop::RemoveIdleHandler( IIdleHandler* pIdleHandler )
    {
        return RemoveElementFromArray(m_aIdleHandler,pIdleHandler);
    }

    BOOL SMessageLoop::AddIdleHandler( IIdleHandler* pIdleHandler )
    {
        return m_aIdleHandler.Add(pIdleHandler);
    }

    BOOL SMessageLoop::RemoveMessageFilter( IMessageFilter* pMessageFilter )
    {
        return RemoveElementFromArray(m_aMsgFilter,pMessageFilter);
    }

    BOOL SMessageLoop::AddMessageFilter( IMessageFilter* pMessageFilter )
    {
        return m_aMsgFilter.Add(pMessageFilter);
    }

    SMessageLoop * SMessageLoop::GetCurMsgLoop()
    {
        return s_pCurMsgLoop;
    }

}//end of namespace SOUI