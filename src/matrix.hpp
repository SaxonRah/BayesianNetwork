#ifndef BNI_MATRIX_HPP
#define BNI_MATRIX_HPP

#include <vector>
#include <boost/multi_array.hpp>

namespace bn {

// ����:
// �A�N�Z�X�� y-x
using matrix_type = boost::multi_array<double, 2>;

// �s�񓯎m�E�s��Ɛ����̐�(��`)
matrix_type operator*(matrix_type const& lhs, matrix_type const& rhs);
template<class T> matrix_type operator*(matrix_type const& rhs, T const& lhs);
template<class T> matrix_type operator*(T const& lhs, matrix_type const& rhs);

} // namespace bn

#include "matrix_impl.hpp"

#endif // #ifndef BNI_MATRIX_HPP

