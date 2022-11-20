#include <iostream>
#include <fstream>
#include <vector>
#include <string>
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

  auto hReadPipeSorter = reinterpret_cast<HANDLE>(_wtoi(argv[1]));
  const wchar_t* enableReadSortedDataName = argv[2];
  const wchar_t* enableWriteSortedDataName = argv[3];

  HANDLE hEnableReadSortedData = OpenEvent(EVENT_ALL_ACCESS, FALSE, enableReadSortedDataName);
  if (hEnableReadSortedData == nullptr)
  {
    cerr << "Open event fail! " << GetLastError() << endl;
    system("pause");
    return GetLastError();
  }

  HANDLE hEnableWriteSortedData = OpenEvent(EVENT_ALL_ACCESS, FALSE, enableWriteSortedDataName);
  if (hEnableWriteSortedData == nullptr)
  {
    cerr << "Open event fail! " << GetLastError() << endl;
    system("pause");
    return GetLastError();
  }

  vector<int> vec;
  for (int i = 0; i < 10; i++)
  {
    WaitForSingleObject(hEnableReadSortedData, INFINITE);
    size_t size;
    DWORD bytesReaded;
    if (!ReadFile(hReadPipeSorter, &size, sizeof(size), &bytesReaded, NULL))
    {
      cerr << "Read file failed." << endl;
      system("pause");
      return GetLastError();
    }
    vec.resize(size);
    if (!ReadFile(hReadPipeSorter, vec.data(), vec.size() * sizeof(int), &bytesReaded, NULL))
    {
      cerr << "Read file failed." << endl;
      system("pause");
      return GetLastError();
    }

    const string fName = "Vec " + to_string(i) + string(".txt");
    ofstream fs(fName, ios::trunc);
    if (!fs.is_open())
    {
      cerr << "File \'" << fName << "\' doesn't open!" << endl;
      system("pause");
      return -1;
    }

    for (const auto& el : vec)
      fs << el << '\n';
    fs.close();

    cout << "Print " << i << '\n';
    SetEvent(hEnableWriteSortedData);
  }
  CloseHandle(hEnableReadSortedData);
  CloseHandle(hEnableWriteSortedData);
  system("pause");
  return 0;
}