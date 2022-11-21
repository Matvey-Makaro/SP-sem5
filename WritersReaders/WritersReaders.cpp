// WritersReaders.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "WritersReaders.h"
#include "SWMRGuard.h"

#include <tchar.h>

SWMRGuard swmrGuard;

DWORD WINAPI runFunction(PVOID pvParam)
{
  TCHAR sz[50];
  wsprintf(sz, TEXT("SWMRGuard Test: Thread %d"), PtrToShort(pvParam));
  int n = MessageBox(NULL, TEXT("YES: Attempt to reader\nNo: Attempt to write"), sz, MB_YESNO);

  if (n == IDYES)
    swmrGuard.waitToRead();
  else swmrGuard.waitToWrite();

  MessageBox(NULL, (n == IDYES) ? TEXT("OK stops READING") : TEXT("OK stops WRITING"), sz, MB_OK);

  swmrGuard.done();
  return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    constexpr size_t threads_num = 8;
    HANDLE threads[threads_num];

    for (int i = 0; i < threads_num; i++)
    {
      DWORD threadId;
      threads[i] = CreateThread(NULL, 0, runFunction, (PVOID)(DWORD_PTR)i, 0, &threadId);
    }

    WaitForMultipleObjects(threads_num, threads, TRUE, INFINITE);
    for (int i = 0; i < threads_num; i++)
      CloseHandle(threads[i]);

  return 0;
}
