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
#include "SettingPageBans.h"
//---------------------------------------------------------------------------
#include "../core/LanguageManager.h"
#include "../core/SettingManager.h"
#include "../core/utility.h"
//---------------------------------------------------------------------------
#ifdef _WIN32
	#pragma hdrstop
#endif
//---------------------------------------------------------------------------

SettingPageBans::SettingPageBans() {
    bUpdateTempBanRedirAddress = bUpdatePermBanRedirAddress = false;

    memset(&hWndPageItems, 0, (sizeof(hWndPageItems) / sizeof(hWndPageItems[0])) * sizeof(HWND));
}
//---------------------------------------------------------------------------

LRESULT SettingPageBans::SettingPageProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if(uMsg == WM_COMMAND) {
        switch(LOWORD(wParam)) {
            case EDT_DEFAULT_TEMPBAN_TIME:
                if(HIWORD(wParam) == EN_CHANGE) {
					MinMaxCheck((HWND)lParam, 1, 32767);

                    return 0;
                }
            case EDT_ADD_MESSAGE:
            case EDT_TEMP_BAN_REDIR:
            case EDT_PERM_BAN_REDIR:
                if(HIWORD(wParam) == EN_CHANGE) {
                    RemovePipes((HWND)lParam);

                    return 0;
                }

                break;
            case BTN_TEMP_BAN_REDIR_ENABLE:
                if(HIWORD(wParam) == BN_CLICKED) {
                    ::EnableWindow(hWndPageItems[EDT_TEMP_BAN_REDIR],
                        ::SendMessage(hWndPageItems[BTN_TEMP_BAN_REDIR_ENABLE], BM_GETCHECK, 0, 0) == BST_CHECKED ? TRUE : FALSE);
                }
                break;
            case BTN_PERM_BAN_REDIR_ENABLE:
                if(HIWORD(wParam) == BN_CLICKED) {
                    ::EnableWindow(hWndPageItems[EDT_PERM_BAN_REDIR],
                        ::SendMessage(hWndPageItems[BTN_PERM_BAN_REDIR_ENABLE], BM_GETCHECK, 0, 0) == BST_CHECKED ? TRUE : FALSE);
                }
                break;
            case CB_BRUTE_FORCE_PASSWORD_PROTECTION_ACTION:
                if(HIWORD(wParam) == CBN_SELCHANGE) {
                    uint32_t ui32Action = (uint32_t)::SendMessage(hWndPageItems[CB_BRUTE_FORCE_PASSWORD_PROTECTION_ACTION], CB_GETCURSEL, 0, 0);
                    ::EnableWindow(hWndPageItems[LBL_TEMP_BAN_TIME], ui32Action == 2 ? TRUE : FALSE);
                    ::EnableWindow(hWndPageItems[EDT_TEMP_BAN_TIME], ui32Action == 2 ? TRUE : FALSE);
                    ::EnableWindow(hWndPageItems[UD_TEMP_BAN_TIME], ui32Action == 2 ? TRUE : FALSE);
                    ::EnableWindow(hWndPageItems[LBL_TEMP_BAN_TIME_HOURS], ui32Action == 2 ? TRUE : FALSE);
                    ::EnableWindow(hWndPageItems[BTN_REPORT_3X_BAD_PASS], ui32Action == 0 ? FALSE : TRUE);
                }

                break;
            case EDT_TEMP_BAN_TIME:
                if(HIWORD(wParam) == EN_CHANGE) {
                    MinMaxCheck((HWND)lParam, 1, 512);

                    return 0;
                }
        }
    }

	return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}
//------------------------------------------------------------------------------

void SettingPageBans::Save() {
    if(bCreated == false) {
        return;
    }

    LRESULT lResult = ::SendMessage(hWndPageItems[UD_DEFAULT_TEMPBAN_TIME], UDM_GETPOS, 0, 0);
    if(HIWORD(lResult) == 0) {
        SettingManager->SetShort(SETSHORT_DEFAULT_TEMP_BAN_TIME, LOWORD(lResult));
    }

    SettingManager->SetBool(SETBOOL_BAN_MSG_SHOW_IP, ::SendMessage(hWndPageItems[BTN_SHOW_IP], BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false);
    SettingManager->SetBool(SETBOOL_BAN_MSG_SHOW_RANGE, ::SendMessage(hWndPageItems[BTN_SHOW_RANGE], BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false);
    SettingManager->SetBool(SETBOOL_BAN_MSG_SHOW_NICK, ::SendMessage(hWndPageItems[BTN_SHOW_NICK], BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false);
    SettingManager->SetBool(SETBOOL_BAN_MSG_SHOW_REASON, ::SendMessage(hWndPageItems[BTN_SHOW_REASON], BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false);
    SettingManager->SetBool(SETBOOL_BAN_MSG_SHOW_BY, ::SendMessage(hWndPageItems[BTN_SHOW_BY], BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false);

    char buf[257];
    int iLen = ::GetWindowText(hWndPageItems[EDT_ADD_MESSAGE], buf, 257);
    SettingManager->SetText(SETTXT_MSG_TO_ADD_TO_BAN_MSG, buf, iLen);

    SettingManager->SetBool(SETBOOL_TEMP_BAN_REDIR, ::SendMessage(hWndPageItems[BTN_TEMP_BAN_REDIR_ENABLE], BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false);

    iLen = ::GetWindowText(hWndPageItems[EDT_TEMP_BAN_REDIR], buf, 257);

    if((SettingManager->sTexts[SETTXT_TEMP_BAN_REDIR_ADDRESS] == NULL && iLen != 0) ||
        (SettingManager->sTexts[SETTXT_TEMP_BAN_REDIR_ADDRESS] != NULL &&
        strcmp(buf, SettingManager->sTexts[SETTXT_TEMP_BAN_REDIR_ADDRESS]) != NULL)) {
        bUpdateTempBanRedirAddress = true;
    }

    SettingManager->SetText(SETTXT_TEMP_BAN_REDIR_ADDRESS, buf, iLen);

    SettingManager->SetBool(SETBOOL_PERM_BAN_REDIR, ::SendMessage(hWndPageItems[BTN_PERM_BAN_REDIR_ENABLE], BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false);

    iLen = ::GetWindowText(hWndPageItems[EDT_PERM_BAN_REDIR], buf, 257);

    if((SettingManager->sTexts[SETTXT_PERM_BAN_REDIR_ADDRESS] == NULL && iLen != 0) ||
        (SettingManager->sTexts[SETTXT_PERM_BAN_REDIR_ADDRESS] != NULL &&
        strcmp(buf, SettingManager->sTexts[SETTXT_PERM_BAN_REDIR_ADDRESS]) != NULL)) {
        bUpdatePermBanRedirAddress = true;
    }

    SettingManager->SetText(SETTXT_PERM_BAN_REDIR_ADDRESS, buf, iLen);

    SettingManager->SetBool(SETBOOL_ADVANCED_PASS_PROTECTION,
        ::SendMessage(hWndPageItems[BTN_ENABLE_ADVANCED_PASSWORD_PROTECTION], BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false);

    SettingManager->SetShort(SETSHORT_BRUTE_FORCE_PASS_PROTECT_BAN_TYPE, (int16_t)::SendMessage(hWndPageItems[CB_BRUTE_FORCE_PASSWORD_PROTECTION_ACTION], CB_GETCURSEL, 0, 0));

    lResult = ::SendMessage(hWndPageItems[UD_TEMP_BAN_TIME], UDM_GETPOS, 0, 0);
    if(HIWORD(lResult) == 0) {
        SettingManager->SetShort(SETSHORT_BRUTE_FORCE_PASS_PROTECT_TEMP_BAN_TIME, LOWORD(lResult));
    }

    SettingManager->SetBool(SETBOOL_REPORT_3X_BAD_PASS, ::SendMessage(hWndPageItems[BTN_REPORT_3X_BAD_PASS], BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false);
}
//------------------------------------------------------------------------------

void SettingPageBans::GetUpdates(bool & /*bUpdateHubNameWelcome*/, bool & /*bUpdateHubName*/, bool & /*bUpdateTCPPorts*/, bool & /*bUpdateUDPPort*/,
        bool & /*bUpdateAutoReg*/, bool & /*bUpdatedMOTD*/, bool & /*bUpdatedHubSec*/, bool & /*bUpdatedRegOnlyMessage*/, bool & /*bUpdatedShareLimitMessage*/,
        bool & /*bUpdatedSlotsLimitMessage*/, bool & /*bUpdatedHubSlotRatioMessage*/, bool & /*bUpdatedMaxHubsLimitMessage*/, bool & /*bUpdatedNoTagMessage*/,
        bool & /*bUpdatedNickLimitMessage*/, bool & /*bUpdatedBotsSameNick*/, bool & /*bUpdatedBotNick*/, bool & /*bUpdatedBot*/, bool & /*bUpdatedOpChatNick*/,
        bool & /*bUpdatedOpChat*/, bool & /*bUpdatedLanguage*/, bool & /*bUpdatedTextFiles*/, bool & /*bUpdatedRedirectAddress*/, bool & bUpdatedTempBanRedirAddress,
        bool & bUpdatedPermBanRedirAddress, bool & /*bUpdatedSysTray*/, bool & /*bUpdatedScripting*/, bool & /*bUpdatedMinShare*/, bool & /*bUpdatedMaxShare*/) {
    bUpdatedTempBanRedirAddress = bUpdateTempBanRedirAddress;
    bUpdatedPermBanRedirAddress = bUpdatePermBanRedirAddress;
}

//------------------------------------------------------------------------------
bool SettingPageBans::CreateSettingPage(HWND hOwner) {
    CreateHWND(hOwner);
    
    if(bCreated == false) {
        return false;
    }

    hWndPageItems[GB_DEFAULT_TEMPBAN_TIME] = ::CreateWindowEx(WS_EX_TRANSPARENT, WC_BUTTON, LanguageManager->sTexts[LAN_TEMP_BAN_TIME_AFTER_ETC], WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        0, 3, 447, 43, m_hWnd, NULL, g_hInstance, NULL);

    hWndPageItems[EDT_DEFAULT_TEMPBAN_TIME] = ::CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_NUMBER | ES_AUTOHSCROLL |
        ES_RIGHT, 8, 18, 110, 20, m_hWnd, (HMENU)EDT_DEFAULT_TEMPBAN_TIME, g_hInstance, NULL);
    ::SendMessage(hWndPageItems[EDT_DEFAULT_TEMPBAN_TIME], EM_SETLIMITTEXT, 5, 0);

    AddUpDown(hWndPageItems[UD_DEFAULT_TEMPBAN_TIME], 118, 18, 17, 20, (LPARAM)MAKELONG(32767, 1), (WPARAM)hWndPageItems[EDT_DEFAULT_TEMPBAN_TIME],
        (LPARAM)MAKELONG(SettingManager->iShorts[SETSHORT_DEFAULT_TEMP_BAN_TIME], 0));

    hWndPageItems[LBL_DEFAULT_TEMPBAN_TIME_MINUTES] = ::CreateWindowEx(0, WC_STATIC, LanguageManager->sTexts[LAN_MINUTES_LWR], WS_CHILD | WS_VISIBLE | SS_LEFT,
        140, 22, 299, 16, m_hWnd, NULL, g_hInstance, NULL);

    hWndPageItems[GB_BAN_MESSAGE] = ::CreateWindowEx(WS_EX_TRANSPARENT, WC_BUTTON, LanguageManager->sTexts[LAN_BAN_MSG], WS_CHILD | WS_VISIBLE |
        BS_GROUPBOX, 0, 46, 447, 153, m_hWnd, NULL, g_hInstance, NULL);

    hWndPageItems[BTN_SHOW_IP] = ::CreateWindowEx(0, WC_BUTTON, LanguageManager->sTexts[LAN_SHOW_BANNED_IP], WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
        8, 60, 431, 16, m_hWnd, NULL, g_hInstance, NULL);
    ::SendMessage(hWndPageItems[BTN_SHOW_IP], BM_SETCHECK, (SettingManager->bBools[SETBOOL_BAN_MSG_SHOW_IP] == true ? BST_CHECKED : BST_UNCHECKED), 0);

    hWndPageItems[BTN_SHOW_RANGE] = ::CreateWindowEx(0, WC_BUTTON, LanguageManager->sTexts[LAN_SHOW_BANNED_RANGE], WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
        8, 79, 431, 16, m_hWnd, NULL, g_hInstance, NULL);
    ::SendMessage(hWndPageItems[BTN_SHOW_RANGE], BM_SETCHECK, (SettingManager->bBools[SETBOOL_BAN_MSG_SHOW_RANGE] == true ? BST_CHECKED : BST_UNCHECKED), 0);

    hWndPageItems[BTN_SHOW_NICK] = ::CreateWindowEx(0, WC_BUTTON, LanguageManager->sTexts[LAN_SHOW_BANNED_NICK], WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
        8, 98, 431, 16, m_hWnd, NULL, g_hInstance, NULL);
    ::SendMessage(hWndPageItems[BTN_SHOW_NICK], BM_SETCHECK, (SettingManager->bBools[SETBOOL_BAN_MSG_SHOW_NICK] == true ? BST_CHECKED : BST_UNCHECKED), 0);

    hWndPageItems[BTN_SHOW_REASON] = ::CreateWindowEx(0, WC_BUTTON, LanguageManager->sTexts[LAN_SHOW_BAN_REASON], WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
        8, 117, 431, 16, m_hWnd, NULL, g_hInstance, NULL);
    ::SendMessage(hWndPageItems[BTN_SHOW_REASON], BM_SETCHECK, (SettingManager->bBools[SETBOOL_BAN_MSG_SHOW_REASON] == true ? BST_CHECKED : BST_UNCHECKED), 0);

    hWndPageItems[BTN_SHOW_BY] = ::CreateWindowEx(0, WC_BUTTON, LanguageManager->sTexts[LAN_SHOW_BAN_WHO], WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
        8, 136, 431, 16, m_hWnd, NULL, g_hInstance, NULL);
    ::SendMessage(hWndPageItems[BTN_SHOW_BY], BM_SETCHECK, (SettingManager->bBools[SETBOOL_BAN_MSG_SHOW_BY] == true ? BST_CHECKED : BST_UNCHECKED), 0);

    hWndPageItems[GB_ADD_MESSAGE] = ::CreateWindowEx(WS_EX_TRANSPARENT, WC_BUTTON, LanguageManager->sTexts[LAN_BAN_MSG_ADD_MSG], WS_CHILD | WS_VISIBLE |
        BS_GROUPBOX, 5, 153, 437, 41, m_hWnd, NULL, g_hInstance, NULL);

    hWndPageItems[EDT_ADD_MESSAGE] = ::CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, SettingManager->sTexts[SETTXT_MSG_TO_ADD_TO_BAN_MSG], WS_CHILD | WS_VISIBLE |
        WS_TABSTOP | ES_AUTOHSCROLL, 13, 168, 421, 18, m_hWnd, (HMENU)EDT_ADD_MESSAGE, g_hInstance, NULL);
    ::SendMessage(hWndPageItems[EDT_ADD_MESSAGE], EM_SETLIMITTEXT, 256, 0);

    hWndPageItems[GB_TEMP_BAN_REDIR] = ::CreateWindowEx(WS_EX_TRANSPARENT, WC_BUTTON, LanguageManager->sTexts[LAN_TEMP_BAN_REDIR_ADDRESS], WS_CHILD |
        WS_VISIBLE | BS_GROUPBOX, 0, 199, 447, 41, m_hWnd, NULL, g_hInstance, NULL);

    hWndPageItems[BTN_TEMP_BAN_REDIR_ENABLE] = ::CreateWindowEx(0, WC_BUTTON, LanguageManager->sTexts[LAN_ENABLE_W_ARROW], WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
        8, 216, 85, 16, m_hWnd, (HMENU)BTN_TEMP_BAN_REDIR_ENABLE, g_hInstance, NULL);
    ::SendMessage(hWndPageItems[BTN_TEMP_BAN_REDIR_ENABLE], BM_SETCHECK, (SettingManager->bBools[SETBOOL_TEMP_BAN_REDIR] == true ? BST_CHECKED : BST_UNCHECKED), 0);

    hWndPageItems[EDT_TEMP_BAN_REDIR] = ::CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, SettingManager->sTexts[SETTXT_TEMP_BAN_REDIR_ADDRESS], WS_CHILD | WS_VISIBLE |
        WS_TABSTOP | ES_AUTOHSCROLL, 98, 214, 341, 18, m_hWnd, (HMENU)EDT_TEMP_BAN_REDIR, g_hInstance, NULL);
    ::SendMessage(hWndPageItems[EDT_TEMP_BAN_REDIR], EM_SETLIMITTEXT, 256, 0);

    hWndPageItems[GB_PERM_BAN_REDIR] = ::CreateWindowEx(WS_EX_TRANSPARENT, WC_BUTTON, LanguageManager->sTexts[LAN_PERM_BAN_REDIR_ADDRESS], WS_CHILD |
        WS_VISIBLE | BS_GROUPBOX, 0, 240, 447, 41, m_hWnd, NULL, g_hInstance, NULL);

    hWndPageItems[BTN_PERM_BAN_REDIR_ENABLE] = ::CreateWindowEx(0, WC_BUTTON, LanguageManager->sTexts[LAN_ENABLE_W_ARROW], WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
        8, 257, 85, 16, m_hWnd, (HMENU)BTN_PERM_BAN_REDIR_ENABLE, g_hInstance, NULL);
    ::SendMessage(hWndPageItems[BTN_PERM_BAN_REDIR_ENABLE], BM_SETCHECK, (SettingManager->bBools[SETBOOL_PERM_BAN_REDIR] == true ? BST_CHECKED : BST_UNCHECKED), 0);

    hWndPageItems[EDT_PERM_BAN_REDIR] = ::CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, SettingManager->sTexts[SETTXT_PERM_BAN_REDIR_ADDRESS], WS_CHILD | WS_VISIBLE |
        WS_TABSTOP | ES_AUTOHSCROLL, 98, 255, 341, 18, m_hWnd, (HMENU)EDT_PERM_BAN_REDIR, g_hInstance, NULL);
    ::SendMessage(hWndPageItems[EDT_PERM_BAN_REDIR], EM_SETLIMITTEXT, 256, 0);

    hWndPageItems[GB_PASSWORD_PROTECTION] = ::CreateWindowEx(WS_EX_TRANSPARENT, WC_BUTTON, LanguageManager->sTexts[LAN_PASSWORD_PROTECTION],
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 0, 281, 447, 125, m_hWnd, NULL, g_hInstance, NULL);

    hWndPageItems[BTN_ENABLE_ADVANCED_PASSWORD_PROTECTION] = ::CreateWindowEx(0, WC_BUTTON, LanguageManager->sTexts[LAN_ADV_PASS_PRTCTN], WS_CHILD | WS_VISIBLE | WS_TABSTOP |
        BS_AUTOCHECKBOX, 8, 295, 431, 16, m_hWnd, NULL, g_hInstance, NULL);
    ::SendMessage(hWndPageItems[BTN_ENABLE_ADVANCED_PASSWORD_PROTECTION], BM_SETCHECK,
        (SettingManager->bBools[SETBOOL_ADVANCED_PASS_PROTECTION] == true ? BST_CHECKED : BST_UNCHECKED), 0);

    hWndPageItems[GB_BRUTE_FORCE_PASSWORD_PROTECTION_ACTION] = ::CreateWindowEx(WS_EX_TRANSPARENT, WC_BUTTON, LanguageManager->sTexts[LAN_BRUTE_FORCE_PASS_PRTC_ACT],
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 5, 312, 437, 89, m_hWnd, NULL, g_hInstance, NULL);

    hWndPageItems[CB_BRUTE_FORCE_PASSWORD_PROTECTION_ACTION] = ::CreateWindowEx(0, WC_COMBOBOX, "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | CBS_DROPDOWNLIST,
        13, 327, 421, 21, m_hWnd, (HMENU)CB_BRUTE_FORCE_PASSWORD_PROTECTION_ACTION, g_hInstance, NULL);
    ::SendMessage(hWndPageItems[CB_BRUTE_FORCE_PASSWORD_PROTECTION_ACTION], CB_ADDSTRING, 0, (LPARAM)LanguageManager->sTexts[LAN_DISABLED]);
    ::SendMessage(hWndPageItems[CB_BRUTE_FORCE_PASSWORD_PROTECTION_ACTION], CB_ADDSTRING, 0, (LPARAM)LanguageManager->sTexts[LAN_PERM_BAN]);
    ::SendMessage(hWndPageItems[CB_BRUTE_FORCE_PASSWORD_PROTECTION_ACTION], CB_ADDSTRING, 0, (LPARAM)LanguageManager->sTexts[LAN_TEMP_BAN]);
    ::SendMessage(hWndPageItems[CB_BRUTE_FORCE_PASSWORD_PROTECTION_ACTION], CB_SETCURSEL, SettingManager->iShorts[SETSHORT_BRUTE_FORCE_PASS_PROTECT_BAN_TYPE], 0);

    hWndPageItems[LBL_TEMP_BAN_TIME] = ::CreateWindowEx(0, WC_STATIC, LanguageManager->sTexts[LAN_TEMPORARY_BAN_TIME],
        WS_CHILD | WS_VISIBLE | SS_LEFT, 13, 358, 245, 16, m_hWnd, NULL, g_hInstance, NULL);

    hWndPageItems[EDT_TEMP_BAN_TIME] = ::CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_NUMBER | ES_AUTOHSCROLL | ES_RIGHT,
        263, 354, 80, 20, m_hWnd, (HMENU)EDT_TEMP_BAN_TIME, g_hInstance, NULL);
    ::SendMessage(hWndPageItems[EDT_TEMP_BAN_TIME], EM_SETLIMITTEXT, 3, 0);

    AddUpDown(hWndPageItems[UD_TEMP_BAN_TIME], 343, 354, 17, 20, (LPARAM)MAKELONG(512, 1), (WPARAM)hWndPageItems[EDT_TEMP_BAN_TIME],
        (LPARAM)MAKELONG(SettingManager->iShorts[SETSHORT_BRUTE_FORCE_PASS_PROTECT_TEMP_BAN_TIME], 0));

    hWndPageItems[LBL_TEMP_BAN_TIME_HOURS] = ::CreateWindowEx(0, WC_STATIC, LanguageManager->sTexts[LAN_HOURS_LWR],
        WS_CHILD | WS_VISIBLE | SS_LEFT, 365, 358, 69, 16, m_hWnd, NULL, g_hInstance, NULL);

    hWndPageItems[BTN_REPORT_3X_BAD_PASS] = ::CreateWindowEx(0, WC_BUTTON, LanguageManager->sTexts[LAN_REPORT_BAD_PASS], WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
        13, 379, 421, 16, m_hWnd, NULL, g_hInstance, NULL);
    ::SendMessage(hWndPageItems[BTN_REPORT_3X_BAD_PASS], BM_SETCHECK, (SettingManager->bBools[SETBOOL_REPORT_3X_BAD_PASS] == true ? BST_CHECKED : BST_UNCHECKED), 0);

    for(uint8_t ui8i = 0; ui8i < (sizeof(hWndPageItems) / sizeof(hWndPageItems[0])); ui8i++) {
        if(hWndPageItems[ui8i] == NULL) {
            ::MessageBox(m_hWnd, "Setting page creation failed!", GetPageName(), MB_OK);
            return false;
        }

        ::SendMessage(hWndPageItems[ui8i], WM_SETFONT, (WPARAM)hfFont, MAKELPARAM(TRUE, 0));
    }

    ::EnableWindow(hWndPageItems[EDT_TEMP_BAN_REDIR], SettingManager->bBools[SETBOOL_TEMP_BAN_REDIR] == true ? TRUE : FALSE);

    ::EnableWindow(hWndPageItems[EDT_PERM_BAN_REDIR], SettingManager->bBools[SETBOOL_PERM_BAN_REDIR] == true ? TRUE : FALSE);

    ::EnableWindow(hWndPageItems[LBL_TEMP_BAN_TIME], SettingManager->iShorts[SETSHORT_BRUTE_FORCE_PASS_PROTECT_BAN_TYPE] == 2 ? TRUE : FALSE);
    ::EnableWindow(hWndPageItems[EDT_TEMP_BAN_TIME], SettingManager->iShorts[SETSHORT_BRUTE_FORCE_PASS_PROTECT_BAN_TYPE] == 2 ? TRUE : FALSE);
    ::EnableWindow(hWndPageItems[UD_TEMP_BAN_TIME], SettingManager->iShorts[SETSHORT_BRUTE_FORCE_PASS_PROTECT_BAN_TYPE] == 2 ? TRUE : FALSE);
    ::EnableWindow(hWndPageItems[LBL_TEMP_BAN_TIME_HOURS], SettingManager->iShorts[SETSHORT_BRUTE_FORCE_PASS_PROTECT_BAN_TYPE] == 2 ? TRUE : FALSE);
    ::EnableWindow(hWndPageItems[BTN_REPORT_3X_BAD_PASS], SettingManager->iShorts[SETSHORT_BRUTE_FORCE_PASS_PROTECT_BAN_TYPE] == 0 ? FALSE : TRUE);

    ::SetWindowLongPtr(hWndPageItems[BTN_REPORT_3X_BAD_PASS], GWLP_USERDATA, (LONG_PTR)this);
    wpOldButtonProc = (WNDPROC)::SetWindowLongPtr(hWndPageItems[BTN_REPORT_3X_BAD_PASS], GWLP_WNDPROC, (LONG_PTR)ButtonProc);

	return true;
}
//------------------------------------------------------------------------------

char * SettingPageBans::GetPageName() {
    return LanguageManager->sTexts[LAN_BAN_HANDLING];
}
//------------------------------------------------------------------------------

void SettingPageBans::FocusLastItem() {
    ::SetFocus(hWndPageItems[BTN_REPORT_3X_BAD_PASS]);
}
//------------------------------------------------------------------------------