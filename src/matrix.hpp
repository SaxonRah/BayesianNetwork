#ifndef BNI_MATRIX_HPP
#define BNI_MATRIX_HPP

#include <vector>
#include <boost/multi_array.hpp>

namespace bn {

// ����:
// �A�N�Z�X�� y-x
typedef boost::multi_array<double, 2> matrix_type;


} // namespace bn

// �s�񓯎m�E�s��Ɛ����̐�(��`)
bn::matrix_type operator*(bn::matrix_type const& lhs, bn::matrix_type const& rhs);
template<class T> bn::matrix_type operator*(bn::matrix_type const& rhs, T const& lhs);
template<class T> bn::matrix_type operator*(T const& lhs, bn::matrix_type const& rhs);

#include "matrix_impl.hpp"

#endif // #ifndef BNI_MATRIX_HPP

