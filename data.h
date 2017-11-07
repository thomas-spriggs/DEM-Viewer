
typedef double z_depth;
typedef unsigned int uint;
typedef unsigned char ubyte;
typedef unsigned short uint16;


namespace data {
	using namespace linear_math;
	template<typename scalar_t = int>
	struct xyz_triangle {
		
		xyz_point<scalar_t> _p1;
		xyz_point<scalar_t> _p2;
		xyz_point<scalar_t> _p3;
		xyz_triangle( xyz_point<scalar_t> p1, xyz_point<scalar_t> p2, xyz_point<scalar_t> p3 ) :
			_p1(p1),
			_p2(p2),
			_p3(p3)
		{}
		xyz_triangle() {};
	};

	template<typename scalar_t = int>
	struct xyz_triangle_references {
		int _p1;
		int _p2;
		int _p3;
		xyz_triangle_references( int p1, int p2, int p3 ) :
			_p1(p1),
			_p2(p2),
			_p3(p3)
		{}
		xyz_triangle_references() {};
	};
	

	//template<typename scalar_t = int>
	//struct xyz_triangle_pointer {
	//	xyz_point<scalar_t> * _p1;
	//	xyz_point<scalar_t> * _p2;
	//	xyz_point<scalar_t> * _p3;
	//	xyz_triangle( xyz_point<scalar_t> * p1, xyz_point<scalar_t> * p2, xyz_point<scalar_t> * p3 ) :
	//		_p1(p1),
	//		_p2(p2),
	//		_p3(p3)
	//	{}
	//	xyz_triangle() {};
	//};
	//

	const double PI = 3.1415926535897932384626433832795;
}

	//                      BLUE  GREEN      RED         ALPHA
const int colour_white  = 255 | 255 << 8 | 255 << 16 | 255 << 24;
const int colour_red    = 0   | 0   << 8 | 255 << 16 | 255 << 24;
const int colour_blue   = 255 | 0   << 8 | 0   << 16 | 255 << 24;
const int colour_green  = 0   | 255 << 8 | 0   << 16 | 255 << 24;
const int colour_orange = 0   | 63  << 8 | 255 << 16 | 255 << 24;

union rgb {
	int value;
	struct {
		unsigned char _b;
		unsigned char _g;
		unsigned char _r;
		unsigned char _a;
	} colour;
	rgb()
	{
		colour._a=255;
	};
};

