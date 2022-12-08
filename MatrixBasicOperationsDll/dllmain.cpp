// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

using Matrix = std::vector<std::vector<int>>;

extern "C" __declspec(dllexport) void fill_matrix(Matrix & matrix)
{
  for (auto& row : matrix)
    for (auto& element : row)
      element = rand() % 100;
}

extern "C" __declspec(dllexport) void resize_matrix(Matrix& matrix, size_t rows_num, size_t cols_num)
{
  matrix.resize(rows_num);
  for (auto& row : matrix)
    row.resize(cols_num);
}


extern "C" __declspec(dllexport) Matrix multiplication_in_one_thread(const Matrix& lhs, const Matrix& rhs)
{
  assert(lhs.size() == rhs.size());
  if (lhs.size() == 0)
    return {};
  assert(lhs.size() == lhs[0].size());
  assert(rhs.size() == rhs[0].size());


  const size_t M = lhs.size();
  Matrix result;

  resize_matrix(result, M, M);

  for (size_t i = 0; i < M; i++)
  {

    for (size_t j = 0; j < M; j++)
    {
      int sum = 0;
      for (size_t k = 0; k < M; k++)
        sum += lhs[i][k] * rhs[k][j];
      result[i][j] = sum;
    }
  }

  return result;
}
