#include "stdafx.h"
#include "dem.h"
#include "renderer.h"

#ifdef WITH_GL
void glwin( digital_elevation_model * m, renderer & r, triangle_surface * s);
#endif

void display_image( SDL_Surface * image );
void render( renderer & screen );

digital_elevation_model * dem1 = NULL;
renderer * Primary1;
#define primary (*Primary1)
triangle_surface * dem2;

string int_to_str( int input )
{
	std::stringstream foo;
	foo << input;
	return foo.str();
}

digital_elevation_model * open_dem() {
	string file = dialog_open_gif();
	if( file.empty() ) {
		return 0;
	}
	dialog_debug( "dialog successful" );
	if( endsWith( file, ".drs" ) ) {
		dialog_debug( "attempting DEM Renderer Script open" );
		runScript( file );
		//render( primary );
		return dem1;
	} else {
		digital_elevation_model *dem;
		try {
			dem = new digital_elevation_model( file );
		} catch( error e ) {
			switch( e.code ) {
				case INVALID_IMAGE_FILE:
					error_critical( IMG_GetError() );
					error_critical( "image file is invalid" );
				case IMAGE_IS_EMPTY:
					error_critical( "image file is empty" );
				default:
					error_critical( "unknown error" );
			}
		}
		dialog_debug( "gif opened" );
		return dem;
	}
}

enum key_mod {
	KEY_MOD_ALT,
	KEY_MOD_CTRL,
 	KEY_MOD_BOTH,
	KEY_MOD_NONE
};

key_mod get_key_mod( SDL_Event const & e )
{
	if( e.key.keysym.mod == KMOD_RALT || e.key.keysym.mod == KMOD_LALT ) {
		if( e.key.keysym.mod == KMOD_RCTRL || e.key.keysym.mod == KMOD_LCTRL ) {
			return KEY_MOD_BOTH;
		} else {
			return KEY_MOD_ALT;
		}	} else {
		if( e.key.keysym.mod == KMOD_RCTRL || e.key.keysym.mod == KMOD_LCTRL ) {
			return KEY_MOD_CTRL;
		} else {
			return KEY_MOD_NONE;
		}
	}
}

void loadDem( string fileName ) {
	if( fileName.empty() ) {
		return;
	}
	digital_elevation_model *dem;
	try {
		dem = new digital_elevation_model( fileName );
	} catch( error e ) {
		switch( e.code ) {
			case INVALID_IMAGE_FILE:
				error_critical( IMG_GetError() );
				error_critical( "image file is invalid" );
			case IMAGE_IS_EMPTY:
				error_critical( "image file is empty" );
			default:
				error_critical( "unknown error" );
		}
	}
	if( dem1 != NULL ) {
		delete dem1;
	}
	dem1 = dem;
	primary.switch_source( dem1 );
	delete dem2;
	dem2 = new triangle_surface( *dem1 );
	//render( primary );
	dialog_debug( "DEM opened" );

  // curve dem
	dem1->transform_surface();
	dem2->calculate_normals( *dem1 );
	primary.switch_source( dem1 );
	//render( primary );
}

void setPosition( double lon, double lat, double rad ) {
	linear_math::xyz_point<double> newCam;
	lat += 90;
	lon *= 0.017453292519943295769236907684886; // PI / 180
	lat *= 0.017453292519943295769236907684886; // PI / 180

	double sin_lambda, cos_lambda, sin_theta, cos_theta;
	sin_lambda = sin( lon );
	cos_lambda = cos( lon );
	sin_theta  = sin( lat  );
	cos_theta  = cos( lat  );
  newCam._x = sin_theta * cos_lambda;
  newCam._y = sin_theta * sin_lambda;
  newCam._z = cos_theta;
	newCam *= rad;
	primary.set_default_camera();
	primary.set_true_camera( newCam );
	primary.rotate_camera( linear_math::x_axis, data::PI / 2 );
	primary.rotate_camera( linear_math::y_axis, data::PI / 2 );
	primary.rotate_camera( linear_math::y_axis, lon );
	primary.rotate_camera( linear_math::x_axis, lat - data::PI / 2 );
	render( primary );
}

void setLight( double lon, double lat ) {
	linear_math::xyz_point<double> newLight;
	lat += 90;
	//lon += 180;
	lon *= 0.017453292519943295769236907684886; // PI / 180
	lat *= 0.017453292519943295769236907684886; // PI / 180

	double sin_lambda, cos_lambda, sin_theta, cos_theta;
	sin_lambda = sin( lon );
	cos_lambda = cos( lon );
	sin_theta  = sin( lat );
	cos_theta  = cos( lat );
  newLight._x = sin_theta * cos_lambda;
  newLight._y = sin_theta * sin_lambda;
  newLight._z = cos_theta;
	newLight = -newLight;
	primary.set_light( newLight );
}

void rotateCamera( linear_math::axis axis, double angle ) {
	primary.rotate_camera( axis, angle );
	render( primary );
}

void scaleRadius( double min, double step ) {
	dem1->set_radius( min, step );
	dem2->calculate_normals( *dem1 );
	primary.switch_source( dem1 );
}

void setFOV( double angle ) {
	primary.setFieldOfView( angle );
	render( primary );
}

void saveRender( string fileName, uint width, uint height ) {
	primary.render_solid_aa( *dem2, aa_4 );
	//error_warning( "rendered" );
}

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char *argv[])
{
	dem1 = new digital_elevation_model();
	if( dem1 == 0 ) {
		error_critical("Out of memory");
		return 0;
	}

	// movable render
	dialog_debug( "Initialising SDL" );
	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		error_critical( "Unable to initialise SDL" );
	}
	dialog_debug( "sdl initialised" );
	atexit(SDL_Quit);

	// open the screen for displaying the results
	SDL_Surface * screen;
	Uint32 screenFlags = SDL_RESIZABLE | SDL_ANYFORMAT;
	//screen = SDL_SetVideoMode(1280, 1024, 32, screenFlags );
	//screen = SDL_SetVideoMode(1024, 768, 32, screenFlags);
	screen = SDL_SetVideoMode(800, 600, 32, screenFlags);
	//screen = SDL_SetVideoMode(300, 300, 32, screenFlags);
	if(!screen) {
		error_critical( string( "SDL_SetVideoMode: " ) + SDL_GetError() );
	}

	/* set the window title */
	SDL_WM_SetCaption( "Digital Elevation Model Viewer", "Digital Elevation Model Viewer");

	/* print some info about the obtained screen */
	dialog_debug( string( "screen is " ) + int_to_str( screen->w ) + "x" + int_to_str( screen->h ) + " " + int_to_str( screen->format->BitsPerPixel ) + "bpp" );

	// create renderer
	Primary1 = new renderer( dem1, screen );
	dem2 = new triangle_surface( *dem1 );

	SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	// main loop
	while( true ) {
		SDL_Event event;
		digital_elevation_model * new_dem = 0;
		while (SDL_PollEvent(&event)) {
			linear_math::xyz_point<double> foo;
			switch (event.type) {
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
						case SDLK_LEFT:
						case SDLK_KP4:
							foo = primary.get_camera();
							foo._x-=primary.get_step_size();
							primary.set_camera( foo );
							render( primary );
							break;
						case SDLK_RIGHT:
						case SDLK_KP6:
							foo = primary.get_camera();
							foo._x+=primary.get_step_size();
							primary.set_camera( foo );
							render( primary );
							break;
						case SDLK_UP:
						case SDLK_KP8:
							foo = primary.get_camera();
							foo._y-=primary.get_step_size();
							primary.set_camera( foo );
							render( primary );
							break;
						case SDLK_DOWN:
						case SDLK_KP2:
							foo = primary.get_camera();
							foo._y+=primary.get_step_size();
							primary.set_camera( foo );
							render( primary );
							break;
						case SDLK_PAGEUP:
						case SDLK_KP9:
							foo = primary.get_camera();
							foo._z+=primary.get_step_size();
							primary.set_camera( foo );
							render( primary );
							break;
						case SDLK_PAGEDOWN:
						case SDLK_KP3:
							foo = primary.get_camera();
							foo._z-=primary.get_step_size();
							primary.set_camera( foo );
							render( primary );
							break;
						case SDLK_KP_PERIOD:
							primary.rotate_camera( linear_math::z_axis, linear_math::clockwise );
							render( primary );
							break;
						case SDLK_KP0:
							primary.rotate_camera( linear_math::z_axis, linear_math::anti_clockwise );
							render( primary );
							break;
						case SDLK_HOME:
						case SDLK_KP7:
							primary.rotate_camera( linear_math::x_axis, linear_math::anti_clockwise );
							render( primary );
							break;
						case SDLK_END:
						case SDLK_KP1:
							primary.rotate_camera( linear_math::x_axis, linear_math::clockwise );
							render( primary );
							break;
						case SDLK_DELETE:
						case SDLK_KP_MULTIPLY:
							primary.rotate_camera( linear_math::y_axis, linear_math::anti_clockwise );
							render( primary );
							break;
						case SDLK_INSERT:
						case SDLK_KP_DIVIDE:
							primary.rotate_camera( linear_math::y_axis, linear_math::clockwise );
							render( primary );
							break;
						default:
							break;
					}
					break;
				case SDL_KEYUP:
					switch( event.key.keysym.sym ) {
						case SDLK_F1:
							dialog_information("Digital Elevation Model Viewer\n\nWritten by Thomas Spriggs in 2007\ndigital.elevation.modeller@gmail.com");
							break;
						case SDLK_F2:
							primary.switch_far_side();
							render( primary );
							break;
						case SDLK_F3:
							switch( get_key_mod( event ) ) {
						  case KEY_MOD_NONE:
								new_dem = open_dem();
								if( new_dem && new_dem != dem1 ) {
									delete dem1;
									dem1 = new_dem;
									primary.switch_source( dem1 );
									delete dem2;
									dem2 = new triangle_surface( *dem1 );
									render( primary );
								}
							  break;
						  case KEY_MOD_ALT:
                dem1->transform_surface();
								dem2->calculate_normals( *dem1 );
								primary.switch_source( dem1 );
								render( primary );
								break;
							case KEY_MOD_CTRL:
								dem1->original_surface();
								dem2->calculate_normals( *dem1 );
								primary.switch_source( dem1 );
								render( primary );
								break;
							}
							break;

						case SDLK_F4:
							switch( get_key_mod( event ) ) {
						  case KEY_MOD_NONE: {
								int time = SDL_GetTicks();
								render( primary );
								dialog_information( int_to_str( SDL_GetTicks() - time ) + " milliseconds taken to render occlusion clipped points" );
  							}
							  break;
							}
							break;

						case SDLK_F5:
							{
								int time = SDL_GetTicks();
								SDL_Surface * screen_surface = primary.get_target();
								// Lock surface if needed
								if ( SDL_MUSTLOCK( screen_surface ) && SDL_LockSurface( screen_surface ) < 0 ) error_critical( "unable to lock display" );

								// clear display
								memset( screen_surface->pixels, 0, screen_surface->h * screen_surface->pitch );
								
								// render
								primary.render( *dem2 );

								// Unlock if needed
								if ( SDL_MUSTLOCK( screen_surface ) ) {
									SDL_UnlockSurface( screen_surface );
								}

								// Tell SDL to update the whole screen
								SDL_UpdateRect( screen_surface, 0, 0, screen_surface->w, screen_surface->h );
								dialog_information( int_to_str( SDL_GetTicks() - time ) + " milliseconds taken to render wireframe" );
							}
							break;
						case SDLK_F6:
							switch( get_key_mod( event ) ) {
						  case KEY_MOD_NONE: {
								int time = SDL_GetTicks();
								// render
								primary.render_solid_aa( *dem2, aa_none );
								dialog_information( int_to_str( SDL_GetTicks() - time ) + " milliseconds taken to render soild surface" );
								}
								break;
							case KEY_MOD_ALT: {
								int time = SDL_GetTicks();
								primary.render_solid_aa( *dem2, aa_4 );
								dialog_information( int_to_str( SDL_GetTicks() - time ) + " milliseconds taken to render 4x antialiased soild surface" );
								}
								break;
							case KEY_MOD_CTRL: {
								int time = SDL_GetTicks();
								primary.render_solid_aa( *dem2, aa_16 );
								dialog_information( int_to_str( SDL_GetTicks() - time ) + " milliseconds taken to render 16x antialiased soild surface" );
								}
								break;
							}
							break;
							
						case SDLK_F7: {
#ifdef WITH_GL							
              glwin(dem1, primary, dem2);
#endif
						} break;
						case SDLK_F9:
							render( primary );
							break;
						case SDLK_F11:
              switch( get_key_mod( event ) ) {
					    case KEY_MOD_NONE:
							  primary.switch_colours();
							  render( primary );
						    break;
					    case KEY_MOD_ALT:
                primary.load_vertex_colours();
							  render( primary );
							  break;
						  case KEY_MOD_CTRL:
							  break;
						  }
							break;
						case SDLK_F12:
							primary.set_default_camera();
							render( primary );
							break;
            case SDLK_l:
              primary.set_light();
              break;
						case SDLK_f:
							//dem2->remove_stretched_triangles( 500, *dem1 );
							//render( primary );
							break;
						case SDLK_p:
							linear_math::xyz_point<double> pos = primary.get_camera_position();
							double lat, lon, rad;
							rad = linear_math::length( pos );
							lat = acos( pos._z / rad ) - data::PI / 2;
							if( pos._x != 0 ) {
								lon = atan( pos._y / pos._x );
								if( pos._x < 0 ) {
									lon += data::PI;
								} else if( pos._y < 0 ) {
									lon += data::PI * 2;
								}
							}

							// convert from radians to degrees
							lat *= 180 / data::PI;
							lon *= 180 / data::PI;

							string d = "The camera position\n";

							d += "radius = ";
							char buffer[256];
							sprintf( buffer, "%.0f", rad );
							d += buffer;
							d += " meters\n";

							d += "latitude = ";
							sprintf( buffer, "%.2f", lat );
							d += buffer;
							d += " degrees\n";

							d += "longitude = ";
							sprintf( buffer, "%.2f", lon );
							d += buffer;
							d += " degrees";

							dialog_information( d );
							break;
					}
					break;
				case SDL_MOUSEMOTION:
					switch( event.motion.state ) {
						case SDL_BUTTON( SDL_BUTTON_LEFT ):
							foo = primary.get_camera();
							foo._x += primary.get_step_size() * event.motion.xrel;
							foo._y += primary.get_step_size() * event.motion.yrel;
							primary.set_camera( foo );
							render( primary );
							break;
						case SDL_BUTTON( SDL_BUTTON_MIDDLE ):
							if( event.motion.xrel >= 0 ) {
								while( event.motion.xrel-- > 0 ) {
									primary.rotate_camera( linear_math::z_axis, linear_math::clockwise );
								}
							}
							if( event.motion.xrel <= 0 ) {
								while( event.motion.xrel++ < 0 ) {
									primary.rotate_camera( linear_math::z_axis, linear_math::anti_clockwise );
								}
							}
							foo = primary.get_camera();
							foo._z -= primary.get_step_size() * event.motion.yrel;
							primary.set_camera( foo );
							render( primary );
							break;
						case SDL_BUTTON( SDL_BUTTON_RIGHT ):
							if( event.motion.xrel >= 0 ) {
								while( event.motion.xrel-- > 0 ) {
									primary.rotate_camera( linear_math::y_axis, linear_math::anti_clockwise );
								}
							}
							if( event.motion.xrel <= 0 ) {
								while( event.motion.xrel++ < 0 ) {
									primary.rotate_camera( linear_math::y_axis, linear_math::clockwise );
								}
							}
							primary.rotate_camera( 0, primary.get_rotation_step_size() * -event.motion.yrel, 0 );
							render( primary );
							break;
						case 5:
							primary.rotate_camera_around_origin( linear_math::z_axis, linear_math::anti_clockwise, event.motion.xrel );
							primary.rotate_camera_around_origin( linear_math::y_axis, linear_math::clockwise, event.motion.yrel );
							render( primary );
							break;
						default:
							break;
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					switch( event.button.button ) {
						case SDL_BUTTON_LEFT:
							break;
						case SDL_BUTTON_MIDDLE:
							break;
						case SDL_BUTTON_RIGHT:
							break;
						case SDL_BUTTON_WHEELUP:
							if( SDL_GetMouseState(0,0) == 5 ) {
								primary.rotate_camera_around_origin( linear_math::x_axis, linear_math::clockwise, 16 );
							}else{
								//foo = primary.get_camera();
								//foo._z+=primary.get_step_size() * 8;
								//primary.set_camera( foo );
								double fov = primary.getFieldOfView();
								fov *= 0.9375;
								primary.setFieldOfView( fov );
							}
							render( primary );
							break;
						case SDL_BUTTON_WHEELDOWN:
							if( SDL_GetMouseState(0,0) == 5 ) {
								primary.rotate_camera_around_origin( linear_math::x_axis, linear_math::anti_clockwise, 16 );
							}else{
								//foo = primary.get_camera();
								//foo._z-=primary.get_step_size() * 8;
								//primary.set_camera( foo );
								double fov = primary.getFieldOfView();
								fov /= 0.9375;
								primary.setFieldOfView( fov );
							}
							render( primary );
							break;
						default:
							break;
					}
					break;
				case SDL_VIDEORESIZE:
					SDL_FreeSurface( screen );
					screen = NULL;
					screen = SDL_SetVideoMode(event.resize.w, event.resize.h, 32, screenFlags);
					if(!screen) {
						error_critical( string( "Resizing error SDL_SetVideoMode: " ) + SDL_GetError() );
					}
					primary.switch_target(screen);
					render(primary);
					break;
				case SDL_QUIT:
					return(0);
			}
		}
		SDL_Delay(1);
	}

	delete dem1;
	delete dem2;
	delete Primary1;
	return 0;
}

void render( renderer & screen )
{
	SDL_Surface * screen_surface = screen.get_target();
	// Lock surface if needed
  if ( SDL_MUSTLOCK( screen_surface ) && SDL_LockSurface( screen_surface ) < 0 ) error_critical( "unable to lock display" );

	// clear display
	memset( screen_surface->pixels, 0, screen_surface->h * screen_surface->pitch );
	
	// render
	screen.render();

  // Unlock if needed
	if ( SDL_MUSTLOCK( screen_surface ) ) {
    SDL_UnlockSurface( screen_surface );
	}

  // Tell SDL to update the whole screen
  SDL_UpdateRect( screen_surface, 0, 0, screen_surface->w, screen_surface->h );
}

