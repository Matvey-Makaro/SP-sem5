#include <vector>
#include <iostream>

template<typename T>
using Matrix = std::vector<std::vector<T>>;

template<typename T>
std::ostream& operator<<(std::ostream& out, const Matrix<T>& matrix)
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
