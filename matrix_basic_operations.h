#include <vector>
#include "iostream"

template<typename T>
using Matrix = std::vector<std::vector<T>>;

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
      for (size_t k = 0; k < M; k++)
        sum += lhs[i][k] * rhs[k][j];
      result[i][j] = sum;
    }
  }

  return result;
}