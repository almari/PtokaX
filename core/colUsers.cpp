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
#include "stdinc.h"
//---------------------------------------------------------------------------
#include "colUsers.h"
//---------------------------------------------------------------------------
#include "globalQueue.h"
#include "LanguageManager.h"
#include "ProfileManager.h"
#include "ServerManager.h"
#include "SettingManager.h"
#include "UdpDebug.h"
#include "User.h"
#include "utility.h"
//---------------------------------------------------------------------------
#ifdef _WIN32
	#pragma hdrstop
//---------------------------------------------------------------------------
	#ifndef _SERVICE
		#ifndef _MSC_VER
			#include "TUsersChatForm.h"
		#endif
	#endif
//---------------------------------------------------------------------------
	#ifndef _MSC_VER
		#pragma package(smart_init)
	#endif
#endif
//---------------------------------------------------------------------------
static const uint32_t MYINFOLISTSIZE = 1024*256;
static const uint32_t IPLISTSIZE = 1024*64;
//---------------------------------------------------------------------------
classUsers *colUsers = NULL;
//---------------------------------------------------------------------------

classUsers::classUsers() {
    llist = NULL;
    elist = NULL;

    RecTimeList = NULL;
    
    ui16ActSearchs = 0;
    ui16PasSearchs = 0;

#ifdef _WIN32
    hRecvHeap = HeapCreate(HEAP_NO_SERIALIZE, 0x20000, 0);
    hSendHeap = HeapCreate(HEAP_NO_SERIALIZE, 0x40000, 0);

    nickList = (char *) HeapAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, NICKLISTSIZE);
#else
	nickList = (char *) calloc(NICKLISTSIZE, 1);
#endif
    if(nickList == NULL) {
		AppendSpecialLog("Cannot create nickList!");
		return;
    }
    memcpy(nickList, "$NickList |", 11);
    nickList[11] = '\0';
    nickListLen = 11;
    nickListSize = NICKLISTSIZE - 1;
#ifdef _WIN32
    sZNickList = (char *) HeapAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, ZLISTSIZE);
#else
	sZNickList = (char *) calloc(ZLISTSIZE, 1);
#endif
    if(sZNickList == NULL) {
		AppendSpecialLog("Cannot create sZNickList!");
		return;
    }
    iZNickListLen = 0;
    iZNickListSize = ZLISTSIZE - 1;
#ifdef _WIN32
    opList = (char *) HeapAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, OPLISTSIZE);
#else
	opList = (char *) calloc(OPLISTSIZE, 1);
#endif
    if(opList == NULL) {
		AppendSpecialLog("Cannot create opList!");
		return;
    }
    memcpy(opList, "$OpList |", 9);
    opList[9] = '\0';
    opListLen = 9;
    opListSize = OPLISTSIZE - 1;
#ifdef _WIN32
    sZOpList = (char *) HeapAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, ZLISTSIZE);
#else
	sZOpList = (char *) calloc(ZLISTSIZE, 1);
#endif
    if(sZOpList == NULL) {
		AppendSpecialLog("Cannot create sZOpList!");
		return;
    }
    iZOpListLen = 0;
    iZOpListSize = ZLISTSIZE - 1;
    
    if(SettingManager->ui8FullMyINFOOption != 0) {
#ifdef _WIN32
        myInfos = (char *) HeapAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, MYINFOLISTSIZE);
#else
		myInfos = (char *) calloc(MYINFOLISTSIZE, 1);
#endif
        if(myInfos == NULL) {
    		AppendSpecialLog("Cannot create myInfos!");
    		return;
        }
        myInfosSize = MYINFOLISTSIZE - 1;
        
#ifdef _WIN32
        sZMyInfos = (char *) HeapAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, ZMYINFOLISTSIZE);
#else
		sZMyInfos = (char *) calloc(ZMYINFOLISTSIZE, 1);
#endif
        if(sZMyInfos == NULL) {
    		AppendSpecialLog("Cannot create sZMyInfos!");
    		return;
        }
        iZMyInfosSize = ZMYINFOLISTSIZE - 1;
    } else {
        myInfos = NULL;
        myInfosSize = 0;
        sZMyInfos = NULL;
        iZMyInfosSize = 0;
    }
    myInfosLen = 0;
    iZMyInfosLen = 0;
    
    if(SettingManager->ui8FullMyINFOOption != 2) {
#ifdef _WIN32
        myInfosTag = (char *) HeapAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, MYINFOLISTSIZE);
#else
		myInfosTag = (char *) calloc(MYINFOLISTSIZE, 1);
#endif
        if(myInfosTag == NULL) {
    		AppendSpecialLog("Cannot create myInfosTag!");
    		return;
        }
        myInfosTagSize = MYINFOLISTSIZE - 1;

#ifdef _WIN32
        sZMyInfosTag = (char *) HeapAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, ZMYINFOLISTSIZE);
#else
		sZMyInfosTag = (char *) calloc(ZMYINFOLISTSIZE, 1);
#endif
        if(sZMyInfosTag == NULL) {
    		AppendSpecialLog("Cannot create sZMyInfosTag!");
    		return;
        }
        iZMyInfosTagSize = ZMYINFOLISTSIZE - 1;
    } else {
        myInfosTag = NULL;
        myInfosTagSize = 0;
        sZMyInfosTag = NULL;
        iZMyInfosTagSize = 0;
    }
    myInfosTagLen = 0;
    iZMyInfosTagLen = 0;

    iChatMsgs = 0;
    iChatMsgsTick = 0;
    iChatLockFromTick = 0;
    bChatLocked = false;

#ifdef _WIN32
    userIPList = (char *) HeapAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, IPLISTSIZE);
#else
	userIPList = (char *) calloc(IPLISTSIZE, 1);
#endif
    if(userIPList == NULL) {
		AppendSpecialLog("Cannot create userIPList!");
		return;
    }
    memcpy(userIPList, "$UserIP |", 9);
    userIPList[9] = '\0';
    userIPListLen = 9;
    userIPListSize = IPLISTSIZE - 1;

#ifdef _WIN32
    sZUserIPList = (char *) HeapAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, ZLISTSIZE);
#else
	sZUserIPList = (char *) calloc(ZLISTSIZE, 1);
#endif
    if(sZUserIPList == NULL) {
		AppendSpecialLog("Cannot create sZUserIPList!");
		return;
    }
    iZUserIPListLen = 0;
    iZUserIPListSize = ZLISTSIZE - 1;
}
//---------------------------------------------------------------------------

classUsers::~classUsers() {
    RecTime * next = RecTimeList;

    while(next != NULL) {
        RecTime * cur = next;
        next = cur->next;

        if(cur->sNick != NULL) {
#ifdef _WIN32
            if(HeapFree(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)cur->sNick) == 0) {
    			string sDbgstr = "[BUF] Cannot deallocate cur->sNick in classUsers::~classUsers! "+string((uint32_t)GetLastError())+" "+
    				string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0));
                AppendSpecialLog(sDbgstr);
            }
#else
			free(cur->sNick);
#endif
        }

        delete cur;
    }

    if(nickList != NULL) {
#ifdef _WIN32
        if(HeapFree(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)nickList) == 0) {
			string sDbgstr = "[BUF] Cannot deallocate nickList in classUsers::~classUsers! "+string((uint32_t)GetLastError())+" "+
				string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0));
            AppendSpecialLog(sDbgstr);
        }
#else
		free(nickList);
#endif
        nickList = NULL;
    }

    if(sZNickList != NULL) {
#ifdef _WIN32
        if(HeapFree(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)sZNickList) == 0) {
			string sDbgstr = "[BUF] Cannot deallocate sZNickList in classUsers::~classUsers! "+string((uint32_t)GetLastError())+" "+
				string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0));
			AppendSpecialLog(sDbgstr);
        }
#else
		free(sZNickList);
#endif
        sZNickList = NULL;
    }

    if(opList != NULL) {
#ifdef _WIN32
        if(HeapFree(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)opList) == 0) {
			string sDbgstr = "[BUF] Cannot deallocate opList in classUsers::~classUsers! "+string((uint32_t)GetLastError())+" "+
				string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0));
			AppendSpecialLog(sDbgstr);
        }
#else
		free(opList);
#endif
        opList = NULL;
    }

    if(sZOpList != NULL) {
#ifdef _WIN32
        if(HeapFree(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)sZOpList) == 0) {
			string sDbgstr = "[BUF] Cannot deallocate sZOpList in classUsers::~classUsers! "+string((uint32_t)GetLastError())+" "+
				string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0));
			AppendSpecialLog(sDbgstr);
        }
#else
		free(sZOpList);
#endif
        sZOpList = NULL;
    }

    if(myInfos != NULL) {
#ifdef _WIN32
        if(HeapFree(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)myInfos) == 0) {
			string sDbgstr = "[BUF] Cannot deallocate myInfos in classUsers::~classUsers! "+string((uint32_t)GetLastError())+" "+
				string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0));
			AppendSpecialLog(sDbgstr);
        }
#else
		free(myInfos);
#endif
        myInfos = NULL;
    }

    if(sZMyInfos != NULL) {
#ifdef _WIN32
        if(HeapFree(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)sZMyInfos) == 0) {
			string sDbgstr = "[BUF] Cannot deallocate sZMyInfos in classUsers::~classUsers! "+string((uint32_t)GetLastError())+" "+
				string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0));
			AppendSpecialLog(sDbgstr);
        }
#else
		free(sZMyInfos);
#endif
        sZMyInfos = NULL;
    }

    if(myInfosTag != NULL) {
#ifdef _WIN32
        if(HeapFree(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)myInfosTag) == 0) {
			string sDbgstr = "[BUF] Cannot deallocate myInfosTag in classUsers::~classUsers! "+string((uint32_t)GetLastError())+" "+
				string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0));
			AppendSpecialLog(sDbgstr);
        }
#else
		free(myInfosTag);
#endif
        myInfosTag = NULL;
    }

    if(sZMyInfosTag != NULL) {
#ifdef _WIN32
        if(HeapFree(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)sZMyInfosTag) == 0) {
			string sDbgstr = "[BUF] Cannot deallocate sZMyInfosTag in classUsers::~classUsers! "+string((uint32_t)GetLastError())+" "+
				string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0));
			AppendSpecialLog(sDbgstr);
        }
#else
		free(sZMyInfosTag);
#endif
        sZMyInfosTag = NULL;
    }

    if(userIPList != NULL) {
#ifdef _WIN32
        if(HeapFree(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)userIPList) == 0) {
			string sDbgstr = "[BUF] Cannot deallocate userIPList in classUsers::~classUsers! "+string((uint32_t)GetLastError())+" "+
				string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0));
			AppendSpecialLog(sDbgstr);
        }
#else
		free(userIPList);
#endif
        userIPList = NULL;
    }

    if(sZUserIPList != NULL) {
#ifdef _WIN32
        if(HeapFree(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)sZUserIPList) == 0) {
			string sDbgstr = "[BUF] Cannot deallocate sZUserIPList in classUsers::~classUsers! "+string((uint32_t)GetLastError())+" "+
				string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0));
			AppendSpecialLog(sDbgstr);
        }
#else
		free(sZUserIPList);
#endif
        sZUserIPList = NULL;
    }

#ifdef _WIN32
    HeapDestroy(hRecvHeap);
    HeapDestroy(hSendHeap);
#endif
}
//---------------------------------------------------------------------------

void classUsers::AddUser(User * u) {
    if(llist == NULL) {
    	llist = u;
    	elist = u;
    } else {
        u->prev = elist;
        elist->next = u;
        elist = u;
    }
}
//---------------------------------------------------------------------------

void classUsers::RemUser(User * u) {
    if(u->prev == NULL) {
        if(u->next == NULL) {
            llist = NULL;
            elist = NULL;
        } else {
            u->next->prev = NULL;
            llist = u->next;
        }
    } else if(u->next == NULL) {
        u->prev->next = NULL;
        elist = u->prev;
    } else {
        u->prev->next = u->next;
        u->next->prev = u->prev;
    }
    #ifdef _DEBUG
        int iret = sprintf(msg, "# User %s removed from linked list.", u->Nick);
        if(CheckSprintf(iret, 1024, "classUsers::RemUser") == true) {
            Cout(msg);
        }
    #endif
}
//---------------------------------------------------------------------------

void classUsers::DisconnectAll() {
    uint32_t iCloseLoops = 0;
    while(llist != NULL && iCloseLoops <= 100) {
        User *next = llist;
        while(next != NULL) {
            User *u = next;
    		next = u->next;
            if(((u->ui32BoolBits & User::BIT_ERROR) == User::BIT_ERROR) == true || u->sbdatalen == 0) {
//              Memo("*** User " + string(u->Nick, u->NickLen) + " closed...");
                if(u->prev == NULL) {
                    if(u->next == NULL) {
                        llist = NULL;
                    } else {
                        u->next->prev = NULL;
                        llist = u->next;
                    }
                } else if(u->next == NULL) {
                    u->prev->next = NULL;
                } else {
                    u->prev->next = u->next;
                    u->next->prev = u->prev;
                }

#ifdef _WIN32
                shutdown(u->Sck, SD_SEND);
				closesocket(u->Sck);
#else
                shutdown(u->Sck, SHUT_RD);
				close(u->Sck);
#endif

				delete u;
            } else {
                UserTry2Send(u);
            }
        }
        iCloseLoops++;
#ifdef _WIN32
        ::Sleep(50);
#else
		usleep(50000);
#endif
    }

    User *next = llist;
    while(next != NULL) {
        User *u = next;
    	next = u->next;
#ifdef _WIN32
    	shutdown(u->Sck, SD_SEND);
		closesocket(u->Sck);
#else
    	shutdown(u->Sck, SHUT_RDWR);
		close(u->Sck);
#endif

		delete u;
	}

#ifdef _WIN32
	#ifndef _SERVICE
		#ifndef _MSC_VER
			if(UsersChatForm != NULL) {
				UsersChatForm->userList->Items->Clear();
			}
		#endif
	#endif
#endif
}
//---------------------------------------------------------------------------

// NICKLIST OPERATIONS
void classUsers::Add2NickList(char * Nick, const size_t &iNickLen, const bool &isOp) {
    // $NickList nick$$nick2$$|

    if(nickListSize < nickListLen+iNickLen+2) {
#ifdef _WIN32
        nickList = (char *) HeapReAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)nickList, nickListSize+NICKLISTSIZE+1);
#else
		nickList = (char *) realloc(nickList, nickListSize+NICKLISTSIZE+1);
#endif
        if(nickList == NULL) {
			string sDbgstr = "[BUF] Cannot reallocate "+string(nickListSize)+"/"+string(nickListSize+NICKLISTSIZE+1)+
				" bytes of memory in classUsers::Add2NickList for NickList!";
#ifdef _WIN32
			sDbgstr += " "+string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0))+GetMemStat();
#endif
			AppendSpecialLog(sDbgstr);
            return;
		}
        nickListSize += NICKLISTSIZE;
    }

    memcpy(nickList+nickListLen-1, Nick, iNickLen);
    nickListLen += (uint32_t)(iNickLen+2);

    nickList[nickListLen-3] = '$';
    nickList[nickListLen-2] = '$';
    nickList[nickListLen-1] = '|';
    nickList[nickListLen] = '\0';

    iZNickListLen = 0;

    if(isOp == false)
        return;

    if(opListSize < opListLen+iNickLen+2) {
#ifdef _WIN32
        opList = (char *) HeapReAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)opList, opListSize+OPLISTSIZE+1);
#else
		opList = (char *) realloc(opList, opListSize+OPLISTSIZE+1);
#endif
        if(opList == NULL) {
			string sDbgstr = "[BUF] Cannot reallocate "+string(opListSize)+"/"+string(opListSize+OPLISTSIZE+1)+
				" bytes of memory in classUsers::Add2NickList for opList!";
#ifdef _WIN32
			sDbgstr += " "+string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0))+GetMemStat();
#endif
            AppendSpecialLog(sDbgstr);
            return;
        }
        opListSize += OPLISTSIZE;
    }

    memcpy(opList+opListLen-1, Nick, iNickLen);
    opListLen += (uint32_t)(iNickLen+2);

    opList[opListLen-3] = '$';
    opList[opListLen-2] = '$';
    opList[opListLen-1] = '|';
    opList[opListLen] = '\0';

    iZOpListLen = 0;
}
//---------------------------------------------------------------------------

void classUsers::Add2OpList(char * Nick, const size_t &iNickLen) {
    if(opListSize < opListLen+iNickLen+2) {
#ifdef _WIN32
        opList = (char *) HeapReAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)opList, opListSize+OPLISTSIZE+1);
#else
		opList = (char *) realloc(opList, opListSize+OPLISTSIZE+1);
#endif
        if(opList == NULL) {
			string sDbgstr = "[BUF] Cannot reallocate "+string(opListSize)+"/"+string(opListSize+OPLISTSIZE+1)+
				" bytes of memory in classUsers::Add2OpList for opList!";
#ifdef _WIN32
			sDbgstr += " "+string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0))+GetMemStat();
#endif
            AppendSpecialLog(sDbgstr);
            return;
        }
        opListSize += OPLISTSIZE;
    }

    memcpy(opList+opListLen-1, Nick, iNickLen);
    opListLen += (uint32_t)(iNickLen+2);

    opList[opListLen-3] = '$';
    opList[opListLen-2] = '$';
    opList[opListLen-1] = '|';
    opList[opListLen] = '\0';

    iZOpListLen = 0;
}
//---------------------------------------------------------------------------

void classUsers::DelFromNickList(char * Nick, const bool &isOp) {
    int m = sprintf(msg, "$%s$", Nick);
    if(CheckSprintf(m, 1024, "classUsers::DelFromNickList") == false) {
        return;
    }

    nickList[9] = '$';
    char *off = strstr(nickList, msg);
    nickList[9] = ' ';

    if(off != NULL) {
        memmove(off+1, off+(m+1), nickListLen-((off+m)-nickList));
        nickListLen -= m;
        iZNickListLen = 0;
    }

    if(!isOp) return;

    opList[7] = '$';
    off = strstr(opList, msg);
    opList[7] = ' ';

    if(off != NULL) {
        memmove(off+1, off+(m+1), opListLen-((off+m)-opList));
        opListLen -= m;
        iZOpListLen = 0;
    }
}
//---------------------------------------------------------------------------

void classUsers::DelFromOpList(char * Nick) {
    int m = sprintf(msg, "$%s$", Nick);
    if(CheckSprintf(m, 1024, "classUsers::DelFromOpList") == false) {
        return;
    }

    opList[7] = '$';
    char *off = strstr(opList, msg);
    opList[7] = ' ';

    if(off != NULL) {
        memmove(off+1, off+(m+1), opListLen-((off+m)-opList));
        opListLen -= m;
        iZOpListLen = 0;
    }
}
//---------------------------------------------------------------------------

// PPK ... check global mainchat flood and add to global queue
void classUsers::SendChat2All(User * cur, char * data, const size_t &iChatLen) {
    UdpDebug->Broadcast(data, iChatLen);

    if(ProfileMan->IsAllowed(cur, ProfileManager::NODEFLOODMAINCHAT) == false && 
        SettingManager->iShorts[SETSHORT_GLOBAL_MAIN_CHAT_ACTION] != 0) {
        if(iChatMsgs == 0) {
			iChatMsgsTick = ui64ActualTick;
			iChatLockFromTick = ui64ActualTick;
            iChatMsgs = 0;
            bChatLocked = false;
		} else if((iChatMsgsTick+SettingManager->iShorts[SETSHORT_GLOBAL_MAIN_CHAT_TIME]) < ui64ActualTick) {
			iChatMsgsTick = ui64ActualTick;
            iChatMsgs = 0;
        }

        iChatMsgs++;

        if(iChatMsgs > (uint16_t)SettingManager->iShorts[SETSHORT_GLOBAL_MAIN_CHAT_MESSAGES]) {
			iChatLockFromTick = ui64ActualTick;
            if(bChatLocked == false) {
                if(SettingManager->bBools[SETBOOL_DEFLOOD_REPORT] == true) {
                    if(SettingManager->bBools[SETBOOL_SEND_STATUS_MESSAGES_AS_PM] == true) {
                        int imsgLen = sprintf(msg, "%s $<%s> *** %s.|", SettingManager->sPreTexts[SetMan::SETPRETXT_HUB_SEC], 
                            SettingManager->sPreTexts[SetMan::SETPRETXT_HUB_SEC], 
                            LanguageManager->sTexts[LAN_GLOBAL_CHAT_FLOOD_DETECTED]);
                        if(CheckSprintf(imsgLen, 1024, "classUsers::SendChat2All1") == true) {
                            QueueDataItem *newItem = globalQ->CreateQueueDataItem(msg, imsgLen, NULL, 0, globalqueue::PM2OPS);
                            globalQ->SingleItemsStore(newItem);
                        }
                    } else {
                        int imsgLen = sprintf(msg, "<%s> *** %s.|", SettingManager->sPreTexts[SetMan::SETPRETXT_HUB_SEC], 
                            LanguageManager->sTexts[LAN_GLOBAL_CHAT_FLOOD_DETECTED]);
                        if(CheckSprintf(imsgLen, 1024, "classUsers::SendChat2All2") == true) {
                            globalQ->OPStore(msg, imsgLen);
                        }
                    }
                }
                bChatLocked = true;
            }
        }

        if(bChatLocked == true) {
            if((iChatLockFromTick+SettingManager->iShorts[SETSHORT_GLOBAL_MAIN_CHAT_TIMEOUT]) > ui64ActualTick) {
                if(SettingManager->iShorts[SETSHORT_GLOBAL_MAIN_CHAT_ACTION] == 1) {
                    return;
                } else if(SettingManager->iShorts[SETSHORT_GLOBAL_MAIN_CHAT_ACTION] == 2) {
                    size_t iWantLen = iChatLen+17;
#ifdef _WIN32
                    char *MSG = (char *) HeapAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, iChatLen+17);
#else
					char *MSG = (char *) malloc(iChatLen+17);
#endif
                    if(MSG == NULL) {
						string sDbgstr = "[BUF] Cannot allocate "+string((uint64_t)iWantLen)+" bytes of memory in classUsers::SendChat2All! "+
							string(data, iChatLen);
#ifdef _WIN32
						sDbgstr += " "+string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0))+GetMemStat();
#endif
                        AppendSpecialLog(sDbgstr);
                        return;
                    }
                	int iMsgLen = sprintf(MSG, "%s ", cur->IP);
                	if(CheckSprintf(iMsgLen, iWantLen, "classUsers::SendChat2All3") == true) {
                        memcpy(MSG+iMsgLen, data, iChatLen);
                        iMsgLen += (uint32_t)iChatLen;
                        MSG[iMsgLen] = '\0';
                        globalQ->OPStore(MSG, iMsgLen);
                    }
#ifdef _WIN32
                    if(HeapFree(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)MSG) == 0) {
						string sDbgstr = "[BUF] Cannot deallocate MSG in classUsers::SendChat2All! "+string((uint32_t)GetLastError())+" "+
							string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0));
						AppendSpecialLog(sDbgstr);
                    }
#else
					free(MSG);
#endif
                    return;
                }
            } else {
                bChatLocked = false;
            }
        }
    }

    globalQ->Store(data, iChatLen);
}
//---------------------------------------------------------------------------

void classUsers::Add2MyInfos(User * u) {
    if(myInfosSize < myInfosLen+u->iMyInfoLen) {
#ifdef _WIN32
        myInfos = (char *) HeapReAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)myInfos, myInfosSize+MYINFOLISTSIZE+1);
#else
		myInfos = (char *) realloc(myInfos, myInfosSize+MYINFOLISTSIZE+1);
#endif
        if(myInfos == NULL) {
			string sDbgstr = "[BUF] Cannot reallocate "+string(myInfosSize)+"/"+string(myInfosSize+MYINFOLISTSIZE+1)+
				" bytes of memory in classUsers::Add2MyInfos! "+string(u->MyInfo, u->iMyInfoLen);
#ifdef _WIN32
			sDbgstr += " "+string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0))+GetMemStat();
#endif
			AppendSpecialLog(sDbgstr);
            return;
        }
        myInfosSize += MYINFOLISTSIZE;
    }

    memcpy(myInfos+myInfosLen, u->MyInfo, u->iMyInfoLen);
    myInfosLen += u->iMyInfoLen;

    myInfos[myInfosLen] = '\0';

    iZMyInfosLen = 0;
}
//---------------------------------------------------------------------------

void classUsers::DelFromMyInfos(User * u) {
	char *match = strstr(myInfos, u->MyInfo+8);
    if(match != NULL) {
		match -= 8;
		memmove(match, match+u->iMyInfoLen, myInfosLen-((match+(u->iMyInfoLen-1))-myInfos));
        myInfosLen -= u->iMyInfoLen;
        iZMyInfosLen = 0;
    }
}
//---------------------------------------------------------------------------

void classUsers::Add2MyInfosTag(User * u) {
    if(myInfosTagSize < myInfosTagLen+u->iMyInfoTagLen) {
#ifdef _WIN32
        myInfosTag = (char *) HeapReAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)myInfosTag, myInfosTagSize+MYINFOLISTSIZE+1);
#else
		myInfosTag = (char *) realloc(myInfosTag, myInfosTagSize+MYINFOLISTSIZE+1);
#endif
        if(myInfosTag == NULL) {
			string sDbgstr = "[BUF] Cannot reallocate "+string(myInfosTagSize)+"/"+string(myInfosTagSize+MYINFOLISTSIZE+1)+
				" bytes of memory in classUsers::Add2MyInfosTag! "+string(u->MyInfoTag, u->iMyInfoTagLen);
#ifdef _WIN32
			sDbgstr += " "+string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0))+GetMemStat();
#endif
			AppendSpecialLog(sDbgstr);
            return;
        }
        myInfosTagSize += MYINFOLISTSIZE;
    }

    memcpy(myInfosTag+myInfosTagLen, u->MyInfoTag, u->iMyInfoTagLen);
    myInfosTagLen += u->iMyInfoTagLen;

    myInfosTag[myInfosTagLen] = '\0';

    iZMyInfosTagLen = 0;
}
//---------------------------------------------------------------------------

void classUsers::DelFromMyInfosTag(User * u) {
	char *match = strstr(myInfosTag, u->MyInfoTag+8);
    if(match != NULL) {
		match -= 8;
        memmove(match, match+u->iMyInfoTagLen, myInfosTagLen-((match+(u->iMyInfoTagLen-1))-myInfosTag));
        myInfosTagLen -= u->iMyInfoTagLen;
        iZMyInfosTagLen = 0;
    }
}
//---------------------------------------------------------------------------

void classUsers::AddBot2MyInfos(char * MyInfo) {
	size_t len = strlen(MyInfo);
	if(myInfosTag != NULL) {
	    if(strstr(myInfosTag, MyInfo) == NULL ) {
            if(myInfosTagSize < myInfosTagLen+len) {
#ifdef _WIN32
                myInfosTag = (char *) HeapReAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)myInfosTag, myInfosTagSize+MYINFOLISTSIZE+1);
#else
				myInfosTag = (char *) realloc(myInfosTag, myInfosTagSize+MYINFOLISTSIZE+1);
#endif
                if(myInfosTag == NULL) {
					string sDbgstr = "[BUF] Cannot reallocate "+string(myInfosTagSize)+"/"+string(myInfosTagSize+MYINFOLISTSIZE+1)+
						" bytes of memory for myInfosTag in classUsers::AddBot2MyInfos! "+string(MyInfo, len);
#ifdef _WIN32
					sDbgstr += " "+string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0))+GetMemStat();
#endif
					AppendSpecialLog(sDbgstr);
                    return;
                }
                myInfosTagSize += MYINFOLISTSIZE;
            }
        	memcpy(myInfosTag+myInfosTagLen, MyInfo, len);
            myInfosTagLen += (uint32_t)len;
            myInfosTag[myInfosTagLen] = '\0';
            iZMyInfosLen = 0;
        }
    }
    if(myInfos != NULL) {
    	if(strstr(myInfos, MyInfo) == NULL ) {
            if(myInfosSize < myInfosLen+len) {
#ifdef _WIN32
                myInfos = (char *) HeapReAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)myInfos, myInfosSize+MYINFOLISTSIZE+1);
#else
				myInfos = (char *) realloc(myInfos, myInfosSize+MYINFOLISTSIZE+1);
#endif
                if(myInfos == NULL) {
					string sDbgstr = "[BUF] Cannot reallocate "+string(myInfosSize)+"/"+string(myInfosSize+MYINFOLISTSIZE+1)+
						" bytes of memory for myInfos in classUsers::AddBot2MyInfos! "+string(MyInfo, len);
#ifdef _WIN32
					sDbgstr += " "+string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0))+GetMemStat();
#endif
					AppendSpecialLog(sDbgstr);
                    return;
                }
                myInfosSize += MYINFOLISTSIZE;
            }
        	memcpy(myInfos+myInfosLen, MyInfo, len);
            myInfosLen += (uint32_t)len;
            myInfos[myInfosLen] = '\0';
            iZMyInfosTagLen = 0;
         }
    }
}
//---------------------------------------------------------------------------

void classUsers::DelBotFromMyInfos(char * MyInfo) {
	size_t len = strlen(MyInfo);
	if(myInfosTag) {
		char *match = strstr(myInfosTag,  MyInfo);
	    if(match) {
    		memmove(match, match+len, myInfosTagLen-((match+(len-1))-myInfosTag));
        	myInfosTagLen -= (uint32_t)len;
        	iZMyInfosTagLen = 0;
         }
    }
	if(myInfos) {
		char *match = strstr(myInfos,  MyInfo);
	    if(match) {
    		memmove(match, match+len, myInfosLen-((match+(len-1))-myInfos));
        	myInfosLen -= (uint32_t)len;
        	iZMyInfosLen = 0;
         }
    }
}
//---------------------------------------------------------------------------

void classUsers::Add2UserIP(User * cur) {
    int m = sprintf(msg,"$%s %s$", cur->Nick, cur->IP);
    if(CheckSprintf(m, 1024, "classUsers::Add2UserIP") == false) {
        return;
    }

    if(userIPListSize < userIPListLen+m) {
#ifdef _WIN32
        userIPList = (char *) HeapReAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)userIPList, userIPListSize+IPLISTSIZE+1);
#else
		userIPList = (char *) realloc(userIPList, userIPListSize+IPLISTSIZE+1);
#endif
        if(userIPList == NULL) {
			string sDbgstr = "[BUF] Cannot reallocate "+string(userIPListSize)+"/"+string(userIPListSize+IPLISTSIZE+1)+
				" bytes of memory in classUsers::Add2UserIP!";
#ifdef _WIN32
			sDbgstr += " "+string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0))+GetMemStat();
#endif
			AppendSpecialLog(sDbgstr);
            return;
        }
        userIPListSize += IPLISTSIZE;
    }

    memcpy(userIPList+userIPListLen-1, msg+1, m-1);
    userIPListLen += m;

    userIPList[userIPListLen-2] = '$';
    userIPList[userIPListLen-1] = '|';
    userIPList[userIPListLen] = '\0';

    iZUserIPListLen = 0;
}
//---------------------------------------------------------------------------

void classUsers::DelFromUserIP(User * cur) {
    int m = sprintf(msg,"$%s %s$", cur->Nick, cur->IP);
    if(CheckSprintf(m, 1024, "classUsers::DelFromUserIP") == false) {
        return;
    }

    userIPList[7] = '$';
    char *off = strstr(userIPList, msg);
    userIPList[7] = ' ';
    
	if(off != NULL) {
        memmove(off+1, off+(m+1), userIPListLen-((off+m)-userIPList));
        userIPListLen -= m;
        iZUserIPListLen = 0;
    }
}
//---------------------------------------------------------------------------

void classUsers::Add2RecTimes(User * curUser) {
    time_t acc_time;
    time(&acc_time);

    if(ProfileMan->IsAllowed(curUser, ProfileManager::NOUSRSAMEIP) == true || 
        (acc_time-curUser->LoginTime) >= SettingManager->iShorts[SETSHORT_MIN_RECONN_TIME]) {
        return;
    }

    RecTime * cur = new RecTime();

	if(cur == NULL) {
#ifdef _WIN32
		string sDbgstr = "[BUF] Cannot allocate RecTime in classUsers::Add2RecTimes! "+string(HeapValidate(GetProcessHeap, 0, 0))+GetMemStat();
#else
		string sDbgstr = "[BUF] Cannot allocate RecTime in classUsers::Add2RecTimes!";
#endif
		AppendSpecialLog(sDbgstr);
        exit(EXIT_FAILURE);
	}

#ifdef _WIN32
    cur->sNick = (char *) HeapAlloc(hPtokaXHeap, HEAP_NO_SERIALIZE, curUser->NickLen+1);
#else
	cur->sNick = (char *) malloc(curUser->NickLen+1);
#endif
	if(cur->sNick == NULL) {
		string sDbgstr = "[BUF] Cannot allocate "+string(curUser->NickLen+1)+" bytes of memory in classUsers::Add2RecTimes!";
#ifdef _WIN32
		sDbgstr += " "+string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0))+GetMemStat();
#endif
        AppendSpecialLog(sDbgstr);
        exit(EXIT_FAILURE);
    }

    memcpy(cur->sNick, curUser->Nick, curUser->NickLen);
    cur->sNick[curUser->NickLen] = '\0';

	cur->ui64DisConnTick = ui64ActualTick-(acc_time-curUser->LoginTime);
    cur->ui32NickHash = curUser->ui32NickHash;
    cur->ui32IpHash = curUser->ui32IpHash;

    cur->prev = NULL;
    cur->next = RecTimeList;

	if(RecTimeList != NULL) {
		RecTimeList->prev = cur;
	}

	RecTimeList = cur;
}
//---------------------------------------------------------------------------

bool classUsers::CheckRecTime(User * curUser) {
    RecTime * next = RecTimeList;

    while(next != NULL) {
        RecTime * cur = next;
        next = cur->next;

        // check expires...
        if(cur->ui64DisConnTick+SettingManager->iShorts[SETSHORT_MIN_RECONN_TIME] <= ui64ActualTick) {
            if(cur->sNick != NULL) {
#ifdef _WIN32
                if(HeapFree(hPtokaXHeap, HEAP_NO_SERIALIZE, (void *)cur->sNick) == 0) {
        			string sDbgstr = "[BUF] Cannot deallocate cur->sNick in classUsers::CheckRecTime! "+string((uint32_t)GetLastError())+" "+
        				string(HeapValidate(hPtokaXHeap, HEAP_NO_SERIALIZE, 0));
                    AppendSpecialLog(sDbgstr);
                }
#else
				free(cur->sNick);
#endif
            }

            if(cur->prev == NULL) {
                if(cur->next == NULL) {
                    RecTimeList = NULL;
                } else {
                    cur->next->prev = NULL;
                    RecTimeList = cur->next;
                }
            } else if(cur->next == NULL) {
                cur->prev->next = NULL;
            } else {
                cur->prev->next = cur->next;
                cur->next->prev = cur->prev;
            }

            delete cur;
            continue;
        }

        if(cur->ui32NickHash == curUser->ui32NickHash && cur->ui32IpHash == curUser->ui32IpHash &&
#ifdef _WIN32
            stricmp(cur->sNick, curUser->Nick) == 0) {
            int imsgLen = sprintf(msg, "<%s> %s %I64d %s.|", SettingManager->sPreTexts[SetMan::SETPRETXT_HUB_SEC], 
#else
			strcasecmp(cur->sNick, curUser->Nick) == 0) {
            int imsgLen = sprintf(msg, "<%s> %s %" PRIu64 " %s.|", SettingManager->sPreTexts[SetMan::SETPRETXT_HUB_SEC], 
#endif
                LanguageManager->sTexts[LAN_PLEASE_WAIT], 
                (cur->ui64DisConnTick+SettingManager->iShorts[SETSHORT_MIN_RECONN_TIME])-ui64ActualTick, 
                LanguageManager->sTexts[LAN_SECONDS_BEFORE_RECONN]);
            if(CheckSprintf(imsgLen, 1024, "classUsers::CheckRecTime1") == true) {
                UserSendChar(curUser, msg, imsgLen);
            }
            return true;
        }
    }

    return false;
}
//---------------------------------------------------------------------------