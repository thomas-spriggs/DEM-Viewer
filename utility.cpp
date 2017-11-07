#include "utility.h"

bool endsWith( const string & filename, const string & ext ) {
  //return filename.find_last_of( ext, l ) == l - ext.length();
  size_t el = ext.length();
  size_t filename_offset = filename.length() - el;
  if( filename_offset < 0 ) {
    return false;
  }
  for( size_t i=0; i<el; ++i ) {
    if( filename.at(i+filename_offset) != ext.at(i) ) {
      return false;
    }
  }
  return true;
}

	// returns true is the given file exists already
bool file_exists( const char * const file_name ) {
	FILE * image_file;
	if( !(image_file = fopen(file_name, "rb")) ) {
		return false;
	} else {
		fclose( image_file );
		return true;
	}
}

double degrees2radians( double degrees )
{
	return degrees * data::PI / 180;
}

int colour_average( int c1, int c2, int c3, int c4 )
{
	rgb foo, bar;
	bar.value = c1;
	foo.colour._r = bar.colour._r / 4;
	foo.colour._g = bar.colour._g / 4;
	foo.colour._b = bar.colour._b / 4;
	bar.value = c2;
	foo.colour._r += bar.colour._r / 4;
	foo.colour._g += bar.colour._g / 4;
	foo.colour._b += bar.colour._b / 4;
	bar.value = c3;
	foo.colour._r += bar.colour._r / 4;
	foo.colour._g += bar.colour._g / 4;
	foo.colour._b += bar.colour._b / 4;
	bar.value = c4;
	foo.colour._r += bar.colour._r / 4;
	foo.colour._g += bar.colour._g / 4;
	foo.colour._b += bar.colour._b / 4;
	return foo.value;
}

int colour_average( int c1, int c2, int c3, int c4, int c5, int c6, int c7, int c8, int c9, int c10, int c11, int c12, int c13, int c14, int c15, int c16 )
{
	rgb foo, bar;
	int r, g, b;
	r = g = b = 0;
	bar.value = c1;
	r += bar.colour._r;
	g += bar.colour._g;
	b += bar.colour._b;
	bar.value = c2;
	r += bar.colour._r;
	g += bar.colour._g;
	b += bar.colour._b;
	bar.value = c3;
	r += bar.colour._r;
	g += bar.colour._g;
	b += bar.colour._b;
	bar.value = c4;
	r += bar.colour._r;
	g += bar.colour._g;
	b += bar.colour._b;
	bar.value = c5;
	r += bar.colour._r;
	g += bar.colour._g;
	b += bar.colour._b;
	bar.value = c6;
	r += bar.colour._r;
	g += bar.colour._g;
	b += bar.colour._b;
	bar.value = c7;
	r += bar.colour._r;
	g += bar.colour._g;
	b += bar.colour._b;
	bar.value = c8;
	r += bar.colour._r;
	g += bar.colour._g;
	b += bar.colour._b;
	bar.value = c9;
	r += bar.colour._r;
	g += bar.colour._g;
	b += bar.colour._b;
	bar.value = c10;
	r += bar.colour._r;
	g += bar.colour._g;
	b += bar.colour._b;
	bar.value = c11;
	r += bar.colour._r;
	g += bar.colour._g;
	b += bar.colour._b;
	bar.value = c12;
	r += bar.colour._r;
	g += bar.colour._g;
	b += bar.colour._b;
	bar.value = c13;
	r += bar.colour._r;
	g += bar.colour._g;
	b += bar.colour._b;
	bar.value = c14;
	r += bar.colour._r;
	g += bar.colour._g;
	b += bar.colour._b;
	bar.value = c15;
	r += bar.colour._r;
	g += bar.colour._g;
	b += bar.colour._b;
	bar.value = c16;
	r += bar.colour._r;
	g += bar.colour._g;
	b += bar.colour._b;
	r /= 16;
	g /= 16;
	b /= 16;
	foo.colour._r = r;
	foo.colour._g = g;
	foo.colour._b = b;
	return foo.value;
}

