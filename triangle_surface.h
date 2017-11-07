#include "stdafx.h"
#include "dem.h"
#include <deque>
//#pragma once

// This class handles how the veticies are joined to form triangles
class triangle_surface
{
	friend class renderer;
public:
	triangle_surface( digital_elevation_model & source );
	~triangle_surface();

		// this should be called whenever the digital elevation model
		// 3d points are changed.
	void calculate_normals( digital_elevation_model & source );
	void remove_stretched_triangles( double maximum_vertex_distance, digital_elevation_model & source );
//private:
	//std::vector< data::xyz_triangle<> > _triangles;
	std::deque< data::xyz_triangle_references<> > _triangles;
	//std::vector< data::xyz_point< double > > _triangle_normals;
	data::xyz_point< double > * _triangle_normals;
	//void add_triangle( xyz_triangle_references<> tr, xyz_triangle<> tp );
};
