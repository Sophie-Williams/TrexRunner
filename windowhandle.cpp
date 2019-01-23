#include "windowhandle.h"



/*QString WindowHandle::GetActiveWindow(){
    TCHAR wnd_title[256];
    HWND hwnd = GetForegroundWindow(); // get handle of currently active window
    GetWindowText(hwnd, wnd_title, 256);

    DWORD dwPID;
    GetWindowThreadProcessId(hwnd, &dwPID);

    HANDLE Handle = OpenProcess(
                      PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                      FALSE,
                      dwPID);
    if (Handle)
    {
        TCHAR Buffer[MAX_PATH];
        if (GetModuleFileNameEx(Handle, 0, Buffer, MAX_PATH))
        {
            _tprintf(_T("Path: %s"), Buffer);
            // At this point, buffer contains the full path to the executable
        }
        CloseHandle(Handle);
    }else{

    }
}*/
