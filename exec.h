#include "stdafx.h"
#include "main.h"
#pragma once

struct command_load_dem {
  char * fileName;
};

struct command_set_position {
  double lon, lat, rad;
};

struct command_scale_radius {
	double min, step;
};

struct command_rotate_camera {
	linear_math::axis direction;
	double angle;
};

struct command_fov {
	double angle;
};

struct command_save_render {
  char * fileName;
	uint width, height;
};

struct command_set_light {
	double lon, lat;
};

enum COMMAND_TYPE { 
    COMMAND_TYPE_NONE,
    COMMAND_TYPE_LOAD_DEM,
    COMMAND_TYPE_SCALE_RADIUS,
    COMMAND_TYPE_SET_POSITION,
		COMMAND_TYPE_SET_LIGHT,
		COMMAND_TYPE_ROTATE_CAMERA,
		COMMAND_TYPE_FOV,
		COMMAND_TYPE_SAVE_RENDER
};

struct command {
  COMMAND_TYPE type;
  union {
    command_load_dem loadDem;
    command_set_position setPosition;
		command_set_light setLight;
		command_scale_radius scaleRadius;
		command_rotate_camera rotateCamera;
		command_fov fov;
		command_save_render saveRender;
  } data;
};

void execCommand( command c );
void runScript( string fileName );

