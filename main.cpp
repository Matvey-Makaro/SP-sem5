#include <iostream>
#include <vector>
#include <ctime>
#include <cassert>
#include <Windows.h>

#include "profile.h"

using namespace std;

template<typename T>
using Matrix = vector<vector<T>>;

template<typename T>
void fill_matrix(Matrix<T>& matrix)
{
  for (auto& row : matrix)
    for (auto& element : row)
      element = rand() % 100;
}

template<typename T>
void resize_matrix(Matrix<T>& matrix, size_t rows_num, size_t cols_num)
{
  matrix.resize(rows_num);
  for (auto& row : matrix)
    row.resize(cols_num);
}

template<typename T>
ostream& operator<<(ostream& out, const Matrix<T>& matrix)
{
  for (const auto& row : matrix)
  {
    for (const auto& el : row)
      out << el << ' ';
    out << '\n';
  }
    
      
  return out;
}

template<typename T>
bool operator==(const Matrix<T>& lhs, const Matrix<T>& rhs)
{
  if (lhs.size() != rhs.size())
    return false;

  for (size_t i = 0; i < lhs.size(); i++)
  {
    if (lhs[i].size() != rhs[i].size())
      return false;

    for (size_t j = 0; j < lhs[i].size(); j++)
    {
      if (lhs[i][j] != rhs[i][j])
        return false;
    }
  }

  return true;
}

template<typename T>
bool operator!=(const Matrix<T>& lhs, const Matrix<T>& rhs)
{
  return !(lhs == rhs);
}

template<typename T>
Matrix<T> multiplication_in_one_thread(const Matrix<T>& lhs, const Matrix<T>& rhs)
{
  assert(lhs.size() == rhs.size());
  if (lhs.size() == 0)
    return {};
  assert(lhs.size() == lhs[0].size());
  assert(rhs.size() == rhs[0].size());


  const size_t M = lhs.size();
  Matrix<T> result;
  resize_matrix(result, M, M);

  for (size_t i = 0; i < M; i++)
  {
    
    for (size_t j = 0; j < M; j++)
    {
      int sum = 0;
      for(size_t k = 0; k < M; k++)
        sum += lhs[i][k] * rhs[k][j];
      result[i][j] = sum;
    }
  }

  return result;
}

template<typename T>
struct ParamsForMultiply
{
  ParamsForMultiply(const Matrix<T>& lhs, const Matrix<T>& rhs, Matrix<T>& result, int start_idx, int N) :
    lhs(lhs), rhs(rhs), result(result), start_idx(start_idx), N(N) {}

  const Matrix<T>& lhs;
  const Matrix<T>& rhs;
  Matrix<T>& result;

  int start_idx;
  int N;
};

DWORD WINAPI thread_for_multiply(PVOID pvParam)
{
  auto* const params = reinterpret_cast<ParamsForMultiply<int>*>(pvParam);
  const Matrix<int>& lhs = params->lhs;
  const Matrix<int>& rhs = params->rhs;
  Matrix<int>& result = params->result;
  const int start_idx = params->start_idx;
  const int N = params->N;
  const int M = lhs.size();

  for (int i = start_idx; i < M; i += N)
  {
    for (int j = 0; j < M; j++)
    {
      int sum = 0;
      for (int k = 0; k < M; k++)
        sum += lhs[i][k] * rhs[k][j];
      result[i][j] = sum;
      // Sleep(1); // Сильно портит производительность.
    }
  }

  return 0;
}

template<typename T>
Matrix<T> multiplication_in_multiple_threads(const Matrix<T>& lhs, const Matrix<T>& rhs)
{
  assert(lhs.size() == rhs.size());
  if (lhs.size() == 0)
    return {};
  assert(lhs.size() == lhs[0].size());
  assert(rhs.size() == rhs[0].size());
  assert(lhs.size() >= 8);

  const size_t M = lhs.size();
  Matrix<T> result;
  resize_matrix(result, M, M);

#if 1
  const int N = M / 8;
#else
  const int N = 16;
#endif

  vector<HANDLE> handles(N);
  std::vector<ParamsForMultiply<int>*> params_vec(N);
  for (int i = 0; i < N; i++)
  {
    params_vec[i] = new ParamsForMultiply<int>(lhs, rhs, result, i, N);
    handles[i] = CreateThread(NULL, 0, thread_for_multiply, (void*)params_vec[i], 0, NULL);
  }

  for (auto handle : handles)
    WaitForSingleObject(handle, INFINITE);

  for (auto handle : handles)
    CloseHandle(handle);

  for (auto* p : params_vec)
    delete p;

  return result;
}

template<typename T>
struct FiberParams
{
  FiberParams(const Matrix<T>& lhs, const Matrix<T>& rhs, Matrix<T>& result, int start_idx, int N, LPVOID fiber_context) :
    params(new ParamsForMultiply<T>(lhs, rhs, result, start_idx, N)), fiber_context(fiber_context) {}

  ~FiberParams()
  {
    delete params;
  }

  ParamsForMultiply<T>* params;
  LPVOID fiber_context;

};

void WINAPI fiber_for_multiply(LPVOID pvParam)
{
  auto* const fiber_params = reinterpret_cast<FiberParams<int>*>(pvParam);
  auto* const params = fiber_params->params;
  const Matrix<int>& lhs = params->lhs;
  const Matrix<int>& rhs = params->rhs;
  Matrix<int>& result = params->result;
  const int start_idx = params->start_idx;
  const int N = params->N;
  const int M = lhs.size();

  for (int i = start_idx; i < M; i += N)
  {
    for (int j = 0; j < M; j++)
    {
      int sum = 0;
      for (int k = 0; k < M; k++)
        sum += lhs[i][k] * rhs[k][j];
      result[i][j] = sum;
      SwitchToFiber(fiber_params->fiber_context);
    }
  }
}

template<typename T>
Matrix<T> multiplication_with_fibers(const Matrix<T>& lhs, const Matrix<T>& rhs)
{
  auto cur_fib = ConvertThreadToFiber(NULL);

  assert(lhs.size() == rhs.size());
  if (lhs.size() == 0)
    return {};
  assert(lhs.size() == lhs[0].size());
  assert(rhs.size() == rhs[0].size());

  constexpr int K = 8;
  assert(lhs.size() >= K);

  const size_t M = lhs.size();
  Matrix<T> result;
  resize_matrix(result, M, M);

  const int N = M / K;
  vector<LPVOID> fibers(N);
  std::vector<FiberParams<int>*> params_vec(N);
  for (int i = 0; i < N; i++)
  {
    params_vec[i] = new FiberParams<int>(lhs, rhs, result, i, N, cur_fib);
    fibers[i] = CreateFiber(0, fiber_for_multiply, params_vec[i]);
  }

  for (int i = 0; i < M * M; i++)
  {
    SwitchToFiber(fibers[i % N]);
  }

  for (auto f : fibers)
    DeleteFiber(f);

  for (auto* p : params_vec)
    delete p;

  return result;
}

int main()
{
  srand(time(nullptr));

  constexpr int M = 160;

  Matrix<int> lhs, rhs;
  resize_matrix(lhs, M, M);
  resize_matrix(rhs, M, M);
  fill_matrix(lhs);
  fill_matrix(rhs);

  // cout << "First matrix:\n" << lhs << endl;
  // cout << '\n' << "Second matrix:\n" << rhs;

  Matrix<int> one_thread_result, multiple_thread_result, fiber_result;

  {
    LOG_DURATION("One thread");
    one_thread_result = multiplication_in_one_thread(lhs, rhs);
  }

  {
    LOG_DURATION("Multiple thread");
    multiple_thread_result = multiplication_in_multiple_threads(lhs, rhs);
  }
  
  {
    LOG_DURATION("Fiber");
    fiber_result = multiplication_with_fibers(lhs, rhs);
  }

  if (one_thread_result == multiple_thread_result)
    cout << "Multiple result is correct." << endl;
  else cout << "Multiple result is not correct." << endl;

  if (one_thread_result == fiber_result)
    cout << "Fiber result is correct." << endl;
  else cout << "Fiber result isn't correct." << endl;

  return 0;
}
