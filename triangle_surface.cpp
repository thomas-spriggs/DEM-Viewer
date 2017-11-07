#include "triangle_surface.h"
using namespace linear_math;
using namespace data;
#define TRIANGULATION_METHOD 1

	// counts the number of non zero pixels in the first line of m
int find_values_in( SDL_Surface * m, int line_length ) {
  int values = 0, test = 0;
	for( int i=0, offset=0; i < line_length; ++i ) {
		for( int j=0; j < m->format->BytesPerPixel; ++j, ++offset ) {
			test |= ((unsigned char *)m->pixels)[offset];
		}
		if( test ) {
      test = 0;
			values++;
		}
	}
	return values;
}

	// // moves the offset to the next non zero byte in memory and
	// returns the number of bytes the offset has been advanced
int find_next_pixel( ubyte * a, int offset ) {
	int advances = 0;
	do {
		++advances;
		++offset;
	} while( a[offset] == 0 );
	return advances;
}

bool more_pixels_exist( SDL_Surface * image, int offset ) {
	while( ++offset < image->pitch * image->h ) {
		if( ((ubyte*)image->pixels)[offset] != 0 ) {
			return true;
		}
	}
	return false;
}

// constructor
triangle_surface::triangle_surface( digital_elevation_model & source ) :
  _triangle_normals( NULL )
{
	SDL_Surface * map = source.get_surface();
	std::vector< linear_math::xyz_point<int> > & vs = source._original_vertices;
	int * lineStarts = new int[map->h];
	int i, y;
	for( i=0; i<map->h; i++ ) {
		lineStarts[i] = -1;
	}
	y=-1;
	for( i=0; i<(int)vs.size(); ++i ) {
		if( vs[i]._y > y ) {
			y = vs[i]._y;
			lineStarts[y] = i;
		}
	}
	int end = (int)vs.size();
	for( y=0;y < map->h - 1; y++ ) {
		if( lineStarts[y] == -1 || lineStarts[y+1] == -1 ) {
			continue;
		}
		int l1, l2;
		l1 = lineStarts[y];
		l2 = lineStarts[y+1];
		do {
			if( l2+1 >= end || vs[l1+1]._y != y || vs[l2+1]._y != y+1 ) {
				break;
			}
			if( vs[l1]._x + 1 != vs[l1+1]._x ) {
				l1++;
				continue;
			}
			if( vs[l2]._x + 1 != vs[l2+1]._x ) {
				l2++;
				continue;
			}
			if( vs[l2]._x < vs[l1]._x ) {
				l2++;
				continue;
			}
			if( vs[l2]._x > vs[l1]._x ) {
				l1++;
				continue;
			}
			xyz_triangle_references<> t1( l1, l1 + 1, l2 );
			xyz_triangle_references<> t2( l1 + 1, l2 + 1, l2 );
			_triangles.push_back( t1 );
			_triangles.push_back( t2 );
			l1++; l2++;
		} while( true );
	}

	delete [] lineStarts;
	lineStarts = 0;

//	SDL_Surface * map = source.get_surface();
//#if TRIANGULATION_METHOD == 1
//	// extract 3d triangles from image
//	//int length = map->pitch * ( map->h - 1 );
//	int new_line = map->pitch - map->w;
//	int high_line_reference = 0;
//	int low_line_reference = find_values_in( map, map->w );
//  //try {
//	for( int offset=0, col=0, row=0; row+1<map->h; offset += map->format->BytesPerPixel ) {
//		unsigned char v1 = ((unsigned char*)map->pixels)[offset];
//		unsigned char v2 = ((unsigned char*)map->pixels)[offset + map->format->BytesPerPixel];
//		unsigned char v3 = ((unsigned char*)map->pixels)[offset + map->pitch];
//		unsigned char v4 = ((unsigned char*)map->pixels)[offset + map->pitch + map->format->BytesPerPixel];
//
//		if( v1 != 0 && v2 != 0 && v3 != 0 && v4 != 0 ) {
//			int vx1, vx2, vx3, vx4;
//      vx1 = high_line_reference;
//      vx2 = high_line_reference + 1;
//      vx3 = low_line_reference;
//      vx4 = low_line_reference  + 1;
//			xyz_triangle_references<> t1( vx1, vx2, vx3 );
//			xyz_triangle_references<> t2( vx2, vx4, vx3 );
//			_triangles.push_back( t1 );
//			_triangles.push_back( t2 );
//		} else {
//			int vx1, vx2, vx3;
//			if( v1 != 0 ) {
//				if( v2 != 0 ) {
//					if( v3 != 0) {
//						vx1 = high_line_reference;
//						vx2 = high_line_reference + 1;
//						vx3 = low_line_reference;
//						xyz_triangle_references<> t( vx1, vx2, vx3 );
//						_triangles.push_back( t );
//					} else if ( v4 != 0 ) {
//						vx1 = high_line_reference;
//						vx2 = high_line_reference + 1;
//						vx3 = low_line_reference;
//						xyz_triangle_references<> t( vx1, vx2, vx3 );
//						_triangles.push_back( t );
//					}
//				} else if ( v3 != 0 && v4 != 0 ) {
//						vx1 = high_line_reference;
//						vx2 = low_line_reference;
//						vx3 = low_line_reference  + 1;
//						xyz_triangle_references<> t( vx1, vx3, vx2 );
//						_triangles.push_back( t );
//				}
//			} else if( v2 != 0 && v3 != 0 && v4 != 0 ) {
//				vx1 = high_line_reference;
//				vx2 = low_line_reference;
//				vx3 = low_line_reference  + 1;
//				xyz_triangle_references<> t( vx1, vx3, vx2 );
//				_triangles.push_back( t );
//			}
//		}
//		  // maintain line references
//		if( v1 ) ++ high_line_reference;
//		if( v3 ) ++ low_line_reference;
//    ++col;
//		  // handle line ends
//		if( col >= map->w ) {
//			col = 0;
//			++row;
//			offset += new_line;
//		}
//	}
//#endif
//#if TRIANGULATION_METHOD == 2
//	// extract 3d triangles from image
//	ubyte * data = (ubyte*)(map->pixels);
//	int length = map->h - 1; // the number of lines in the dem minus the last line
//	int high_pixel_ref = 0;
//	int low_pixel_ref = find_values_in( map, map->w );
//	int high_offset = 0; // stores the offset of the start of the high line
//	int low_offset = map->pitch; // stores the offset of the start of the low line
//	
//	if( data[high_offset] == 0 ) {
//		high_offset += find_next_pixel( data, high_offset );
//	}
//	if( data[low_offset] == 0 ) {
//		low_offset += find_next_pixel( data, low_offset );
//	}
//	//while( low_offset + find_next_pixel( data, low_offset ) < map->pitch * map->h ) {
//	while( more_pixels_exist( map, low_offset ) ) {
//		int next_offset;
//		if( high_offset < low_offset - map->pitch ) {
//			next_offset = high_offset + find_next_pixel( data, high_offset );
//			xyz_triangle_references<> t( high_pixel_ref, high_pixel_ref + 1, low_pixel_ref );
//			_triangles.push_back( t );
//			high_offset = next_offset;
//			++high_pixel_ref;
//		} else {
//			next_offset = low_offset + find_next_pixel( data, low_offset );
//			xyz_triangle_references<> t( low_pixel_ref + 1, low_pixel_ref, high_pixel_ref );
//			_triangles.push_back( t );
//			low_offset = next_offset;
//			++low_pixel_ref;
//		}
//	}
//	//for( int row=0; row<length; ++row ) {
//	//	// int initial_low_offset = low_offset;
//	//	int high_col=0, low_col=0;
//	//		// find first non zero pixel on the high line
//	//	if( data[high_offset] == 0 ) {
//	//		high_col += find_next_pixel( data, high_offset );
//	//	}
//	//		// find first non zero pixel on the low line
//	//	if( data[low_offset] == 0 ) {
//	//		low_col += find_next_pixel( data, low_offset );
//	//	}
//	//		// deal with empty lines ************ fix line refrences!!!!!!!!! *****************
//	//	if( high_col >= map->w || low_col >= map->w ) {
//	//		high_offset = low_offset + (low_col >= map->w ? map->pitch : 0); // skip an extra line if the low line was empty
//	//		low_offset = high_offset + map->pitch;
//	//		continue;
//	//	}
//
//	//	while( high_col < map->w && low_col < map->w ) {
//	//		if( high_col < low_col ) {
//	//			xyz_triangle_references<> t1( vx1, vx2, vx3 );
//	//		} else {
//	//		}
//	//	}
//	//}
//#endif

	calculate_normals( source );
}

//                                            ->            ->
// calculates the cross product of the vector ab and vector ac
xyz_point< double > calculate_normal1( xyz_point< double > const & a, xyz_point< double > const & b, xyz_point< double > const & c )
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

	// this should be called whenever the digital elevation model
	// 3d points are changed.
void triangle_surface::calculate_normals( digital_elevation_model & source )
{
	//_triangle_normals.clear();
	//std::vector< xyz_point<double> > * verticies = source.get_verticies();
	//const size_t LENGTH = _triangles.size();
	//for( uint i=0; i<LENGTH; ++i ) {
	//	_triangle_normals.push_back( calculate_normal( (*verticies)[_triangles[i]._p1], (*verticies)[_triangles[i]._p2], (*verticies)[_triangles[i]._p3] ) );
	//}
	
	if( _triangle_normals != NULL ) {
		free( _triangle_normals );
		_triangle_normals = NULL;
	}
	
	//xyz_point<double> * verticies = source.get_verticies();
	//const size_t LENGTH = _triangles.size();
	//_triangle_normals = (linear_math::xyz_point<double>*)malloc( sizeof( xyz_point<double> ) * LENGTH );
	//for( uint i=0; i<LENGTH; ++i ) {
	//	_triangle_normals[i] = calculate_normal( (verticies)[_triangles[i]._p1], (verticies)[_triangles[i]._p2], (verticies)[_triangles[i]._p3] );
	//}
}

void triangle_surface::remove_stretched_triangles( double maximum_vertex_distance, digital_elevation_model & source ) {
	//vector::iterator i = _triangles.begin;
	double vd2 = maximum_vertex_distance * maximum_vertex_distance;
	std::deque< data::xyz_triangle_references<> >::iterator i = _triangles.begin();
	while( i != _triangles.end() ) {
		//bool remove_triangle = false;
		data::xyz_triangle_references<> triangle_i = *i;
		double d1 = linear_math::length2(source._transformed_vertices[triangle_i._p1] - source._transformed_vertices[triangle_i._p2]);
		double d2 = linear_math::length2(source._transformed_vertices[triangle_i._p1] - source._transformed_vertices[triangle_i._p3]);
		double d3 = linear_math::length2(source._transformed_vertices[triangle_i._p3] - source._transformed_vertices[triangle_i._p2]);
		if( d1 > vd2 || d2 > vd2 || d3 > vd2 ) {
		  //std::vector< data::xyz_triangle_references<> >::iterator i_temp = i+1;
			i = _triangles.erase( i );
			//i = i_temp;
			//i = _triangles..begin();
		} else {
			++i;
		}
	}
	dialog_information("Triangle filtering complete");
	calculate_normals( source );
}

// destructor
triangle_surface::~triangle_surface()
{
	if( _triangle_normals != NULL ) {
		free( _triangle_normals );
		_triangle_normals = NULL;
	}
}

