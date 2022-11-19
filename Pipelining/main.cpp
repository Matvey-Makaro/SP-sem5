#include <iostream>
#include <ctime>
#include <vector>
#include <Windows.h>
using namespace std;

void fillVec(vector<int>& vec)
{
  for (auto& el : vec)
    el = rand();
}

int main()
{
  srand(time(NULL));

  constexpr size_t SIZE = 1000;
  vector<int> vec(SIZE);
  fillVec(vec);

  constexpr size_t COMMAND_LINE_SIZE = 100;
  wchar_t commandLine[COMMAND_LINE_SIZE];

  HANDLE hWritePipe, hReadPipe;

  if (!CreatePipe(&hReadPipe, &hWritePipe, NULL, 0))
  {
    cerr << "Create pipe failed!" << endl;
    system("pause");
    return GetLastError();
  }

  HANDLE hInheritReadPipe;
  if (!DuplicateHandle(GetCurrentProcess(), hReadPipe, GetCurrentProcess(), &hInheritReadPipe, 0, TRUE, DUPLICATE_SAME_ACCESS))
  {
    cerr << "Duplicate handle error!" << endl;
    system("pause");
    return GetLastError();
  }
  CloseHandle(hReadPipe);

  wchar_t enableReadName[] = L"EnableReadGeneratedData";
  HANDLE hEnableRead = CreateEvent(NULL, FALSE, FALSE, enableReadName);
  if (hEnableRead == NULL)
  {
    cerr << "Create event failed." << endl;
    system("pause");
    return GetLastError();
  }

  STARTUPINFO si;
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);
  wsprintf(commandLine, L"Pipelining-sorter.exe %d %s", reinterpret_cast<int>(hInheritReadPipe), enableReadName);
  PROCESS_INFORMATION pi;

  
  
  if (!CreateProcess(NULL, commandLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
  {
    cerr << "Create process failed!" << endl;
    system("pause");
    return GetLastError();
  }
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  CloseHandle(hInheritReadPipe);

  DWORD bytesWrite;
  if (!WriteFile(hWritePipe, vec.data(), vec.size() * sizeof(int), &bytesWrite, NULL))
  {
    cerr << "Write to file failed!" << endl;
    system("pause");
    return GetLastError();
  }

  SetEvent(hEnableRead);
  Sleep(1000);
  CloseHandle(hEnableRead);
  CloseHandle(hWritePipe);
  system("pause");
  return 0;
}