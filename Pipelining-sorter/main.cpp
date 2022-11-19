#include <iostream>
#include <vector>
#include <algorithm>

#include <Windows.h>
using namespace std;

int wmain(int argc, wchar_t* argv[])
{
  if (argc < 3)
  {
    cerr << "Few arguments!" << endl;
    system("pause");
    return -1;
  }

  auto hReadPipe = reinterpret_cast<HANDLE>(_wtoi(argv[1]));
  const wchar_t* enableReadName = argv[2];
  // wchar_t enableReadName[] = L"EnableReadGeneratedData";

  HANDLE hEnableRead = OpenEvent(EVENT_ALL_ACCESS, FALSE, enableReadName);
  
  //HANDLE hEnableRead = CreateEvent(NULL, FALSE, FALSE, enableReadName);
  if (hEnableRead == nullptr)
  {
    cerr << "Open event fail! " << GetLastError() << endl;
    system("pause");
    return GetLastError();
  }

  WaitForSingleObject(hEnableRead, INFINITE);
  constexpr size_t SIZE = 1000;
  vector<int> vec(SIZE);
  DWORD bytesReaded;
  if (!ReadFile(hReadPipe, vec.data(), vec.size() * sizeof(int), &bytesReaded, NULL))
  {
    cerr << "Read file failed." << endl;
    system("pause");
    return GetLastError();
  }

  std::sort(begin(vec), end(vec));
  for (const auto i : vec)
    cout << i << '\n';

  CloseHandle(hEnableRead);
  system("pause");
  return 0;
}