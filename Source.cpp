#include <windows.h>
#include <stdio.h>
#include <Wtsapi32.h>
#include <sddl.h>
#include <aclapi.h>
#include <tchar.h>
#include <psapi.h>
#include <processthreadsapi.h>

#pragma  comment (lib,"Wtsapi32.lib")
struct LinkedList {
    int argNum = 0;
    int spID;
    LPTSTR spSid;
    LPWSTR processName;
    LinkedList* next = nullptr;
};

PWTS_PROCESS_INFO pWtspi;
DWORD dwCount;
LinkedList* head;

DWORD WINAPI ManageLinkedList1(void* data);
DWORD WINAPI ManageLinkedList2(void* data);
LinkedList* initalizeFirstProcessLinkedList();
void PrintLiknedList(LinkedList* head);

int main()
{
    printf("Start Runing...\n");
    head = initalizeFirstProcessLinkedList();
    HANDLE thread1 = CreateThread(NULL, 0, ManageLinkedList1, NULL, 0, NULL);
    HANDLE thread2 = CreateThread(NULL, 0, ManageLinkedList2, NULL, 0, NULL);
    WaitForSingleObject(thread1, INFINITE);
    WaitForSingleObject(thread2, INFINITE);
}

LinkedList* initalizeFirstProcessLinkedList() {
    LPTSTR pSid = NULL;
    head = new LinkedList();
    LinkedList* current = head;
    if (!WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pWtspi, &dwCount))
    {
        int a = GetLastError();
        exit(0);
    }
    for (DWORD i = 0; i < dwCount; i++)
    {
        if (pWtspi[i].ProcessId != 0) {
            if (ConvertSidToStringSid(pWtspi[i].pUserSid, &pSid)) {
                head->argNum = head->argNum + 1;
                current->spID = pWtspi[i].ProcessId;
                current->spSid = pSid;
                current->processName = pWtspi[i].pProcessName;
                current->next = new LinkedList();
                current = current->next;
            }
        }
    }
    return head;
}

void PrintLiknedList(LinkedList* printhead) {
    int i = 1;
    while (printhead != nullptr) {
        printf("%d)   %ls : %ls  (%d)\n",i, printhead->spSid, printhead->processName, printhead->spID);
        printhead = printhead->next;
        i++;
    }
}

DWORD WINAPI ManageLinkedList1(void* data) {
    LPTSTR pSid = NULL;
    int flag = 0;
    LinkedList* current;
    while (1) {
        pWtspi = NULL;
        dwCount = NULL;
        if (!WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pWtspi, &dwCount))
        {
            int a = GetLastError();
            exit(0);
        }
        for (DWORD i = 0; i < dwCount; i++) {
            if (ConvertSidToStringSid(pWtspi[i].pUserSid, &pSid)) {
                current = head;
                int flag2 = 0;
                flag = 0;
                while (flag == 0 && flag2 == 0) {
                    if (pWtspi != NULL) {
                        if (pWtspi[i].ProcessId == current->spID) {
                            flag = 1;
                        }
                    }
                    if (current->next == NULL) {
                        flag2 = 1;
                    }
                    else {
                        current = current->next;
                    }
                }
                if (flag2 == 1 && flag == 0) {
                    {
                        if (pWtspi != NULL) {
                            head->argNum = head->argNum + 1;
                            LinkedList* newNode = new LinkedList();
                            newNode->spID = pWtspi[i].ProcessId;
                            newNode->spSid = pSid;
                            newNode->processName = pWtspi[i].pProcessName;
                            current->next = newNode;
                        }
                    }
                }
            }
        }
        Sleep(2000);
    }
    return 0;
}

DWORD WINAPI ManageLinkedList2(void* data) {
    LPTSTR pSid = NULL;
    int flag = 0;
    LinkedList* current;
    LinkedList* DeleteNode = head;
    while (1) {
        current = head;
        while (current != NULL) {
            pWtspi = NULL;
            dwCount = NULL;
            DWORD lpExitCode = 0;
            flag = 0;
            if (!WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pWtspi, &dwCount))
            {
                int a = GetLastError();
                exit(0);
            }
            for (DWORD i = 0; i < dwCount && flag==0; i++) {
                if (ConvertSidToStringSid(pWtspi[i].pUserSid, &pSid)) {
                    if (pWtspi[i].ProcessId == current->spID) {
                        flag = 1;
                    }
                }
            }
            if (flag == 0) {
                GetExitCodeProcess(OpenProcess(PROCESS_ALL_ACCESS, FALSE, current->spID), &lpExitCode);
                if (lpExitCode == STILL_ACTIVE) {
                    printf("%ls (%d) process still active and change privilage\n", current->processName,current->spID);
                }
                else {
                    head->argNum = head->argNum -1;
                    DeleteNode->next = current->next;               
                }
            }
            DeleteNode = current;
            current = current->next;
        }
        Sleep(2000);
    }
    return 0;
}


