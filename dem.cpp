#include "dem.h"
using namespace linear_math;
#include <math.h>
#include <freeimage.h>
//#include "data.h"
//#define phobos
//#define phobos2
//#define apollo11
#define wholeMoon1

digital_elevation_model::digital_elevation_model() {
  _map = SDL_CreateRGBSurface(0, 1, 1, 32, 0, 0, 0, 0);
	_no_pixels = 0;
	_transformed_vertices = NULL;
  // check validity of image
  if( _map == 0 ) {
	  throw error( OUT_OF_MEMORY );
  }
  if( _map->w <= 0 || _map->h <= 0 ) {
	  throw error( IMAGE_IS_EMPTY );
  }
	original_surface();
}

digital_elevation_model::digital_elevation_model( string file )
{
	_no_pixels = 0;
	_transformed_vertices = NULL;
	if( endsWith( file, ".tiff" ) || endsWith( file, ".tif" ) ) {
    FIBITMAP * image = FreeImage_Load(FIF_TIFF, file.c_str(), TIFF_DEFAULT);
    if( !image ) {
			error_critical(string("File \"") + file + "\" is not a valid tiff.\n");
		}else{ // image loaded into memory
			if( FreeImage_GetBPP( image ) != 16 ) {
				error_critical( "Only 16 bit greyscale tiff images are supported." );
			}
      uint dem_row, dem_col;
	    uint dem_width  = FreeImage_GetWidth( image );
	    uint dem_height = FreeImage_GetHeight( image );
	    uint dem_pitch  = FreeImage_GetPitch( image );
      _map = SDL_CreateRGBSurface( SDL_SWSURFACE, dem_width, dem_height, 8, 0xff, 0xff, 0xff, 0x00);
      if( !_map ) {
        error_critical("Unable to allocate surface for DEM");
      }
			
			uint pixel, vertices_i;

				// count pixels
			pixel = 0;
			_max = 0;
			_no_pixels = 0;
      dem_row = 0;
	    dem_col = 0;
		  for( ; dem_row < dem_height; dem_row++ ) {
		    uint16 * dem_pixel  = (uint16*)FreeImage_GetScanLine( image,  dem_row );
		    dem_col = 0;
		    do { // while dem_col
			    if( *dem_pixel != 0 ) {
            ((unsigned char *)_map->pixels)[pixel] = 0xff;
						_no_pixels++;
						if( *dem_pixel > _max) _max = *dem_pixel;
					} else {
            ((unsigned char *)_map->pixels)[pixel] = 0x0;
					}
          pixel++;
          dem_pixel++;
        } while( ++dem_col < dem_width );
      } // end for dem_row

				// load original_verticies
			vertices_i = pixel = 0;
			xyz_point<> vertex;
			_original_vertices.resize( _no_pixels );
      dem_row = 0;
	    dem_col = 0;
		  for( ; dem_row < dem_height; dem_row++ ) {
		    uint16 * dem_pixel  = (uint16*)FreeImage_GetScanLine( image,  dem_row );
		    dem_col = 0;
				do { // while dem_col
			    if( *dem_pixel != 0 ) {
			      vertex._x = dem_col;
			      vertex._y = dem_row;
			      vertex._z = *dem_pixel;
			      _original_vertices[vertices_i++] = vertex;
          }
          pixel++;
          dem_pixel++;
        } while( ++dem_col < dem_width );
      } // end for dem_row

      FreeImage_Unload( image );
    }
  } else {  // standard gif reader
    _map = IMG_Load( file.c_str() );
    // check validity of image
	  if( _map == 0 ) {
		  throw error( INVALID_IMAGE_FILE );
	  }
	  if( _map->w <= 0 || _map->h <= 0 ) {
		  throw error( IMAGE_IS_EMPTY );
	  }

	  // extract 3d points from image
	  int length = _map->pitch * _map->h;
	  int new_line = _map->pitch - _map->w - 1;
	  for( int offset=0, col=0, row=0; offset<length; offset += _map->format->BytesPerPixel ) {
		  unsigned char value = ((unsigned char*)_map->pixels)[offset];
		  if( value != 0 ) {
			  xyz_point<> vertex;
			  vertex._x = col;
			  vertex._y = row;
			  vertex._z = value;
			  _original_vertices.push_back( vertex );
				_no_pixels++;
		  }
		  if( ++col > _map->w ) {
			  col = 0;
			  ++row;
			  offset += new_line;
		  }
	  }
  }
	original_surface();
}

//std::vector< linear_math::xyz_point<double> > * digital_elevation_model::get_verticies()
linear_math::xyz_point<double> * digital_elevation_model::get_verticies()
{
	return _transformed_vertices;
}

digital_elevation_model::~digital_elevation_model()
{
	delete [] _transformed_vertices;
	SDL_FreeSurface( _map );
}

SDL_Surface * digital_elevation_model::get_surface()
{
	return _map;
}

void digital_elevation_model::original_surface()
{
	if( _transformed_vertices != NULL ) {
		delete [] _transformed_vertices;
	}
	_transformed_vertices = new linear_math::xyz_point<double>[_no_pixels];
	for( uint i = 0; i < _no_pixels; ++i ) {
		_transformed_vertices[i] = _original_vertices[i];
	}

/*
  //_transformed_vertices = _original_vertices;
  //_transformed_vertices.clear();

	//_transformed_vertices. = _original_vertices;
	

	uint no_vertices = _no_pixels;//(uint)_original_vertices.size();
	_transformed_vertices.resize( no_vertices );
	for( uint i = 0; i < no_vertices; ++i ) {
		_transformed_vertices[i] = _original_vertices[i];
	}
*/
	generate_size();
	//_size._x = _map->w;
	//_size._y = _map->h;
}

void digital_elevation_model::transform_surface()
{
	/*
  _transformed_vertices.clear();
	for( uint i = 0; i < _original_vertices.size(); ++i ) {
		xyz_point<double> vertex = _original_vertices[i];
		double radius, lambda, theta;
#ifdef phobos2
    radius = 12200 - 3200 + vertex._z;
    const double INTER_PIXEL_ANGLE = 0.004256900614620316041277294557;
		lambda = (double)vertex._x * 0.0042569006146203160412772945572893;
		theta  = data::PI / 2 - (double)vertex._y * 0.0042569006146203160412772945572893;
#else
#ifdef phobos
    radius = 12200 + 25 * (double)vertex._z - 3200;
    const double INTER_PIXEL_ANGLE = 0.0085138012292406320825545891145786;//0.0084679047266571246319747800088396;
		lambda = (double)vertex._x * INTER_PIXEL_ANGLE;
		theta  = (double)(184 - vertex._y) * INTER_PIXEL_ANGLE;
#else		
#ifdef apollo11
		radius = 1737400 + 75 * (double)vertex._z - 9600;
		const double INTER_PIXEL_ANGLE = 0.00057559415240517665821423720574833; // apollo
		lambda = (double)vertex._x *  INTER_PIXEL_ANGLE; // (1.0/30.32222) / 360 * data::PI * 2;
		theta  = (double)vertex._y * -INTER_PIXEL_ANGLE; // (1.0/30.32222) / 360 * data::PI * 2;
		theta += INTER_PIXEL_ANGLE * _map->h / 2;
#else
#ifdef wholeMoon1
		radius = 	1727800 + 75 * (double)vertex._z / 256;
		//const double INTER_PIXEL_ANGLE = data::PI / _map->h;
		//lambda = (double)vertex._x *  INTER_PIXEL_ANGLE / 2;
		//theta  = (double)vertex._y * -INTER_PIXEL_ANGLE;
		//theta += INTER_PIXEL_ANGLE * _map->h / 2;
		lambda = (double)vertex._x * 2 * data::PI / _map->w;
		theta  = data::PI / 2 + (double)vertex._y * data::PI / _map->h;
#else
		radius = 1737400 + 75 * (double)vertex._z - 9600;
		//const double INTER_PIXEL_ANGLE = 0.00058177641733144319230789692282954; // pole
		const double INTER_PIXEL_ANGLE = 0.00057559415240517665821423720574833;
		lambda = (double)vertex._x *  INTER_PIXEL_ANGLE; // (1.0/30.32222) / 360 * data::PI * 2;
		theta  = (double)vertex._y * -INTER_PIXEL_ANGLE; // (1.0/30.32222) / 360 * data::PI * 2;
		theta += INTER_PIXEL_ANGLE * _map->h / 2; //data::PI / 2 + 0.5180347371646589923928134851735;
#endif
#endif
#endif
#endif
		double sin_lambda, cos_lambda, sin_theta, cos_theta;
		sin_lambda = sin( lambda );
		cos_lambda = cos( lambda );
		sin_theta  = sin( theta  );
		cos_theta  = cos( theta  );
    vertex._x = radius * sin_lambda * cos_theta;
    vertex._y = radius * cos_lambda * cos_theta;
    vertex._z = radius * sin_theta;
		//matrix_3x3<> foo = linear_math::generate_rotation_matrix( linear_math::x_axis, data::PI / 2);
		_transformed_vertices.push_back( vertex );
	}
	generate_size();*/
}

//void digital_elevation_model::transform_surface( double inter_pixel_distance, double )
//{
//  _transformed_vertices.clear();
//	for( int i = 0; i < _original_vertices.size(); ++i )
//	{
//		xyz_point<> vertex = _original_vertices[i];
//		/*vertex._x *= 23;
//		vertex._y *= 23;
//		vertex._z *= 2;*/
//		double radius = 1737400 + 75 * (double)vertex._z - 9600;
//		double lambda = (double)vertex._x * 0.00057559415240517665821423720574833; // (1.0/30.32222) / 360 * data::PI * 2;
//		double theta  = (double)vertex._y * -0.00057559415240517665821423720574833; // (1.0/30.32222) / 360 * data::PI * 2;
//		double sin_lambda = sin( lambda );
//		double cos_lambda = cos( lambda );
//		double sin_theta  = sin( theta  );
//		double cos_theta  = cos( theta  );
//    vertex._x = radius * sin_lambda * cos_theta;
//    vertex._y = radius * cos_lambda * cos_theta;
//    vertex._z = radius * sin_theta;
//		_transformed_vertices.push_back( vertex );
//	}
//	generate_size();
//}

xy_point<> digital_elevation_model::get_size()
{
  return _size;
}

void digital_elevation_model::generate_size() {
	int min_x, min_y, max_x, max_y;
  min_x = min_y = max_x = max_y = 0;
	for( uint i = 0; i < _no_pixels; ++ i ) {
		min_x = (int)(min_x < _transformed_vertices[i]._x ? min_x : _transformed_vertices[i]._x);
		min_y = (int)(min_y < _transformed_vertices[i]._y ? min_y : _transformed_vertices[i]._y);
		max_x = (int)(max_x > _transformed_vertices[i]._x ? max_x : _transformed_vertices[i]._x);
		max_y = (int)(max_y > _transformed_vertices[i]._y ? max_y : _transformed_vertices[i]._y); }
	_size._x = max_x - min_x;
	_size._y = max_y - min_y; }

void digital_elevation_model::transform() {
	for( uint i = 0; i < _original_vertices.size(); ++i ) {
		xyz_point<double> & vertex = _transformed_vertices[i];
		vertex = _original_vertices[i];

		double radius, lon, phi;
		radius = 	_rad_min + _rad_step * (double)vertex._z;
		lon = (double)vertex._x * 2 * data::PI / _map->w;
		phi = (double)vertex._y * data::PI / _map->h;

		// for dems where lon=0 at width/2
		lon += data::PI;

		double sin_phi, cos_phi, sin_lon, cos_lon;
		sin_phi = sin( phi );
		cos_phi = cos( phi );
		sin_lon = sin( lon  );
		cos_lon = cos( lon  );
    vertex._x = radius * sin_phi * cos_lon;
    vertex._y = radius * sin_phi * sin_lon;
    vertex._z = radius * cos_phi;
	}
	generate_size();
}

void digital_elevation_model::set_radius( double min, double step ) {
	dialog_debug("scaling radius");
	_rad_min = min;
	_rad_step = step;
	transform();
}

uint digital_elevation_model::get_max() {
	return _max;
}
