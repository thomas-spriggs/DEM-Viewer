#include "linear_math.h"
#include "error.h"
#include <math.h>

namespace linear_math {

	matrix_3x3<double> generate_rotation_matrix( axis axis_of_rotation, double angle )
	{
		matrix_3x3<double> result( 1 );
		switch( axis_of_rotation ) {
		case x_axis:
			result._e33 = result._e22 = cos( angle );
			result._e23 = sin( angle );
			result._e32 = -result._e23;
			break;
		case y_axis:
			result._e33 = result._e11 = cos( angle );
			result._e31 = sin( angle );
			result._e13 = -result._e31;
			break;
		case z_axis:
			result._e22 = result._e11 = cos( angle );
			result._e12 = sin( angle );
			result._e21 = -result._e12;
			break;
		default:
			error_warning( "Attempted to generate rotation matrix about invalid axis" );
			break;
		}
		return result;
	}

	matrix_3x3<double> generate_rotation_matrix( const double pitch, const double yaw, const double roll )
	{
		matrix_3x3<double> result;
		result._e11 = cos(pitch)*cos(roll) - sin(pitch)*cos(yaw)*sin(roll); result._e12 = -sin(pitch)*cos(roll) - cos(pitch)*cos(yaw)*sin(roll); result._e13 = sin(yaw)*sin(roll) ;
		result._e21 = cos(pitch)*sin(roll) + sin(pitch)*cos(yaw)*cos(roll); result._e22 = -sin(pitch)*sin(roll) + cos(pitch)*cos(yaw)*cos(roll); result._e23 = -sin(yaw)*cos(roll);
		result._e31 = sin(pitch)*sin(yaw)                                 ; result._e32 = cos(pitch)*sin(yaw)                                  ; result._e33 = cos(yaw)           ;
		return result;
	}

}


