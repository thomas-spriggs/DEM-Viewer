#include "stdafx.h"
#include "dem.h"
#include "triangle_surface.h"
#pragma once

enum antialaising_level {
	aa_none,
	aa_4,
	aa_16
};

class renderer
{
	//friend digital_elevation_model;
public:
	renderer( digital_elevation_model * source_data, SDL_Surface * _target );
	~renderer();

	SDL_Surface * get_target();
	void render();
	//void render_no_z();
	//void render_draw_z();
	void render( triangle_surface const & source );
	void render_solid( triangle_surface const & source );
	void render_solid_aa( triangle_surface const & source, antialaising_level level );
	//SDL_Surface * render();
	void switch_colours();
	void switch_source( digital_elevation_model * source_data );
	void switch_target( SDL_Surface * target );

	void set_default_camera();
	linear_math::xyz_point<double> const & get_camera();
	void set_camera( linear_math::xyz_point<double> const & camera );
	void set_true_camera(linear_math::xyz_point<double> const & camera );
	void set_light(linear_math::xyz_point<double> const & light );
	void rotate_camera( linear_math::axis axis, double angle );
	void rotate_camera( linear_math::axis axis, linear_math::rotational_direction direction );
	//void rotate_camera_around_origin( linear_math::axis axis, linear_math::rotational_direction direction );
	void rotate_camera_around_origin( linear_math::axis axis, linear_math::rotational_direction direction, int steps );
	void rotate_camera( const double pitch, const double yaw, const double roll );
	int get_step_size();
	double get_rotation_step_size();
	linear_math::xyz_point<double> get_camera_position();
	linear_math::matrix_3x3<double> get_camera_angle();
	digital_elevation_model * get_source_data();
	std::vector<int> get_vertex_colours();
  void load_vertex_colours();
  void set_light();
	void setFieldOfView( double fov );
	double getFieldOfView();
	void switch_far_side();

private:
	int _colour_state;
	int gen_colour( int const value );
	digital_elevation_model * _source_data;
	std::vector<int> _vertex_colours;
	void generate_vertex_colours();
	SDL_Surface * _target;
	SDL_Surface * _screen;
	linear_math::xy_point<int> _window_size;
	//unsigned int * _z_buffer;
	z_depth * _z_buffer;

	int _step_size;
	linear_math::xyz_point<double> _camera_position;
	const double _rotation_step_size;
	linear_math::matrix_3x3<double> _camera_angle;
	double _field_of_view;
	bool _show_far_side;
  linear_math::xyz_point<double> _light;

	void draw_h_line( int y, int x1, int x2, int colour );
	void draw_h_line( int y, int x1, int x2, int colour, z_depth z1, z_depth z2 );
	void draw_h_line_clipped( int y, int x1, int x2, int colour, z_depth z1, z_depth z2 );
	void draw_triangle_point_left(  linear_math::xy_point<int> const & p1, linear_math::xy_point<int> const & p2, linear_math::xy_point<int> const & p3, int colour, z_depth z1, z_depth z2, z_depth z3 );
	void draw_triangle_point_right( linear_math::xy_point<int> const & p1, linear_math::xy_point<int> const & p2, linear_math::xy_point<int> const & p3, int colour, z_depth z1, z_depth z2, z_depth z3 );
	void draw_triangle_point_left_clipped(  linear_math::xy_point<int> const & p1, linear_math::xy_point<int> const & p2, linear_math::xy_point<int> const & p3, int colour, z_depth z1, z_depth z2, z_depth z3 );
	void draw_triangle_point_right_clipped( linear_math::xy_point<int> const & p1, linear_math::xy_point<int> const & p2, linear_math::xy_point<int> const & p3, int colour, z_depth z1, z_depth z2, z_depth z3 );
  void set_pixel( int x, int y, int colour );
  void set_pixel( int x, int y, int colour, z_depth z );
  void draw_line( linear_math::xy_point<int> start, linear_math::xy_point<int> end, int colour );
	void draw_triangle_frame( linear_math::xy_point<int> const & p1, linear_math::xy_point<int> const & p2, linear_math::xy_point<int> const & p3, int colour );
	void draw_triangle( linear_math::xy_point<int> const & p1, linear_math::xy_point<int> const & p2, linear_math::xy_point<int> const & p3, int colour, z_depth z1, z_depth z2, z_depth z3, bool clipped = false );
	bool window_clipped( linear_math::xy_point<int> const & point );
  void clear_z_buffer();
};
