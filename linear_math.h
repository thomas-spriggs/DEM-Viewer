#pragma once
#include <cmath>

namespace linear_math
{
	template<typename scalar_t = int>
	struct xyz_point {
		scalar_t _x;
		scalar_t _y;
		scalar_t _z;
		xyz_point( scalar_t x, scalar_t y, scalar_t z ) :
			_x(x),
			_y(y),
			_z(z)
		{}
		template<typename s>
		xyz_point( xyz_point<s> const & source ) :
			_x(source._x),
			_y(source._y),
			_z(source._z)
		{}
		//xyz_point( xyz_point<int> const & source ) :
		//	_x(source._x),
		//	_y(source._y),
		//	_z(source._z)
		//{}
		xyz_point & operator*=( scalar_t scalar ) {
			_x *= scalar;
			_y *= scalar;
			_z *= scalar;
			return *this;
		}
		xyz_point & operator+=( xyz_point<scalar_t> const & operand ) {
			xyz_point<scalar_t> result;
			_x += operand._x;
			_y += operand._y;
			_z += operand._z;
			return *this;
		}
		xyz_point & operator-=( xyz_point<scalar_t> const & operand ) {
			xyz_point<scalar_t> result;
			_x -= operand._x;
			_y -= operand._y;
			_z -= operand._z;
			return *this;
		}
		xyz_point operator-( xyz_point<scalar_t> const & operand ) const {
			xyz_point<scalar_t> result;
			result._x = _x - operand._x;
			result._y = _y - operand._y;
			result._z = _z - operand._z;
			return result;
		}
		xyz_point operator-() {
			xyz_point<scalar_t> result;
			result._x = -_x;
			result._y = -_y;
			result._z = -_z;
			return result;
		}
		xyz_point() {};
	};

	template<typename scalar_t = int>
	struct xy_point {
		scalar_t _x;
		scalar_t _y;
		xy_point() {};
		xy_point( scalar_t x, scalar_t y ) : _x(x), _y(y) {};
		//template<typename s>
		//xy_point( xy_point<s> const & source ) :
		//	_x(scalar_t(source._x)),
		//	_y(scalar_t(source._y))
		//{};
		xy_point( xy_point<double> const & source ) :
			_x(scalar_t(source._x)),
			_y(scalar_t(source._y))
		{};
		xy_point( xy_point<int> const & source ) :
			_x(scalar_t(source._x)),
			_y(scalar_t(source._y))
		{};
		explicit xy_point( xyz_point<double> const & source ) :
			_x(source._x),
			_y(source._y)
		{};
		//explicit xy_point( xyz_point<scalar_t> const & source ) :
		//	_x(source._x),
		//	_y(source._y)
		//{};
		xy_point & operator*=( scalar_t const & scalar ) {
			_x *= scalar;
			_y *= scalar;
			return *this;
		};
		xy_point & operator/=( scalar_t const & scalar ) {
			_x /= scalar;
			_y /= scalar;
			return *this;
		};
		xy_point & operator+=( xy_point<scalar_t> const & operand ) {
			_x += operand._x;
			_y += operand._y;
			return *this;
		};
	};

	template<typename T> xyz_point<T> cross_product(xyz_point<T> const & a, xyz_point<T> const & b)
	{
		xyz_point<T> result;
		result._x = a._y * b._z - a._z * b._y;
		result._y = a._z * b._x - a._x * b._z;
		result._z = a._x * b._y - a._y * b._x;
		return result;
	};

	template<typename T> T length(xyz_point<T> const & a)
	{
		T result;
		result = a._x * a._x + a._y * a._y + a._z * a._z;
		return sqrt(result);
	};

		// returns the square of the length
	template<typename T> T length2(xyz_point<T> const & a)
	{
		T result;
		result = a._x * a._x + a._y * a._y + a._z * a._z;
		return result;
	};

	template<typename T> T dot_product(xyz_point<T> const & a, xyz_point<T> const & b)
	{
		T result;
		result = a._x * b._x + a._y * b._y + a._z * b._z;
		return result;
	};

	template<typename scalar_t = double>
	struct matrix_2x2 {
		scalar_t _e11; // _e11 _e12
		scalar_t _e12; // _e21 _e22
		scalar_t _e21;
		scalar_t _e22;
		matrix_2x2() {};
		matrix_2x2( scalar_t leading_diagonal ) :
			_e11(leading_diagonal),
			_e12(0),
			_e21(0),
			_e22(leading_diagonal)
		{};
		matrix_2x2( scalar_t e11, scalar_t e12, scalar_t e21, scalar_t e22 ) :
			_e11(e11),
			_e12(e12),
			_e21(e21),
			_e22(e22)
		{};
		scalar_t determinant() {
			return (_e11 * _e22) - (_e12 * _e21);
		}
	};

	template<typename scalar_t = double>
	struct matrix_3x3 {
		scalar_t _e11; // _e11 _e12 _e13
		scalar_t _e12; // _e21 _e22 _e23
		scalar_t _e13; // _e31 _e32 _e33
		scalar_t _e21;
		scalar_t _e22;
		scalar_t _e23;
		scalar_t _e31;
		scalar_t _e32;
		scalar_t _e33;
		matrix_3x3() {};
		matrix_3x3( scalar_t leading_diagonal ) :
			_e11(leading_diagonal),
			_e12(0),
			_e13(0),
			_e21(0),
			_e22(leading_diagonal),
			_e23(0),
			_e31(0),
			_e32(0),
			_e33(leading_diagonal)
		{};
		matrix_3x3 operator*( matrix_3x3<scalar_t> const & operand ) {
			matrix_3x3 product;
			product._e11 = _e11*operand._e11 + _e12*operand._e21 + _e13*operand._e31;
			product._e21 = _e21*operand._e11 + _e22*operand._e21 + _e23*operand._e31;
			product._e31 = _e31*operand._e11 + _e32*operand._e21 + _e33*operand._e31;

			product._e12 = _e11*operand._e12 + _e12*operand._e22 + _e13*operand._e32;
			product._e22 = _e21*operand._e12 + _e22*operand._e22 + _e23*operand._e32;
			product._e32 = _e31*operand._e12 + _e32*operand._e22 + _e33*operand._e32;

			product._e13 = _e11*operand._e13 + _e12*operand._e23 + _e13*operand._e33;
			product._e23 = _e21*operand._e13 + _e22*operand._e23 + _e23*operand._e33;
			product._e33 = _e31*operand._e13 + _e32*operand._e23 + _e33*operand._e33;
			return product;
		};
		xyz_point<scalar_t> operator*( xyz_point<scalar_t> const & operand ) {
			xyz_point<scalar_t> result;
			result._x = _e11*operand._x + _e12*operand._y + _e13*operand._z;
			result._y = _e21*operand._x + _e22*operand._y + _e23*operand._z;
			result._z = _e31*operand._x + _e32*operand._y + _e33*operand._z;
			return result;
		};
		scalar_t determinant() {
			return (_e11 * _e22 * _e33) - (_e11 * _e23 * _e32) - (_e12 * _e21 * _e33) + (_e12 * _e23 * _e31) + (_e13 * _e21 * _e32) - (_e13 * _e22 * _e31);
		};
		matrix_3x3 operator-() { // matrix inverse
			matrix_3x3 inverse;
      scalar_t divisor = this->determinant();
			inverse._e11 = matrix_2x2< scalar_t >(_e22, _e23, _e32, _e33).determinant() / divisor;
			inverse._e12 = matrix_2x2< scalar_t >(_e13, _e12, _e33, _e32).determinant() / divisor;
			inverse._e13 = matrix_2x2< scalar_t >(_e12, _e13, _e22, _e23).determinant() / divisor;

			inverse._e21 = matrix_2x2< scalar_t >(_e23, _e21, _e33, _e31).determinant() / divisor;
			inverse._e22 = matrix_2x2< scalar_t >(_e11, _e13, _e31, _e33).determinant() / divisor;
			inverse._e23 = matrix_2x2< scalar_t >(_e13, _e11, _e23, _e21).determinant() / divisor;

			inverse._e31 = matrix_2x2< scalar_t >(_e21, _e22, _e31, _e32).determinant() / divisor;
			inverse._e32 = matrix_2x2< scalar_t >(_e12, _e11, _e32, _e31).determinant() / divisor;
			inverse._e33 = matrix_2x2< scalar_t >(_e11, _e12, _e21, _e22).determinant() / divisor;
			return inverse;
		}
	};

	enum axis {
		x_axis,
		y_axis,
		z_axis
	};

	enum rotational_direction {
		clockwise,
		anti_clockwise
	};

	matrix_3x3<double> generate_rotation_matrix( axis axis_of_rotation, double angle = 0.1 );
	matrix_3x3<double> generate_rotation_matrix( double pitch, double yaw, double roll );
}
