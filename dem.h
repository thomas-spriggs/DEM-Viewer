#include "stdafx.h"
//#include "renderer.h"
#pragma once

class digital_elevation_model
{
	//friend class renderer;
public:
	digital_elevation_model( string file );
	digital_elevation_model();
	~digital_elevation_model();
	SDL_Surface * get_surface();
	void transform_surface();
	void original_surface();
	linear_math::xy_point<> get_size();
	//std::vector< linear_math::xyz_point<double> > * get_verticies();
	linear_math::xyz_point<double> * get_verticies();
	void set_radius( double min, double step );
	uint get_max(); // returns the maximum z value in the original vertices

	SDL_Surface * _map;
	//std::vector< linear_math::xyz_point<double> > _transformed_vertices;
	linear_math::xyz_point<double> * _transformed_vertices;
	std::vector< linear_math::xyz_point<int> > _original_vertices;
private:	
	linear_math::xy_point<> _size;
	void generate_size();
//private:
	double _rad_min;
	double _rad_step;
	void transform();
	uint _max;
	uint _no_pixels;
};


