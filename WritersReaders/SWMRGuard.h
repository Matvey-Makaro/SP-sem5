#pragma once
#include <Windows.h>

class SWMRGuard
{
public:
  SWMRGuard();
  ~SWMRGuard();

  void waitToRead();
  void waitToWrite();
  void done();

private:
  CRITICAL_SECTION cs;
  HANDLE readersSemaphore;
  HANDLE writersSemaphore;
  int waitingReadersNumber;
  int waitingWritersNumber;
  int activeThreadsNumber;  // -1 if active thread is writer.
};