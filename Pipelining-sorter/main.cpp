#include <iostream>
#include <vector>
#include <algorithm>

#include <Windows.h>
using namespace std;

int wmain(int argc, wchar_t* argv[])
{
  if (argc < 4)
  {
    cerr << "Few arguments!" << endl;
    system("pause");
    return -1;
  }

  auto hReadPipeGenerator = reinterpret_cast<HANDLE>(_wtoi(argv[1]));
  const wchar_t* enableReadGeneratedDataName = argv[2];
  const wchar_t* enableWriteGeneratedDataName = argv[3];

  HANDLE hEnableReadGeneratedData = OpenEvent(EVENT_ALL_ACCESS, FALSE, enableReadGeneratedDataName);
  if (hEnableReadGeneratedData == nullptr)
  {
    cerr << "Open event fail! " << GetLastError() << endl;
    system("pause");
    return GetLastError();
  }

  HANDLE hEnableWriteGeneratedData = OpenEvent(EVENT_ALL_ACCESS, FALSE, enableWriteGeneratedDataName);
  if (hEnableWriteGeneratedData == nullptr)
  {
    cerr << "Open event fail! " << GetLastError() << endl;
    system("pause");
    return GetLastError();
  }

  HANDLE hWritePipeSorter, hReadPipeSorter;
  if (!CreatePipe(&hReadPipeSorter, &hWritePipeSorter, NULL, 0))
  {
    cerr << "Create pipe failed!" << endl;
    system("pause");
    return GetLastError();
  }

  HANDLE hInheritReadPipeSorter;
  if (!DuplicateHandle(GetCurrentProcess(), hReadPipeSorter, GetCurrentProcess(), &hInheritReadPipeSorter, 0, TRUE, DUPLICATE_SAME_ACCESS))
  {
    cerr << "Duplicate handle error!" << endl;
    system("pause");
    return GetLastError();
  }
  CloseHandle(hReadPipeSorter);

  wchar_t enableReadSortedDataName[] = L"EnableReadSortedData";
  HANDLE hEnableReadSorted = CreateEvent(NULL, FALSE, FALSE, enableReadSortedDataName);
  if (hEnableReadSorted == NULL)
  {
    cerr << "Create event failed." << endl;
    system("pause");
    return GetLastError();
  }

  wchar_t enableWriteNameSortedData[] = L"EnableWriteSortedData";
  HANDLE hEnableWriteSorted = CreateEvent(NULL, FALSE, TRUE, enableWriteNameSortedData);
  if (hEnableWriteSorted == NULL)
  {
    cerr << "Create event failed." << endl;
    system("pause");
    return GetLastError();
  }

  constexpr size_t COMMAND_LINE_SIZE = 100;
  wchar_t commandLine[COMMAND_LINE_SIZE];
  STARTUPINFO si;
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);
  wsprintf(commandLine, L"Pipelining-printer.exe %d %s %s", reinterpret_cast<int>(hInheritReadPipeSorter), enableReadSortedDataName, enableWriteNameSortedData);
  PROCESS_INFORMATION pi;

  if (!CreateProcess(NULL, commandLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
  {
    cerr << "Create process failed!" << endl;
    system("pause");
    return GetLastError();
  }
  CloseHandle(hInheritReadPipeSorter);

  vector<int> vec;
  for (int i = 0; i < 10; i++)
  {
    WaitForSingleObject(hEnableReadGeneratedData, INFINITE);
    size_t size;
    DWORD bytesReaded;
    if (!ReadFile(hReadPipeGenerator, &size, sizeof(size), &bytesReaded, NULL))
    {
      cerr << "Read file failed." << endl;
      system("pause");
      return GetLastError();
    }
    vec.resize(size);
    if (!ReadFile(hReadPipeGenerator, vec.data(), vec.size() * sizeof(int), &bytesReaded, NULL))
    {
      cerr << "Read file failed." << endl;
      system("pause");
      return GetLastError();
    }

    std::sort(begin(vec), end(vec));
    cout << "Sorted " << i << '\n';
    SetEvent(hEnableWriteGeneratedData);

    WaitForSingleObject(hEnableWriteSorted, INFINITE);
    DWORD bytesWrite;
    if (!WriteFile(hWritePipeSorter, &size, sizeof(size), &bytesWrite, NULL))
    {
      cerr << "Write to file failed!" << endl;
      system("pause");
      return GetLastError();
    }

    if (!WriteFile(hWritePipeSorter, vec.data(), vec.size() * sizeof(int), &bytesWrite, NULL))
    {
      cerr << "Write to file failed!" << endl;
      system("pause");
      return GetLastError();
    }
    SetEvent(hEnableReadSorted);
  }

  WaitForSingleObject(pi.hProcess, INFINITE);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  CloseHandle(hEnableReadGeneratedData);
  CloseHandle(hEnableWriteGeneratedData);
  CloseHandle(hWritePipeSorter);
  system("pause");
  return 0;
}