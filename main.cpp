#include <iostream>
#include <vector>
#include <ctime>
#include <fstream>
#include <exception>
#include <Windows.h>
#include <algorithm>

#include "profile.h"

using namespace std;

void fill_file_random_async(const std::wstring& fName, int amount, int max_number)
{
  auto h_end_write = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (h_end_write == NULL)
    throw runtime_error("Create event return NULL.");

  OVERLAPPED ovrl;
  ovrl.Offset = 0;
  ovrl.OffsetHigh = 0;
  ovrl.hEvent = h_end_write;

  auto hFile = CreateFile(fName.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
  if (hFile == INVALID_HANDLE_VALUE)
  {
    CloseHandle(h_end_write);
    throw runtime_error("File doesn't open.");
  }


  for (int i = 0; i < amount; i++)
  {
    DWORD bytes_write;
    int num = rand() % max_number;

    if (!WriteFile(hFile, &num, sizeof(num), &bytes_write, &ovrl))
    {
      if (GetLastError() != ERROR_IO_PENDING)
      {
        CloseHandle(h_end_write);
        CloseHandle(hFile);
        throw runtime_error("Write error.");
      }
        
    }

    WaitForSingleObjectEx(h_end_write, INFINITE, TRUE);
    ovrl.Offset += sizeof(num);
  }

  CloseHandle(h_end_write);
  CloseHandle(hFile);
}

std::vector<int> read_file_async(const std::wstring& fName)
{
  auto h_end_read = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (h_end_read == NULL)
    throw runtime_error("Create event return NULL.");

  OVERLAPPED ovrl;
  ovrl.Offset = 0;
  ovrl.OffsetHigh = 0;
  ovrl.hEvent = h_end_read;

  auto hFile = CreateFile(fName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
  if (hFile == INVALID_HANDLE_VALUE)
  {
    CloseHandle(h_end_read);
    throw runtime_error("File doesn't open.");
  }


  auto file_size = GetFileSize(hFile, NULL);
  auto num_of_elements = file_size / sizeof(int);
  std::vector<int> result;
  result.reserve(num_of_elements);

  for(int i = 0; i < num_of_elements; i++)
  {
    DWORD bytes_read;
    int buff;

    if (!ReadFile(hFile, &buff, sizeof(buff), &bytes_read, &ovrl))
    {
      
      if (GetLastError() == ERROR_HANDLE_EOF)
        break;
      else if (GetLastError() != ERROR_IO_PENDING)
      {
        CloseHandle(h_end_read);
        CloseHandle(hFile);
        throw runtime_error("Read error.");
      }
    }

    WaitForSingleObjectEx(h_end_read, INFINITE, TRUE);
    ovrl.Offset += sizeof(buff);
    result.push_back(buff);
  }

  CloseHandle(h_end_read);
  CloseHandle(hFile);
  return result;
}

void write_file_async(const std::wstring& fName, const std::vector<int>& vec)
{
  auto h_end_write = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (h_end_write == NULL)
    throw runtime_error("Create event return NULL.");

  OVERLAPPED ovrl;
  ovrl.Offset = 0;
  ovrl.OffsetHigh = 0;
  ovrl.hEvent = h_end_write;

  auto hFile = CreateFile(fName.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
  if (hFile == INVALID_HANDLE_VALUE)
  {
    CloseHandle(h_end_write);
    throw runtime_error("File doesn't open.");
  }


  for (int i = 0; i < vec.size(); i++)
  {
    DWORD bytes_write;
    int num = vec[i];

    if (!WriteFile(hFile, &num, sizeof(num), &bytes_write, &ovrl))
    {
      if (GetLastError() != ERROR_IO_PENDING)
      {
        CloseHandle(h_end_write);
        CloseHandle(hFile);
        throw runtime_error("Write error.");
      }

    }

    WaitForSingleObjectEx(h_end_write, INFINITE, TRUE);
    ovrl.Offset += sizeof(num);
  }

  CloseHandle(h_end_write);
  CloseHandle(hFile);
}

int comp(const int* i, const int* j)
{
  return *i - *j;
}

void sort_file_with_mapping(const std::wstring& fName)
{
  auto hFile = CreateFile(fName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE)
    throw runtime_error("File doesn't open.");

  auto file_mapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
  if (file_mapping == NULL)
  {
    CloseHandle(hFile);
    throw runtime_error("File mappint doesn't create.");
  }

  int* view = (int*)MapViewOfFile(file_mapping, FILE_MAP_WRITE, 0, 0, 0);
  if (view == NULL)
  {
    CloseHandle(hFile);
    CloseHandle(file_mapping);
    throw runtime_error("MapViewOfFile return NULL.");
  }

  auto file_size = GetFileSize(hFile, NULL);
  auto num_of_elements = file_size / sizeof(*view);

  qsort(view, num_of_elements, sizeof(*view), (int(*) (const void*, const void*)) comp);

  CloseHandle(hFile);
  CloseHandle(file_mapping);
  UnmapViewOfFile(view);
}

void sort_file_standart(const std::wstring& fName)
{
  std::vector<int> vec;
  {
    LOG_DURATION("READ");
    vec = read_file_async(fName);
  }
  // auto vec = read_file_async(fName);
  auto* data = vec.data();

  {
    LOG_DURATION("SORTING");
    qsort(data, vec.size(), sizeof(*data), (int(*) (const void*, const void*)) comp);
  }
  

  {
    LOG_DURATION("WRITE");
    write_file_async(fName, vec);
  }
}

int main()
{
  srand(time(NULL));
  const std::wstring fName = L"number_map.dat";
  const std::wstring fName_standart = L"number_standart.dat";
  constexpr int num_of_elements = 100000;

  try
  {
    fill_file_random_async(fName, num_of_elements, num_of_elements);
    CopyFile(fName.c_str(), fName_standart.c_str(), FALSE);

    {
      LOG_DURATION("Sort with mapping");
      sort_file_with_mapping(fName);
    }

    {
      LOG_DURATION("Standart sort");
      sort_file_standart(fName_standart);
    }

    auto map_vec = read_file_async(fName);
    if (std::is_sorted(map_vec.cbegin(), map_vec.cend()))
      cout << "Map_vec sorted." << endl;
    else cout << "Map_vec doesn't sorted." << endl;

    auto std_vec = read_file_async(fName_standart);
    if (std::is_sorted(std_vec.cbegin(), std_vec.cend()))
      cout << "Std_vec sorted." << endl;
    else cout << "Std_vec doesn't sorted." << endl;
  }
  catch (const std::exception& ex)
  {
    cerr << ex.what() << endl;
    return -1;
  }

  return 0;
}