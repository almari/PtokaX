/*
 * PtokaX - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2002-2005  Ptaczek, Ptaczek at PtokaX dot org
 * Copyright (C) 2004-2010  Petr Kozelka, PPK at PtokaX dot org

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//---------------------------------------------------------------------------
#include "../core/stdinc.h"
//---------------------------------------------------------------------------
#include "MainWindowPage.h"
//---------------------------------------------------------------------------
#ifdef _WIN32
	#pragma hdrstop
#endif
//---------------------------------------------------------------------------
static ATOM atomMainWindowPage = 0;
//---------------------------------------------------------------------------

MainWindowPage::MainWindowPage() {
    m_hWnd = NULL;
}
//---------------------------------------------------------------------------

LRESULT CALLBACK MainWindowPage::StaticMainWindowPageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MainWindowPage * pMainWindowPage = (MainWindowPage *)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if(pMainWindowPage == NULL) {
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return pMainWindowPage->MainWindowPageProc(uMsg, wParam, lParam);
}
//---------------------------------------------------------------------------

void MainWindowPage::CreateHWND(HWND hOwner) {
    if(atomMainWindowPage == 0) {
        WNDCLASSEX m_wc;
        memset(&m_wc, 0, sizeof(WNDCLASSEX));
        m_wc.cbSize = sizeof(WNDCLASSEX);
        m_wc.lpfnWndProc = ::DefWindowProc;
        m_wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
        m_wc.lpszClassName = "PtokaX_MainWindowPage";
        m_wc.hInstance = g_hInstance;
        m_wc.hCursor = ::LoadCursor(m_wc.hInstance, IDC_ARROW);
        m_wc.style = CS_HREDRAW | CS_VREDRAW;

        atomMainWindowPage = ::RegisterClassEx(&m_wc);
    }

    RECT rcMain;
    ::GetClientRect(hOwner, &rcMain);

    m_hWnd = ::CreateWindowEx(WS_EX_CONTROLPARENT, MAKEINTATOM(atomMainWindowPage), "", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 22, rcMain.right, rcMain.bottom-22, hOwner, NULL, g_hInstance, NULL);

    if(m_hWnd != NULL) {
        ::SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
        ::SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)StaticMainWindowPageProc);
    }
}
//---------------------------------------------------------------------------