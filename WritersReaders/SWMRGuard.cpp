#include "SWMRGuard.h"

SWMRGuard::SWMRGuard() : 
  readersSemaphore{CreateSemaphore(NULL, 0, MAXLONG, NULL)},
  writersSemaphore{CreateSemaphore(NULL, 0, MAXLONG, NULL)},
  waitingReadersNumber{ 0 }, waitingWritersNumber{ 0 }, activeThreadsNumber{0}
{
  InitializeCriticalSection(&cs);
}

SWMRGuard::~SWMRGuard()
{
  waitingReadersNumber = waitingWritersNumber = activeThreadsNumber = 0;
  DeleteCriticalSection(&cs);
  CloseHandle(readersSemaphore);
  CloseHandle(writersSemaphore);
}

void SWMRGuard::waitToRead()
{
  EnterCriticalSection(&cs);

  bool isResourceWritePending = (waitingWritersNumber > 0) || (activeThreadsNumber < 0);
  if (isResourceWritePending)
    waitingReadersNumber++;
  else activeThreadsNumber++;

  LeaveCriticalSection(&cs);

  if (isResourceWritePending)
    WaitForSingleObject(readersSemaphore, INFINITE);
}

void SWMRGuard::waitToWrite()
{
  EnterCriticalSection(&cs);

  bool isResourcesOwned = activeThreadsNumber != 0;
  if (isResourcesOwned)
    waitingWritersNumber++;
  else activeThreadsNumber = -1;

  LeaveCriticalSection(&cs);

  if (isResourcesOwned)
    WaitForSingleObject(writersSemaphore, INFINITE);
}

void SWMRGuard::done()
{
  EnterCriticalSection(&cs);

  if (activeThreadsNumber > 0)
    activeThreadsNumber--;
  else activeThreadsNumber++;

  HANDLE semaphore = NULL;
  LONG count = 1;

  if (activeThreadsNumber == 0)
  {
    if (waitingWritersNumber > 0)
    {
      activeThreadsNumber = -1;
      waitingWritersNumber--;
      semaphore = writersSemaphore;
    }
    else if (waitingReadersNumber > 0)
    {
      activeThreadsNumber = waitingReadersNumber;
      count = activeThreadsNumber;
      waitingReadersNumber = 0;
      semaphore = readersSemaphore;
    }
  }

  LeaveCriticalSection(&cs);

  if (semaphore != NULL)
    ReleaseSemaphore(semaphore, count, NULL);
}
