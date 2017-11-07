#include "renderer.h"
using namespace linear_math;

// helper function prototypes

int gen_rainbow_colour( int hue_angle ); // angle in degrees
int colour_average( int c1, int c2 );
int colour_average( int c1, int c2, int c3 );

// data and constant definitions

enum colour_states {
	colour_state_white,
	colour_state_grey,
	colour_state_red_blue,
	colour_state_red_purple_blue,
	colour_state_rainbow1,
	colour_state_rainbow2,
	colour_state_rainbow3,
	colour_state_number_of_states,
	colour_state_albedo
};

// method implementations

// constructor
renderer::renderer( digital_elevation_model * source_data, SDL_Surface * target ) :
	//_rotation_step_size( 0.098174770424681038701957605727484 ), // pi/32
	_rotation_step_size( 0.024543692606170259675489401431871 ), // pi/128
	_target( target ),
	_window_size( _target->w, _target->h ),
	_z_buffer( (z_depth *)malloc( sizeof(z_depth) * _target->pitch * _target->h ) ),
	_field_of_view( 90 ),
	_show_far_side( false ),
	_light( 0,0,1 )
{
	switch_source( source_data );
	switch_target( target );
}

void renderer::switch_source( digital_elevation_model * source_data )
{
  _colour_state = colour_state_grey;
	_source_data = source_data;

	// initialise camera position, angle, and movement speeds
	set_default_camera();
	xy_point<> surface_size = _source_data->get_size();
	_step_size = surface_size._y / 200 + surface_size._x / 200;
	if( _step_size <= 0 )
		_step_size = 1;

	generate_vertex_colours();
}

void renderer::switch_target( SDL_Surface * target )
{
	_target = target;
	_screen = target;
	_window_size._x = _target->w;
	_window_size._y = _target->h;
	free( _z_buffer );
  _z_buffer = (z_depth *)malloc( sizeof(z_depth) * _target->pitch * _target->h );
}

// destructor
renderer::~renderer()
{
	//delete [] _z_buffer;
	free( _z_buffer );
}

void renderer::switch_far_side()
{
	_show_far_side = !_show_far_side;
}

void renderer::set_default_camera()
{
	xy_point<> surface_size = _source_data->get_size();
	_camera_position = xyz_point<>( surface_size._x / 2, surface_size._y / 2, surface_size._x > surface_size._y ? surface_size._x : surface_size._y );
	_camera_angle = matrix_3x3<>( 1 );
	//_camera_angle = generate_rotation_matrix( x_axis, data::PI / 2 );
}

void renderer::generate_vertex_colours()
{
	if(_colour_state == colour_state_albedo ) {
    return;                   
  }
	uint length = (uint)_source_data->_original_vertices.size();
	_vertex_colours.resize( length );
	double max = _source_data->get_max();
	for( uint i=0; i<length; ++i ) {
		_vertex_colours[i] = gen_colour( (uint)(_source_data->_original_vertices[i]._z / max * 255) );
	}
}

int convert_8to32( unsigned char colour )
{
  return colour | colour << 8 | colour << 16 | colour << 24;
}

void renderer::load_vertex_colours()
{
	string file = dialog_open_gif( "open albedo map" );
	if( file.empty() ) {
		return;
	}
  SDL_Surface * alb = IMG_Load( file.c_str() ); // alb is short for Albedo
	if( alb == 0 ) {
		error_critical( IMG_GetError() );
	}
  if( alb->h != _source_data->get_surface()->h || alb->w != _source_data->get_surface()->w || alb->pitch != _source_data->get_surface()->pitch ) {
    SDL_FreeSurface( alb );
    error_warning( "Albeido map is not the same size as the DEM" );
    return;
  }
  _vertex_colours.clear();
	uint length = (uint)_source_data->_original_vertices.size();
	//error_warning("loaded gif");
	_vertex_colours.resize( length );
	//error_warning("resized array");
	double max = _source_data->get_max();
	for( uint i=0; i<length; ++i ) {
		_vertex_colours[i] = convert_8to32( ((ubyte*)alb->pixels)[_source_data->_original_vertices[i]._x + alb->pitch * _source_data->_original_vertices[i]._y] );
	}
  SDL_FreeSurface( alb );
	dialog_debug( "gif opened" );
	_colour_state = colour_state_albedo;
}

//void renderer::genrate_triangle_colours( triangle_surface const & source )
//{
//	_triangle_colours.clear();
//	int number_of_triangles = source._triangles.size();
//	for( int i=0; i<number_of_triangles; ++i ) {
//		#ifdef ACCURATE
//			_triangle_colours.push_back( gen_colour( source._triangles[i]._p1._z / 2 ) );
//		#else
//			_triangle_colours.push_back( gen_colour( source._triangles[i]._p1._z + 256 ) );
//		#endif
//	}
//}

void renderer::switch_colours()
{
	_colour_state = (_colour_state + 1) % colour_state_number_of_states;
	generate_vertex_colours();
}

int renderer::gen_colour( int const value )
{
	switch( _colour_state ) {
		case colour_state_white:
			return( colour_white ); // white
			break;
		case colour_state_grey:
			return( value | ( value << 8) | ( value << 16 ) ); // grey
			break;
		case colour_state_red_blue:
			return( value | ( ( 255 - value ) << 16) ); // red / blue
			break;
		case 	colour_state_red_purple_blue:
			if( value > 127 ) {
				return( 255 | ( ( 255 - ((value-127)*2) ) << 16) );
			} else {
				return( value * 2 | ( 255 << 16) );
			}
			break;
		case colour_state_rainbow1:
			return( gen_rainbow_colour( value * 2 % 360 ) ); // rainbow
			break;
		case colour_state_rainbow2:
			return( gen_rainbow_colour( value * 4 % 360 ) ); // rainbow
			break;
		case colour_state_rainbow3:
			return( gen_rainbow_colour( value * 8 % 360 ) ); // rainbow
			break;
		default:
			error_critical( "renderer in invalid colour state" );
			return 0;
			break;
	}
}

//void renderer::render_draw_z()
//{
//	clear_z_buffer();
//	xy_point<> centre( _target->w/2, _target->h/2 );
//	for( uint i = 0; i < _source_data->_transformed_vertices.size(); ++i ) {
//		// put origin at camera
//		xyz_point<double> bar = _source_data->_transformed_vertices[i];
//		bar -= _camera_position;
//		bar = _camera_angle * bar;
//		// remove all points behind the camera
//		if( bar._z < 0 ) {
//			xy_point<double> fred( bar );
//			// depth scale
//			fred *= (double)_target->w;
//			fred /= -bar._z;
//			// put origin at top left of target
//			xy_point<int> foo( fred );
//			foo += centre;
//			// clip points to target size
//			if( foo._x >= 0 && foo._x < _target->w && foo._y >= 0 && foo._y < _target->h ) {
//				int const offset = foo._x + foo._y * _target->pitch / _target->format->BytesPerPixel;
//				int & pixel_i = ((int*)_target->pixels)[offset];
//				// perform z_buffer based occlusion clipping
//				//unsigned int distance_from_camera_squared = (unsigned int)(bar._x*bar._x) + (unsigned int)(bar._y*bar._y) + (unsigned int)(bar._z*bar._z);
//				double distance_from_camera_squared = (double)bar._x*(double)bar._x + (double)bar._y*(double)bar._y + (double)bar._z*(double)bar._z;
//				if( _z_buffer[offset] == 0 || distance_from_camera_squared <= _z_buffer[offset] ) {
//					_z_buffer[offset] = distance_from_camera_squared;
//					//pixel_i = _vertex_colours[i];
//					pixel_i = (int)_z_buffer[offset];
//				}
//			}
//		}
//	}
//}


void renderer::render()
{
	memset( _z_buffer, 0, _target->pitch * _target->h * sizeof( double ) / _target->format->BytesPerPixel );
	xy_point<> centre( _target->w/2, _target->h/2 );
	double depthScaler = tan( degrees2radians( _field_of_view / 2 ) );
	uint length = (uint)_source_data->_original_vertices.size();

	for( uint i = 0; i < length; ++i ) {
		// put origin at camera
		xyz_point<double> bar = _source_data->_transformed_vertices[i];
		bar -= _camera_position;
		bar = _camera_angle * bar;

		// remove all points behind the camera
		if( bar._z >= 0 ) {
			continue;
		}

		// remove all points on the other side of the origin
		if( !_show_far_side ) {
			xyz_point<double> spam = _source_data->_transformed_vertices[i];
			spam = _camera_angle * spam;
			if( spam._z < 0 ) {
				continue;
			}
		}

		// depth scale
		xy_point<double> fred( bar );
		fred *= (double)_target->w;
		fred /= -bar._z * depthScaler;

		// put origin at top left of target
		xy_point<int> foo( fred );
		foo += centre;

		// clip points to target size
		if( !(foo._x >= 0 && foo._x < _target->w && foo._y >= 0 && foo._y < _target->h) ) {
			continue;
		}

		// map coordinate to pixel location
		int const offset = foo._x + foo._y * _target->pitch / _target->format->BytesPerPixel;
		int & pixel_i = ((int*)_target->pixels)[offset];

		// perform z_buffer based occlusion clipping
		double distance_from_camera_squared = (double)bar._x*(double)bar._x + (double)bar._y*(double)bar._y + (double)bar._z*(double)bar._z;
		if( !(_z_buffer[offset] == 0 || distance_from_camera_squared <= _z_buffer[offset]) ) {
			continue;
		}

		// draw pixel
		_z_buffer[offset] = distance_from_camera_squared;
		pixel_i = _vertex_colours[i];
		//pixel_i = i % _window_size._x == 0 ? colour_blue : _vertex_colours[i];
	} // end for
}	// end render


//void renderer::render_no_z()
//{
//	memset( _z_buffer, colour_white, _target->pitch * _target->h * sizeof( unsigned int ) / _target->format->BytesPerPixel );
//	//memset( _z_buffer, 0, _target->pitch * _target->h * sizeof( double ) / _target->format->BytesPerPixel );
//	xy_point<> centre( _target->w/2, _target->h/2 );
//	for( uint i = 0; i < _source_data->_transformed_vertices.size(); ++i ) {
//		// put origin at camera
//		xyz_point<double> bar = _source_data->_transformed_vertices[i];
//		bar -= _camera_position;
//		bar = _camera_angle * bar;
//		// remove all points behind the camera
//		if( bar._z < 0 ) {
//			xy_point<double> fred( bar );
//			// depth scale
//			fred *= (double)_target->w;
//			fred /= -bar._z;
//			// put origin at top left of target
//			xy_point<int> foo( fred );
//			foo += centre;
//			// clip points to target size
//			if( foo._x >= 0 && foo._x < _target->w && foo._y >= 0 && foo._y < _target->h ) {
//				int const offset = foo._x + foo._y * _target->pitch / _target->format->BytesPerPixel;
//				int & pixel_i = ((int*)_target->pixels)[offset];
//				pixel_i = _vertex_colours[i];
//			}
//		}
//	}
//}

bool renderer::window_clipped( xy_point<int> const & point )
{
	return point._x < 0 || point._x >= _window_size._x || point._y < 0 || point._y >= _window_size._y;
}

enum order {
	first,
	second, 
	third
};

int minimum_of_3( int x1, int x2, int x3 )
{
	if( x1 < x2 ) {
		if( x1 < x3 ) {
			return first;
		} else {
			return third;
		}
	} else {
		if( x2 < x3 ) {
			return second;
		} else {
			return third;
		}
	}
}

int maximum_of_3( int x1, int x2, int x3 )
{
	if( x1 > x2 ) {
		if( x1 > x3 ) {
			return first;
		} else {
			return third;
		}
	} else {
		if( x2 > x3 ) {
			return second;
		} else {
			return third;
		}
	}
}

void renderer::set_pixel( int x, int y, int colour )
{
	((int*)_target->pixels)[x + _target->pitch / _target->format->BytesPerPixel * y] = colour;
}

void renderer::set_pixel( int x, int y, int colour, z_depth z )
{
	int offset = x + _target->pitch / _target->format->BytesPerPixel * y;
	if( _z_buffer[offset] == 0 || z <= _z_buffer[offset] ) {
		((int*)_target->pixels)[offset] = colour;
	}
}

void renderer::draw_line( xy_point<int> start, xy_point<int> end, int colour )
{
	int dx = end._x - start._x;
	int dy = end._y - start._y;

	set_pixel( start._x, start._y, colour );
  if( dx != 0 ) {
    float m = (float) dy / (float) dx;
		if( abs(m) < 1 ) {
 			float b = start._y - m*start._x;
			dx = (end._x > start._x) ? 1 : -1;
			while( end._x != start._x ) {
				start._x += dx;
				start._y = (int)(m * start._x + b);
				set_pixel( start._x, start._y, colour );
			}
		} else {
 			float b = start._x - start._y/m;
			dy = (end._y > start._y) ? 1 : -1;
			while( end._y != start._y ) {
				start._y += dy;
				start._x = (int)(start._y/m + b);
				set_pixel( start._x, start._y, colour );
			}
		}
	} else {
		if( dy > 0 ) {
			while( start._y++ != end._y ) {
				set_pixel( start._x, start._y, colour );
			}
		} else {
			while( start._y-- != end._y ) {
				set_pixel( start._x, start._y, colour );
			}
		}
	}
}

void renderer::draw_triangle_frame( xy_point<int> const & p1, xy_point<int> const & p2, xy_point<int> const & p3, int colour )
{
  draw_line( p1, p2, colour );
  draw_line( p2, p3, colour );
  draw_line( p3, p1, colour );
}

  // wireframe
void renderer::render( triangle_surface const & source )
{
	auto number_of_triangles = source._triangles.size();
	int triangle_colour;
	xy_point<> centre( _target->w/2, _target->h/2 );
	for( uint i=0; i<number_of_triangles; ++i ) {
		triangle_colour = _vertex_colours[source._triangles[i]._p1];

		// put origin at camera
		xyz_point<double> bar1 = _source_data->_transformed_vertices[source._triangles[i]._p1];
		bar1 -= _camera_position;
		bar1 = _camera_angle * bar1;
		xyz_point<double> bar2 = _source_data->_transformed_vertices[source._triangles[i]._p2];
		bar2 -= _camera_position;
		bar2 = _camera_angle * bar2;
		xyz_point<double> bar3 = _source_data->_transformed_vertices[source._triangles[i]._p3];
		bar3 -= _camera_position;
		bar3 = _camera_angle * bar3;
		// remove all triangles behind the camera
		if( bar1._z < 0 || bar2._z < 0 || bar3._z < 0 ) {
			xy_point<double> fred1( bar1 );
			// depth scale
			fred1 *= (double)_target->w;
			fred1 /= -bar1._z;
			// put origin at top left of target
			xy_point<int> foo1( fred1 );
			foo1 += centre;

			xy_point<double> fred2( bar2 );
			// depth scale
			fred2 *= (double)_target->w;
			fred2 /= -bar2._z;
			// put origin at top left of target
			xy_point<int> foo2( fred2 );
			foo2 += centre;

			xy_point<double> fred3( bar3 );
			// depth scale
			fred3 *= (double)_target->w;
			fred3 /= -bar3._z;
			// put origin at top left of target
			xy_point<int> foo3( fred3 );
			foo3 += centre;

			// remove all triangles even partially outside the target window size
			if( !window_clipped( foo1 ) && !window_clipped( foo2 ) && !window_clipped( foo3 ) ) {
				draw_triangle_frame( foo1, foo2, foo3, triangle_colour );
			}
		}
	}
}

//class line_coords_r {
//public:
//	line_coords_r( xy_point<int> start, xy_point<int> end ) {
//		deltax = end._x - start._x;           // The difference in the x's
//		deltay = end._y - start._y;           // The difference in the y's
//		h = deltay == 0;
//		x = start._x;
//		xnum = deltay / 2;
//	}
//	int get_next_x() {
//		if( h ) return x + deltax;
//		xnum += deltax;
//		while (xnum >= deltay)
//		{
//			xnum -= deltay;
//			x++;
//		}
//		return x;
//	}
//private:
//  int deltax, deltay, x, xnum;
//	bool h;
//};
//
//class line_coords_l {
//public:
//	line_coords_l( xy_point<int> start, xy_point<int> end ) {
//		deltax = end._x - start._x;           // The difference in the x's
//		deltay = end._y - start._y;           // The difference in the y's
//		h = deltay == 0;
//		x = start._x;
//		xnum = deltay / 2;
//	}
//	int get_next_x() {
//		if( h ) return x + deltax;
//		xnum += deltax;
//		/*if( xnum <= deltay ) {
//			int xinc = deltay % 
//		}*/
//		while (xnum <= deltay) {
//			xnum += deltay;
//			x--;
//		}
//		return x;
//	}
//private:
//  int deltax, deltay, x, xnum;
//	bool h;
//};
//
//class line_coords {
//public:
//	line_coords( xy_point<int> start, xy_point<int> end ) :
//		r( start, end ),
//		l( start, end ),
//		is_left( start._x - end._x > 0 )
//	{
//	}
//	int get_next_x() {
//		if( is_left ) {
//			return l.get_next_x();
//		} else {
//			return r.get_next_x();
//		}
//	}
//private:
//	bool is_left;
//	line_coords_r r;
//	line_coords_l l;
//};

class line_coords {
public:
	line_coords( xy_point<int> start, xy_point<int> end ) :
		deltax2( start._x - end._x > 0 ? -1 : 1 ),
		is_left( start._x - end._x > 0 )
	{
		deltax = end._x - start._x;           // The difference in the x's
		deltay = end._y - start._y;           // The difference in the y's
		h = deltay == 0;
		x = start._x;
		xnum = deltay / 2;
	}
	int get_next_x() {
		if( h ) return x + deltax;
		xnum += deltax;
		if( is_left ) {
			while (xnum <= deltay)
			{
				xnum += deltay;
				x--;
			}
		} else {
			while (xnum >= deltay)
			{
				xnum -= deltay;
				x++;
			}
		}
		return x;
	}
private:
  int deltax, deltax2, deltay, x, xnum;
	bool h, is_left;
};

class z_line {
public:
	z_line( z_depth start, z_depth end, int steps ) :
		z( start ),
		deltaz( (end-start) / steps )
	{}
		z_depth get_next_z() {
			z += deltaz;
			return z;
		}
private:
	z_depth deltaz, z;
};

void renderer::draw_h_line( int y, int x1, int x2, int colour )
{
	int offset = y * _target->pitch / _target->format->BytesPerPixel + x1;
	for( ; x1 <= x2; ++ x1, ++ offset ) {
		((int *)_target->pixels)[offset] = colour;
	}
}

void renderer::draw_h_line( int y, int x1, int x2, int colour, z_depth z1, z_depth z2 )
{
	int offset = y * _target->pitch / _target->format->BytesPerPixel + x1;
	z_line zl( z1, z2, x2 - x1 );
  z_depth z = z1;
	for( ; x1 < x2; ++ x1, ++ offset ) {
		if( _z_buffer[offset] == 0 || z <= _z_buffer[offset] ) {
			((int *)_target->pixels)[offset] = colour;
			_z_buffer[offset] = z;
		}
		z = zl.get_next_z();
	}
}

void renderer::draw_h_line_clipped( int y, int x1, int x2, int colour, z_depth z1, z_depth z2 )
{
	if( y < 0 || y >= _target->h || x1 > _target->w || x2 < 0 ) {
		return;
	}
	int offset = y * _target->pitch / _target->format->BytesPerPixel + x1;
	z_line zl( z1, z2, x2 - x1 );
  z_depth z = z1;
	while( x1 < 0 ) {
		++x1; ++offset;
		z = zl.get_next_z();
	}
	if( x2 > _target->w ) {
		x2 = _target->w;
	}
	for( ; x1 < x2; ++ x1, ++ offset ) {
		if( _z_buffer[offset] == 0 || z <= _z_buffer[offset] ) {
			((int *)_target->pixels)[offset] = colour;
			_z_buffer[offset] = z;
		}
		z = zl.get_next_z();
	}
}

void renderer::draw_triangle_point_left( xy_point<int> const & p1, xy_point<int> const & p2, xy_point<int> const & p3, int colour, z_depth z1, z_depth z2, z_depth z3 )
{
	int y = p1._y;
	int xl = p1._x;
	int xr = p1._x;
	z_depth zl = z1;
	z_depth zr = z1;
  line_coords r(p1, p3);
	z_line rz(z1, z3, p3._y - p1._y);
  line_coords l1(p1, p2);
	z_line l1z(z1, z2, p2._y - p1._y);
  line_coords l2(p2, p3);
	z_line l2z(z2, z3, p3._y - p2._y);
	for( ; y < p2._y ; ++ y ) {
		draw_h_line( y, xl, xr, colour, zl, zr );
		xl = l1.get_next_x();
		zl = l1z.get_next_z();
		xr = r.get_next_x();
		zr = rz.get_next_z();
	}
	//draw_h_line( y++, xl, xr, 0x0000ff00, zl, zr );
	zl = z2;
	xl = p2._x;
	for( ; y <= p3._y ; ++ y ) {
		draw_h_line( y, xl, xr, colour, zl, zr );
		xl = l2.get_next_x();
		zl = l2z.get_next_z();
		xr = r.get_next_x();
		zr = rz.get_next_z();
	}
}

void renderer::draw_triangle_point_right( xy_point<int> const & p1, xy_point<int> const & p2, xy_point<int> const & p3, int colour, z_depth z1, z_depth z2, z_depth z3 )
{
	int y = p1._y;
	int xl = p1._x;
	int xr = p1._x;
	z_depth zl = z1;
	z_depth zr = z1;
  line_coords l(p1, p3);
	z_line lz(z1, z3, p3._y - p1._y);
  line_coords r1(p1, p2);
	z_line r1z(z1, z2, p2._y - p1._y);
  line_coords r2(p2, p3);
	z_line r2z(z2, z3, p3._y - p2._y);
	for( ; y < p2._y ; ++ y ) {
		draw_h_line( y, xl, xr, colour, zl, zr );
		xl = l.get_next_x();
		zl = lz.get_next_z();
		xr = r1.get_next_x();
		zr = r1z.get_next_z();
	}
	//draw_h_line( y++, xl, xr, 0x000000ff, zl, zr );
	zr = z2;
	xr = p2._x;
	for( ; y <= p3._y ; ++ y ) {
		draw_h_line( y, xl, xr, colour, zl, zr );
		xl = l.get_next_x();
		zl = lz.get_next_z();
		xr = r2.get_next_x();
		zr = r2z.get_next_z();
	}
}

void renderer::draw_triangle_point_left_clipped( xy_point<int> const & p1, xy_point<int> const & p2, xy_point<int> const & p3, int colour, z_depth z1, z_depth z2, z_depth z3 )
{
	int y = p1._y;
	int xl = p1._x;
	int xr = p1._x;
	z_depth zl = z1;
	z_depth zr = z1;
  line_coords r(p1, p3);
	z_line rz(z1, z3, p3._y - p1._y);
  line_coords l1(p1, p2);
	z_line l1z(z1, z2, p2._y - p1._y);
  line_coords l2(p2, p3);
	z_line l2z(z2, z3, p3._y - p2._y);
	for( ; y < p2._y ; ++ y ) {
		draw_h_line_clipped( y, xl, xr, colour, zl, zr );
		xl = l1.get_next_x();
		zl = l1z.get_next_z();
		xr = r.get_next_x();
		zr = rz.get_next_z();
	}
	zl = z2;
	xl = p2._x;
	for( ; y <= p3._y ; ++ y ) {
		draw_h_line_clipped( y, xl, xr, colour, zl, zr );
		xl = l2.get_next_x();
		zl = l2z.get_next_z();
		xr = r.get_next_x();
		zr = rz.get_next_z();
	}
}

void renderer::draw_triangle_point_right_clipped( xy_point<int> const & p1, xy_point<int> const & p2, xy_point<int> const & p3, int colour, z_depth z1, z_depth z2, z_depth z3 )
{
	int y = p1._y;
	int xl = p1._x;
	int xr = p1._x;
	z_depth zl = z1;
	z_depth zr = z1;
  line_coords l(p1, p3);
	z_line lz(z1, z3, p3._y - p1._y);
  line_coords r1(p1, p2);
	z_line r1z(z1, z2, p2._y - p1._y);
  line_coords r2(p2, p3);
	z_line r2z(z2, z3, p3._y - p2._y);
	for( ; y < p2._y ; ++ y ) {
		draw_h_line_clipped( y, xl, xr, colour, zl, zr );
		xl = l.get_next_x();
		zl = lz.get_next_z();
		xr = r1.get_next_x();
		zr = r1z.get_next_z();
	}
	zr = z2;
	xr = p2._x;
	for( ; y <= p3._y ; ++ y ) {
		draw_h_line_clipped( y, xl, xr, colour, zl, zr );
		xl = l.get_next_x();
		zl = lz.get_next_z();
		xr = r2.get_next_x();
		zr = r2z.get_next_z();
	}
}

bool is_left_of_line( xy_point<int> const & start, xy_point<int> const & end, xy_point<int> const & p )
{
	int dx = end._x - start._x;
	int dy = end._y - start._y;

  if( dx != 0 ) {
    float m = (float) dy / (float) dx;
		float b = start._x - start._y/m;
		return p._x < (p._y/m + b);
	} else {
		return p._x < end._x;
	}
}

void renderer::draw_triangle( xy_point<int> const & p1
														, xy_point<int> const & p2
														, xy_point<int> const & p3
														, int colour
														, z_depth z1
														, z_depth z2
														, z_depth z3
														, bool clipped )
{
	xy_point<int> top, middle, bottom;
	z_depth top_z, middle_z, bottom_z;
	switch( minimum_of_3(p1._y, p2._y, p3._y) ) {
	case first:
		top = p1;
		top_z = z1;
		if( p2._y < p3._y ) {
			middle = p2;
			middle_z = z2;
			bottom = p3;
			bottom_z = z3;
		} else {
			middle = p3;
			middle_z = z3;
			bottom = p2;
			bottom_z = z2;
		}
		break;
	case second:
		top = p2;
		top_z = z2;
		if( p1._y < p3._y ) {
			middle = p1;
			middle_z = z1;
			bottom = p3;
			bottom_z = z3;
		} else {
			middle = p3;
			middle_z = z3;
			bottom = p1;
			bottom_z = z1;
		}
		break;
	case third:
		top = p3;
		top_z = z3;
		if( p1._y < p2._y ) {
			middle = p1;
			middle_z = z1;
			bottom = p2;
			bottom_z = z2;
		} else {
			middle = p2;
			middle_z = z2;
			bottom = p1;
			bottom_z = z1;
		}
		break;
	}
	if( clipped ) {
		if( is_left_of_line( top, bottom, middle ) ) {
			draw_triangle_point_left_clipped( top, middle, bottom, colour, top_z, middle_z, bottom_z );
		}else {
			draw_triangle_point_right_clipped( top, middle, bottom, colour, top_z, middle_z, bottom_z );
		}
	} else {
		if( is_left_of_line( top, bottom, middle ) ) {
			draw_triangle_point_left( top, middle, bottom, colour, top_z, middle_z, bottom_z );
		}else {
			draw_triangle_point_right( top, middle, bottom, colour, top_z, middle_z, bottom_z );
		}
	}
}

int scale_colour( int colour, double scale ) {
  rgb result;
  result.value = colour;
  result.colour._r = (unsigned char)(result.colour._r * scale);
  result.colour._g = (unsigned char)(result.colour._g * scale);
  result.colour._b = (unsigned char)(result.colour._b * scale);
  return result.value;
}

void draw_v_line( int colour, int x, int top, int height, SDL_Surface * target ) {
	int line_size = target->pitch / target->format->BytesPerPixel;
	int offset = line_size * top + x;
	for( int i=height; i > 0; i-- ) {
		((int*)target->pixels)[offset] = colour;
		offset += line_size;
	}
	SDL_UpdateRect( target, x, top, 1, height );
}

//                                            ->            ->
// calculates the cross product of the vector ab and vector ac
xyz_point< double > calculate_normal( xyz_point< double > const & a, xyz_point< double > const & b, xyz_point< double > const & c )
{
	xyz_point< double > result;
	xyz_point< double > u(b);
	u -= a;
	xyz_point< double > v(c);
	v -= a;
	result = cross_product< double >( u, v );
  result *= 1/length(result);
	return result;
}

void renderer::render_solid( triangle_surface const & source )
{	
  clear_z_buffer();
	size_t number_of_triangles = source._triangles.size();

  double lightLength = length(_light);
	if( lightLength != 1 ){
		_light *= (1 / lightLength);
	}
	double depthScaler = tan( degrees2radians( _field_of_view / 2 ) );
	int triangle_colour;
	xyz_point<double> camera_vector( -_camera_angle * xyz_point<double>( 0, 0, 1 ) );
	xy_point<> centre( _target->w/2, _target->h/2 );

		// set up progress bar
	int progress, progress1, progress_triangles, progress_height, progress_top;
	progress = progress1 = progress_triangles = 0;
	progress_triangles = (int)number_of_triangles / _screen->w;
	progress = progress_triangles;
	progress_height = _screen->h / 16;
	progress_top = _screen->h / 2 - progress_height;

		// enter rendering loop
	for( uint i=0; i<number_of_triangles; ++i ) {
		if( --progress == 0 ) {
			progress = progress_triangles;
			draw_v_line( colour_orange, progress1, progress_top, progress_height, _screen );
			progress1++;
		}


		// put origin at camera
		xyz_point<double> bar1, bar2, bar3;
		bar1 = _source_data->_transformed_vertices[source._triangles[i]._p1];
		bar1 -= _camera_position;
		bar1 = _camera_angle * bar1;
		bar2 = _source_data->_transformed_vertices[source._triangles[i]._p2];
		bar2 -= _camera_position;
		bar2 = _camera_angle * bar2;
		bar3 = _source_data->_transformed_vertices[source._triangles[i]._p3];
		bar3 -= _camera_position;
		bar3 = _camera_angle * bar3;

		// remove all triangles behind the camera
		if( !(bar1._z < 0 && bar2._z < 0 && bar3._z < 0) ) {
			continue;
		}

		xy_point<double> fred1( bar1 );
		xy_point<double> fred2( bar2 );
		xy_point<double> fred3( bar3 );

		// depth scale
		fred1._x *= (double)_target->w;
		fred1._y *= (double)_target->w;
		fred1 /= -bar1._z * depthScaler;

		fred2._x *= (double)_target->w;
		fred2._y *= (double)_target->w;
		fred2 /= -bar2._z * depthScaler;

		fred3._x *= (double)_target->w;
		fred3._y *= (double)_target->w;
		fred3 /= -bar3._z * depthScaler;

		// put origin at top left of target
		xy_point<int> foo1( fred1 );
		foo1 += centre;
		xy_point<int> foo2( fred2 );
		foo2 += centre;
		xy_point<int> foo3( fred3 );
		foo3 += centre;

    // remove all triangles totally outside the target window
		if( foo1._x < 0 && foo2._x < 0 && foo3._x < 0 ||
				foo1._y < 0 && foo2._y < 0 && foo3._y < 0 ||
				foo1._x >= _target->w && foo2._x >= _target->w && foo3._x >= _target->w ||
				foo1._y >= _target->h && foo2._y >= _target->h && foo3._y >= _target->h ) {
			continue;
		}

		triangle_colour = colour_average( _vertex_colours[source._triangles[i]._p1], _vertex_colours[source._triangles[i]._p2], _vertex_colours[source._triangles[i]._p3] );

		xyz_point<double> tn = calculate_normal( _source_data->_transformed_vertices[source._triangles[i]._p1], _source_data->_transformed_vertices[source._triangles[i]._p2], _source_data->_transformed_vertices[source._triangles[i]._p3]);
    double sun_dot_prod = dot_product( _light, tn );
    double scale = sun_dot_prod;
    if(scale < 0) scale = 0;
    scale = scale * 0.9375 + 0.0625;
    triangle_colour = scale_colour( triangle_colour, scale );        
    //if( sun_dot_prod < 0 ) {
    //  triangle_colour = 0;//colour_average( triangle_colour, 0 );
    //}

		// remove all triangles facing away from the camera
		// i.e. don't render any portion of the surface from underneath
	  //if( !_show_far_side &&  dot_product( camera_vector, source._triangle_normals[i] ) > 0 ) {
		if( !_show_far_side &&  dot_product( bar1, _camera_angle * tn ) < 0 ) {
			continue;
			//triangle_colour = colour_green;
		}

		// calculate z values
		z_depth z1 = (double)bar1._x*(double)bar1._x + (double)bar1._y*(double)bar1._y + (double)bar1._z*(double)bar1._z;
		z_depth z2 = (double)bar2._x*(double)bar2._x + (double)bar2._y*(double)bar2._y + (double)bar2._z*(double)bar2._z;
		z_depth z3 = (double)bar3._x*(double)bar3._x + (double)bar3._y*(double)bar3._y + (double)bar3._z*(double)bar3._z;
		draw_triangle( foo1, foo2, foo3, triangle_colour, z1, z2, z3, window_clipped( foo1 ) || window_clipped( foo2 ) || window_clipped( foo3 ) );
	} // end triangle rendering loop
}

void renderer::render_solid_aa( triangle_surface const & source, antialaising_level level ) {
	SDL_Surface * scratch, * foo;
	int length, new_line;

	// create tempory surface
	switch(level) {
		case aa_none:
			scratch = SDL_CreateRGBSurface( SDL_SWSURFACE, _screen->w, _screen->h, 32, _screen->format->Rmask, _screen->format->Gmask, _screen->format->Bmask, _screen->format->Amask );
			break;
		case aa_4:
			scratch = SDL_CreateRGBSurface( SDL_SWSURFACE, _screen->w * 2, _screen->h * 2, 32, _screen->format->Rmask, _screen->format->Gmask, _screen->format->Bmask, _screen->format->Amask );
			break;
		case aa_16:
			scratch = SDL_CreateRGBSurface( SDL_SWSURFACE, _screen->w * 4, _screen->h * 4, 32, _screen->format->Rmask, _screen->format->Gmask, _screen->format->Bmask, _screen->format->Amask );
			break;
		default:
			error_warning("Attempted to render at invalid antialaising level");
			return;
	}
	memset( scratch->pixels, 0, scratch->h * scratch->pitch );

	// render
	foo = _screen;
	switch_target( scratch );
	_screen = foo;
	render_solid( source );
	switch_target( _screen );

	// copy scratch to screen
	if ( SDL_MUSTLOCK( _screen ) && SDL_LockSurface( _screen ) < 0 ) error_critical( "unable to lock display" );
	length = _screen->w * _screen->h;
	new_line = _screen->pitch / _screen->format->BytesPerPixel - _screen->w - 1;
	switch(level) {
		case aa_none:
			for( int offset=0, col=0, row=0; offset<length; offset++ ) {
				((unsigned int*)_screen->pixels)[offset] = ((int*)scratch->pixels)[offset];
				if( ++col > _screen->w ) {
					col = 0;
					++row;
					offset += new_line;
				}
			}
			break;
		case aa_4:
			for( int offset=0, col=0, row=0; offset<length; offset++ ) {
				((unsigned int*)_screen->pixels)[offset] = colour_average (
					((int*)scratch->pixels)[col*2   + scratch->pitch / scratch->format->BytesPerPixel * (row*2)],
					((int*)scratch->pixels)[col*2+1 + scratch->pitch / scratch->format->BytesPerPixel * (row*2)],
					((int*)scratch->pixels)[col*2   + scratch->pitch / scratch->format->BytesPerPixel * (row*2+1)],
					((int*)scratch->pixels)[col*2+1 + scratch->pitch / scratch->format->BytesPerPixel * (row*2+1)]);
				if( ++col > _screen->w ) {
					col = 0;
					++row;
					offset += new_line;
				}
			}
			break;
		case aa_16:
			for( int offset=0, col=0, row=0; offset<length; offset++ ) {
					((unsigned int*)_screen->pixels)[offset] = colour_average (
						((int*)scratch->pixels)[col*4   + scratch->pitch / scratch->format->BytesPerPixel * (row*4)],
						((int*)scratch->pixels)[col*4+1 + scratch->pitch / scratch->format->BytesPerPixel * (row*4)],
						((int*)scratch->pixels)[col*4+2 + scratch->pitch / scratch->format->BytesPerPixel * (row*4)],
						((int*)scratch->pixels)[col*4+3 + scratch->pitch / scratch->format->BytesPerPixel * (row*4)],
						((int*)scratch->pixels)[col*4   + scratch->pitch / scratch->format->BytesPerPixel * (row*4+1)],
						((int*)scratch->pixels)[col*4+1 + scratch->pitch / scratch->format->BytesPerPixel * (row*4+1)],
						((int*)scratch->pixels)[col*4+2 + scratch->pitch / scratch->format->BytesPerPixel * (row*4+1)],
						((int*)scratch->pixels)[col*4+3 + scratch->pitch / scratch->format->BytesPerPixel * (row*4+1)],
						((int*)scratch->pixels)[col*4   + scratch->pitch / scratch->format->BytesPerPixel * (row*4+2)],
						((int*)scratch->pixels)[col*4+1 + scratch->pitch / scratch->format->BytesPerPixel * (row*4+2)],
						((int*)scratch->pixels)[col*4+2 + scratch->pitch / scratch->format->BytesPerPixel * (row*4+2)],
						((int*)scratch->pixels)[col*4+3 + scratch->pitch / scratch->format->BytesPerPixel * (row*4+2)],
						((int*)scratch->pixels)[col*4   + scratch->pitch / scratch->format->BytesPerPixel * (row*4+3)],
						((int*)scratch->pixels)[col*4+1 + scratch->pitch / scratch->format->BytesPerPixel * (row*4+3)],
						((int*)scratch->pixels)[col*4+2 + scratch->pitch / scratch->format->BytesPerPixel * (row*4+3)],
						((int*)scratch->pixels)[col*4+3 + scratch->pitch / scratch->format->BytesPerPixel * (row*4+3)]);
					if( ++col > _screen->w ) {
					col = 0;
					++row;
					offset += new_line;
				}
			}
			break;
		default:
			error_warning("Attempted to render at invalid antialaising level");
			return;
	}
	if ( SDL_MUSTLOCK( _screen ) ) {
		SDL_UnlockSurface( _screen );
	}

	// destroy tempory surface
	SDL_FreeSurface( scratch );
	scratch = NULL;
	// Tell SDL to update the whole screen
	SDL_UpdateRect( _screen, 0, 0, _screen->w, _screen->h );
}

xyz_point<double> const & renderer::get_camera() {
	return _camera_position;
}

void renderer::set_light() {
  _light = -_camera_angle * xyz_point<double>(0,0,-1);
}

void renderer::set_light( xyz_point<double> const & light ) {
	_light = light;
}

void renderer::set_camera( xyz_point<double> const & camera ) {
	xyz_point<double> camera_change = camera - _camera_position;
	_camera_position += -_camera_angle * camera_change;
}

void renderer::set_true_camera( xyz_point<double> const & camera ) {
	_camera_position = camera;
}

SDL_Surface * renderer::get_target()
{
	return _target;
}

// helper function implemetations

// This function is based on conversion to rgb from hsv with value/brightness
// set to 255 and saturation set to 255. Note: equations this is based on
// were lifted from http://en.wikipedia.org/wiki/HSV_color_space
int gen_rainbow_colour( int hue_angle ) // angle in degrees
{
#ifdef _DEBUG
	if( hue_angle < 0 || hue_angle >= 360 )
		error_warning( "gen_colour has been passed an out of range angle" );
#endif
	rgb result;
	int v = 255;//, s = 255;
	int hi = (hue_angle / 60) % 6;
	double f;
	ubyte q, t;
	f = hue_angle / 60.0 - hi;
	q = (ubyte)(255 - f * 255);
	t = (ubyte)(f * 255);
	switch(hi) {
		case 0:
			result.colour._r = v;
			result.colour._g = t;
			result.colour._b = 0;
			break;
		case 1:
			result.colour._r = q;
			result.colour._g = v;
			result.colour._b = 0;
			break;
		case 2:
			result.colour._r = 0;
			result.colour._g = v;
			result.colour._b = t;
			break;
		case 3:
			result.colour._r = 0;
			result.colour._g = q;
			result.colour._b = v;
			break;
		case 4:
			result.colour._r = t;
			result.colour._g = 0;
			result.colour._b = v;
			break;
		case 5:
			result.colour._r = v;
			result.colour._g = 0;
			result.colour._b = q;
			break;
	}
	return result.value;	
}

int renderer::get_step_size()
{
	return _step_size;
}

double renderer::get_rotation_step_size()
{
	return _rotation_step_size;
}

int colour_average( int c1, int c2 )
{
	rgb foo, bar;
	bar.value = c1;
	foo.colour._r = bar.colour._r / 2;
	foo.colour._g = bar.colour._g / 2;
	foo.colour._b = bar.colour._b / 2;
	bar.value = c2;
	foo.colour._r += bar.colour._r / 2;
	foo.colour._g += bar.colour._g / 2;
	foo.colour._b += bar.colour._b / 2;
	return foo.value;
}

int colour_average( int c1, int c2, int c3 )
{
	rgb foo, bar;
	bar.value = c1;
	foo.colour._r = bar.colour._r / 3;
	foo.colour._g = bar.colour._g / 3;
	foo.colour._b = bar.colour._b / 3;
	bar.value = c2;
	foo.colour._r += bar.colour._r / 3;
	foo.colour._g += bar.colour._g / 3;
	foo.colour._b += bar.colour._b / 3;
	bar.value = c3;
	foo.colour._r += bar.colour._r / 3;
	foo.colour._g += bar.colour._g / 3;
	foo.colour._b += bar.colour._b / 3;
	return foo.value;
}

void renderer::rotate_camera( linear_math::axis axis, double angle )
{
	_camera_angle = linear_math::generate_rotation_matrix( axis, angle ) * _camera_angle;
}

void renderer::rotate_camera( linear_math::axis axis, linear_math::rotational_direction direction )
{
	_camera_angle = linear_math::generate_rotation_matrix( axis, direction == linear_math::clockwise ? _rotation_step_size : -_rotation_step_size ) * _camera_angle;
}

//void renderer::rotate_camera_around_origin( linear_math::axis axis, linear_math::rotational_direction direction ) 
//{
//	_camera_position = linear_math::generate_rotation_matrix( axis, direction == linear_math::clockwise ? _rotation_step_size/8 : -_rotation_step_size/8 ) * _camera_position;
//}

void renderer::rotate_camera_around_origin( linear_math::axis axis, linear_math::rotational_direction direction, int steps ) 
{
	matrix_3x3<double> rotation1 = linear_math::generate_rotation_matrix( axis, (direction == linear_math::clockwise ? _rotation_step_size : -_rotation_step_size) * steps/8 );
	matrix_3x3<double> rotation2 = linear_math::generate_rotation_matrix( axis, (direction == linear_math::clockwise ? -_rotation_step_size : _rotation_step_size) * steps/8 );
	_camera_position = rotation1 * _camera_position;
	//if( axis == y_axis )
		_camera_angle = _camera_angle * rotation2;
}

void renderer::rotate_camera( const double pitch, const double yaw, const double roll )
{
	_camera_angle = linear_math::generate_rotation_matrix( pitch, yaw, roll ) * _camera_angle;
}

void renderer::clear_z_buffer()
{
	memset( _z_buffer, 0, _target->pitch * _target->h * sizeof( z_depth ) / _target->format->BytesPerPixel );
}

linear_math::xyz_point<double> renderer::get_camera_position()
{
	return _camera_position;
}

linear_math::matrix_3x3<double> renderer::get_camera_angle()
{
	return _camera_angle;
}

digital_elevation_model * renderer::get_source_data()
{
	return _source_data;
}

std::vector<int> renderer::get_vertex_colours()
{
	return _vertex_colours;
}

void renderer::setFieldOfView( double fov )
{
	_field_of_view = fov;
}

double renderer::getFieldOfView( )
{
	return _field_of_view;
}

