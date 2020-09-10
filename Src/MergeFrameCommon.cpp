/** 
 * @file  MergeFrameCommon.cpp
 *
 * @brief Implementation file for CMergeFrameCommon
 *
 */
#include "StdAfx.h"
#include "MergeFrameCommon.h"
#include "OptionsDef.h"
#include "OptionsMgr.h"
#include "utils/DpiAware.h"
#include "Merge.h"
#include <../src/mfc/afximpl.h>

IMPLEMENT_DYNCREATE(CMergeFrameCommon, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CMergeFrameCommon, DpiAware::PerMonitorDpiAwareCWnd<CMDIChildWnd>)
	//{{AFX_MSG_MAP(CMergeFrameCommon)
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
	ON_WM_MDIACTIVATE()
//	ON_MESSAGE(WM_GETICON, OnGetIcon)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CMergeFrameCommon::CMergeFrameCommon(int nIdenticalIcon, int nDifferentIcon)
	: m_hIdentical(nIdenticalIcon < 0 ? nullptr : AfxGetApp()->LoadIcon(nIdenticalIcon))
	, m_hDifferent(nDifferentIcon < 0 ? nullptr : AfxGetApp()->LoadIcon(nDifferentIcon))
	, m_bActivated(false)
	, m_nLastSplitPos{0}
{
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WMU_CHILDFRAMEADDED, 0, reinterpret_cast<LPARAM>(this));
}

CMergeFrameCommon::~CMergeFrameCommon()
{
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WMU_CHILDFRAMEREMOVED, 0, reinterpret_cast<LPARAM>(this));
}

BOOL CMergeFrameCommon::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CMDIFrameWnd* pParentWnd, CCreateContext* pContext)
{
	if (!__super::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, pContext))
		return FALSE;
	UpdateDpi();
	return TRUE;
}

void CMergeFrameCommon::ActivateFrame(int nCmdShow)
{
	if (!m_bActivated) 
	{
		m_bActivated = true;

		// get the active child frame, and a flag whether it is maximized
		BOOL bMaximized = FALSE;
		CMDIChildWnd * oldActiveFrame = GetMDIFrame()->MDIGetActive(&bMaximized);
		if (oldActiveFrame == nullptr)
			// for the first frame, get the restored/maximized state from the registry
			bMaximized = GetOptionsMgr()->GetBool(OPT_ACTIVE_FRAME_MAX);
		if (bMaximized)
			nCmdShow = SW_SHOWMAXIMIZED;
		else
			nCmdShow = SW_SHOWNORMAL;
	}

	__super::ActivateFrame(nCmdShow);
}

void CMergeFrameCommon::SaveWindowState()
{
	// If we are active, save the restored/maximized state
	// If we are not, do nothing and let the active frame do the job.
 	if (GetParentFrame()->GetActiveFrame() == this)
	{
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(&wp);
		GetOptionsMgr()->SaveOption(OPT_ACTIVE_FRAME_MAX, (wp.showCmd == SW_MAXIMIZE));
	}
}

void CMergeFrameCommon::RemoveBarBorder()
{
	afxData.cxBorder2 = 0;
	afxData.cyBorder2 = 0;
	for (int i = 0; i < 4; ++i)
	{
		CControlBar* pBar = GetControlBar(i + AFX_IDW_DOCKBAR_TOP);
		pBar->SetBarStyle(pBar->GetBarStyle() & ~(CBRS_BORDER_ANY | CBRS_BORDER_3D));
	}
}

/**
 * @brief Reflect comparison result in window's icon.
 * @param nResult [in] Last comparison result which the application returns.
 */
void CMergeFrameCommon::SetLastCompareResult(int nResult)
{
	HICON hCurrent = GetIcon(FALSE);
	HICON hReplace = (nResult == 0) ? m_hIdentical : m_hDifferent;

	if (hCurrent != hReplace)
	{
		SetIcon(hReplace, TRUE);

		AfxGetMainWnd()->SetTimer(IDT_UPDATEMAINMENU, 500, nullptr);
	}

	theApp.SetLastCompareResult(nResult);
}

void CMergeFrameCommon::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	__super::OnGetMinMaxInfo(lpMMI);
	if (IsDifferentDpiFromSystemDpi())
	{
		CRect rc;
		CFrameWnd* pFrameWnd = GetParentFrame();
		pFrameWnd->GetClientRect(rc);
		AdjustWindowRectEx(&rc, GetStyle(), FALSE, GetExStyle());
		lpMMI->ptMaxPosition.x = rc.left;
		lpMMI->ptMaxPosition.y = rc.top;
		lpMMI->ptMaxSize.x = rc.right - rc.left;
		lpMMI->ptMaxSize.y = rc.bottom - rc.top;
	}
	// [Fix for MFC 8.0 MDI Maximizing Child Window bug on Vista]
	// https://groups.google.com/forum/#!topic/microsoft.public.vc.mfc/iajCdW5DzTM
#if _MFC_VER >= 0x0800
	lpMMI->ptMaxTrackSize.x = max(lpMMI->ptMaxTrackSize.x, lpMMI->ptMaxSize.x);
	lpMMI->ptMaxTrackSize.y = max(lpMMI->ptMaxTrackSize.y, lpMMI->ptMaxSize.y);
#endif
}

void CMergeFrameCommon::OnDestroy()
{
	// https://stackoverflow.com/questions/35553955/getting-rid-of-3d-look-of-mdi-frame-window
	CFrameWnd::OnDestroy();
}

void CMergeFrameCommon::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	// call the base class to let standard processing switch to
	// the top-level menu associated with this window
	__super::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

	if (bActivate)
		::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WMU_CHILDFRAMEACTIVATED, 0, reinterpret_cast<LPARAM>(this));
}

/*
LRESULT CMergeFrameCommon::OnGetIcon(WPARAM wParam, LPARAM lParam)
{
	return 0;
}
*/
