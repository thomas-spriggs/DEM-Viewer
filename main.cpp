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

#ifdef WITH_GL
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////// BEGIN OPENGL CODE HERE //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
// Code based on http://nehe.gamedev.net/tutorial/creating_an_opengl_window_(win32)/13001/

#include <windows.h>										// Header File For Windows
#include <gl\gl.h>											// Header File For The OpenGL32 Library
#include <gl\glu.h>											// Header File For The GLu32 Library
#include <gl\glaux.h>										// Header File For The GLaux Library

HGLRC           hRC = NULL;								// Permanent Rendering Context
HDC             hDC = NULL;								// Private GDI Device Context
HWND            hWnd = NULL;							// Holds Our Window Handle
HINSTANCE       hInstance;							// Holds The Instance Of The Application

bool	keys[256];												// Array Used For The Keyboard Routine
bool	active = TRUE;											// Window Active Flag Set To TRUE By Default
bool	fullscreen = TRUE;									// Fullscreen Flag Set To Fullscreen Mode By Default

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);			// Declaration For WndProc

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)				// Resize And Initialize The GL Window
{
	if (height == 0) {								// Prevent A Divide By Zero By
		height = 1;							// Making Height Equal One
	}

	glViewport(0, 0, width, height);					              // Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						                // Select The Projection Matrix
	glLoadIdentity();							                          // Reset The Projection Matrix

																		  // Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);						                  // Select The Modelview Matrix
	glLoadIdentity();																				// Reset The Modelview Matrix
}

int InitGL(GLvoid)								                        // All Setup For OpenGL Goes Here
{
	glShadeModel(GL_SMOOTH);					                    	// Enables Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);					          // Black Background

	glClearDepth(1.0f);																			// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);																// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);																	// The Type Of Depth Test To Do

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);			// Really Nice Perspective Calculations
	return TRUE;								                            // Initialization Went OK
}

void set_gl_colour(int colour) {
	//unsigned char * c = (unsigned char *)&colour;
	rgb c;
	c.value = colour;
	glColor3f(c.colour._r / 255.0f, c.colour._g / 255.0f, c.colour._b / 255.0f);
}

int DrawGLScene(digital_elevation_model * m, renderer & r, triangle_surface * s)							// Here's Where We Do All The Drawing
{
	using namespace linear_math;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
	glLoadIdentity();						// Reset The View
	triangle_surface & source(*s);


	linear_math::xyz_point<double> _camera_position(r.get_camera_position());
	//linear_math::matrix_3x3<double> & _camera_angle(r.get_camera_angle());
	digital_elevation_model * _source_data(r.get_source_data());
	std::vector<int> _vertex_colours(r.get_vertex_colours());

	int number_of_triangles = source._triangles.size();
	int triangle_colour;
	xyz_point<double> camera_vector(-r.get_camera_angle() * xyz_point<double>(0, 0, 1));
	//xy_point<> centre( _target->w/2, _target->h/2 );
	//glTranslatef(-1.5f, 0.0f, -6.0f);
	glBegin(GL_TRIANGLES);						// Drawing Using Triangles
	glColor3f(1.0f, 1.0f, 1.0f);
	for (int i = 0; i<number_of_triangles; ++i) {
		//triangle_colour = colour_average( _vertex_colours[source._triangles[i]._p1], _vertex_colours[source._triangles[i]._p2], _vertex_colours[source._triangles[i]._p3] );

		// put origin at camera
		xyz_point<double> bar1 = _source_data->_transformed_vertices[source._triangles[i]._p1];
		bar1 -= _camera_position;
		bar1 = r.get_camera_angle() * bar1;
		xyz_point<double> bar2 = _source_data->_transformed_vertices[source._triangles[i]._p2];
		bar2 -= _camera_position;
		bar2 = r.get_camera_angle() * bar2;
		xyz_point<double> bar3 = _source_data->_transformed_vertices[source._triangles[i]._p3];
		bar3 -= _camera_position;
		bar3 = r.get_camera_angle() * bar3;
		//		bar1 *= 0.00390625;//0.03125;
		//		bar2 *= 0.00390625;//0.03125;
		//		bar3 *= 0.00390625;//0.03125;
		bar1 *= 0.000244140625;//0.00390625;//0.03125;
		bar2 *= 0.000244140625;//0.00390625;//0.03125;
		bar3 *= 0.000244140625;//0.00390625;//0.03125;
							   // remove all triangles behind the camera
							   //if( bar1._z < 0 || bar2._z < 0 || bar3._z < 0 ) {
		set_gl_colour(_vertex_colours[source._triangles[i]._p1]);
		glVertex3d(bar1._x, -bar1._y, bar1._z);
		set_gl_colour(_vertex_colours[source._triangles[i]._p2]);
		glVertex3d(bar2._x, -bar2._y, bar2._z);
		set_gl_colour(_vertex_colours[source._triangles[i]._p3]);
		glVertex3d(bar3._x, -bar3._y, bar3._z);
		//} // end of behind camera checked
	} // end triangle loop
	glEnd();							// Finished Drawing The Triangle


										//glTranslatef(-1.5f, 0.0f, -6.0f);					// Move Left 1.5 Units And Into The Screen 6.0
										//glBegin(GL_TRIANGLES);						// Drawing Using Triangles
										//	glColor3f(1.0f, 0.0f, 0.0f);			// Set The Color To Red
										//	glVertex3f( 0.0f, 1.0f, 0.0f);				// Top
										//	glColor3f(0.0f, 1.0f, 0.0f);			// Set The Color To Red
										//	glVertex3f(-1.0f, -1.0f, 0.0f);				// Bottom Left
										//	glColor3f(0.0f, 0.0f, 1.0f);			// Set The Color To Red
										//	glVertex3f( 1.0f, -1.0f, 0.0f);				// Bottom Right
										//glEnd();							// Finished Drawing The Triangle
										//glTranslatef(3.0f,0.0f,0.0f);					// Move Right 3 Units
										//glBegin(GL_QUADS);						// Draw A Quad
										//	glColor3f(0.5f,0.5f,1.0f);
										//	glVertex3f(-1.0f, 1.0f, 0.0f);				// Top Left
										//	glVertex3f( 1.0f, 1.0f, 0.0f);				// Top Right
										//	glVertex3f( 1.0f,-1.0f, 0.0f);				// Bottom Right
										//	glVertex3f(-1.0f,-1.0f, 0.0f);				// Bottom Left
										//glEnd();							// Done Drawing The Quad
	return TRUE;							// Keep Going
}



//int DrawGLScene(GLvoid)								                    // Here's Where We Do All The Drawing
//{
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);			// Clear The Screen And The Depth Buffer
//	glLoadIdentity();							                          // Reset The Current Modelview Matrix
//	return TRUE;								                            // Everything Went OK
//}

GLvoid KillGLWindow(GLvoid)							                  // Properly Kill The Window
{
	if (fullscreen) {			  			                          // Are We In Fullscreen Mode?
		ChangeDisplaySettings(NULL, 0);					              // If So Switch Back To The Desktop
		ShowCursor(TRUE);						                          // Show Mouse Pointer
	}

	if (hRC) {  						                                // Do We Have A Rendering Context?
		if (!wglMakeCurrent(NULL, NULL)) {				            // Are We Able To Release The DC And RC Contexts?
			MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		if (!wglDeleteContext(hRC)) {				                // Are We Able To Delete The RC?
			MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;							                                // Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd, hDC)) {				            // Are We Able To Release The DC
		MessageBox(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;							                              // Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd)) {				            // Are We Able To Destroy The Window?
		MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;							                            // Set hWnd To NULL
	}
	if (!UnregisterClass("OpenGL", hInstance)) {			      // Are We Able To Unregister Class
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;							                        // Set hInstance To NULL
	}
}

BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;						                      // Holds The Results After Searching For A Match
	WNDCLASS	wc;							                              // Windows Class Structure
	DWORD		dwExStyle;						                          // Window Extended Style
	DWORD		dwStyle;						                            // Window Style

	RECT WindowRect;							                          // Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long)0;				                		    // Set Left Value To 0
	WindowRect.right = (long)width;						                // Set Right Value To Requested Width
	WindowRect.top = (long)0;							                    // Set Top Value To 0
	WindowRect.bottom = (long)height;						              // Set Bottom Value To Requested Height

	fullscreen = fullscreenflag;						                  // Set The Global Fullscreen Flag
	hInstance = GetModuleHandle(NULL);			        // Grab An Instance For Our Window
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Move, And Own DC For Window
	wc.lpfnWndProc = (WNDPROC)WndProc;					        // WndProc Handles Messages
	wc.cbClsExtra = 0;							        	          // No Extra Window Data
	wc.cbWndExtra = 0;						        	        	  // No Extra Window Data
	wc.hInstance = hInstance;					  	        	  // Set The Instance
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);			  // Load The Default Icon
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);		  	// Load The Arrow Pointer
	wc.hbrBackground = NULL;						                    // No Background Required For GL
	wc.lpszMenuName = NULL;						                    // We Don't Want A Menu
	wc.lpszClassName = "OpenGL";					                  // Set The Class Name

	if (!RegisterClass(&wc)) {						                // Attempt To Register The Window Class
		MessageBox(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;							                            // Exit And Return FALSE
	}

	if (fullscreen) {								// Attempt Fullscreen Mode?
		DEVMODE dmScreenSettings;					// Device Mode
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));		// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth = width;			// Selected Screen Width
		dmScreenSettings.dmPelsHeight = height;			// Selected Screen Height
		dmScreenSettings.dmBitsPerPel = bits;				// Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			// If The Mode Fails, Offer Two Options.  Quit Or Run In A Window.
			if (MessageBox(NULL, "The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", "NeHe GL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				fullscreen = FALSE;				// Select Windowed Mode (Fullscreen=FALSE)
			}
			else {
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL, "Program Will Now Close.", "ERROR", MB_OK | MB_ICONSTOP);
				return FALSE;					// Exit And Return FALSE
			}
		}
	}
	if (fullscreen) {								// Are We Still In Fullscreen Mode?
		dwExStyle = WS_EX_APPWINDOW;					// Window Extended Style
		dwStyle = WS_POPUP;						// Windows Style
												//ShowCursor(FALSE);						// Hide Mouse Pointer
	}
	else {
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle = WS_OVERLAPPEDWINDOW;					// Windows Style
	}
	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size
	if (!(hWnd = CreateWindowEx(dwExStyle,				// Extended Style For The Window
		"OpenGL",				// Class Name
		title,					// Window Title
		WS_CLIPSIBLINGS |			// Required Window Style
		WS_CLIPCHILDREN |			// Required Window Style
		dwStyle,				// Selected Window Style
		0, 0,					// Window Position
		WindowRect.right - WindowRect.left,	// Calculate Adjusted Window Width
		WindowRect.bottom - WindowRect.top,	// Calculate Adjusted Window Height
		NULL,					// No Parent Window
		NULL,					// No Menu
		hInstance,				// Instance
		NULL))) {					// Don't Pass Anything To WM_CREATE
		KillGLWindow();							// Reset The Display
		MessageBox(NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}
	static	PIXELFORMATDESCRIPTOR pfd =					// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),					// Size Of This Pixel Format Descriptor
		1,								// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,						// Must Support Double Buffering
		PFD_TYPE_RGBA,							// Request An RGBA Format
		bits,								// Select Our Color Depth
		0, 0, 0, 0, 0, 0,						// Color Bits Ignored
		0,								// No Alpha Buffer
		0,								// Shift Bit Ignored
		0,								// No Accumulation Buffer
		0, 0, 0, 0,							// Accumulation Bits Ignored
		16,								// 16Bit Z-Buffer (Depth Buffer)
		0,								// No Stencil Buffer
		0,								// No Auxiliary Buffer
		PFD_MAIN_PLANE,							// Main Drawing Layer
		0,								// Reserved
		0, 0, 0								// Layer Masks Ignored
	};
	if (!(hDC = GetDC(hWnd))) {							// Did We Get A Device Context?
		KillGLWindow();							// Reset The Display
		MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}
	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd))) {				// Did Windows Find A Matching Pixel Format?
		KillGLWindow();							// Reset The Display
		MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}
	if (!SetPixelFormat(hDC, PixelFormat, &pfd)) {				// Are We Able To Set The Pixel Format?
		KillGLWindow();							// Reset The Display
		MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}
	if (!(hRC = wglCreateContext(hDC))) {					// Are We Able To Get A Rendering Context?
		KillGLWindow();							// Reset The Display
		MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}
	if (!wglMakeCurrent(hDC, hRC)) {						// Try To Activate The Rendering Context
		KillGLWindow();							// Reset The Display
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}
	ShowWindow(hWnd, SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);								// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);						// Set Up Our Perspective GL Screen
	if (!InitGL()) {								// Initialize Our Newly Created GL Window
		KillGLWindow();							// Reset The Display
		MessageBox(NULL, "Initialization Failed.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}
	return TRUE;								// Success
}

LRESULT CALLBACK WndProc(HWND	hWnd,					// Handle For This Window
	UINT	uMsg,					// Message For This Window
	WPARAM	wParam,					// Additional Message Information
	LPARAM	lParam)					// Additional Message Information
{
	switch (uMsg)								// Check For Windows Messages
	{
	case WM_ACTIVATE:						// Watch For Window Activate Message
	{
		if (!HIWORD(wParam))					// Check Minimization State
		{
			active = TRUE;					// Program Is Active
		}
		else
		{
			active = FALSE;					// Program Is No Longer Active
		}

		return 0;						// Return To The Message Loop
	}
	case WM_SYSCOMMAND:						// Intercept System Commands
	{
		switch (wParam)						// Check System Calls
		{
		case SC_SCREENSAVE:				// Screensaver Trying To Start?
		case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
			return 0;					// Prevent From Happening
		}
		break;							// Exit
	}
	case WM_CLOSE:							// Did We Receive A Close Message?
	{
		PostQuitMessage(0);					// Send A Quit Message
		return 0;						// Jump Back
	}
	case WM_KEYDOWN:						// Is A Key Being Held Down?
	{
		keys[wParam] = TRUE;					// If So, Mark It As TRUE
		return 0;						// Jump Back
	}
	case WM_KEYUP:							// Has A Key Been Released?
	{
		keys[wParam] = FALSE;					// If So, Mark It As FALSE
		return 0;						// Jump Back
	}
	case WM_SIZE:							// Resize The OpenGL Window
	{
		ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));		// LoWord=Width, HiWord=Height
		return 0;						// Jump Back
	}
	}
	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void glwin(digital_elevation_model * m, renderer & r, triangle_surface * s)
{
	HINSTANCE	hInstance = 0;				// Instance
	HINSTANCE	hPrevInstance = 0;				// Previous Instance
	LPSTR		lpCmdLine = 0;				// Command Line Parameters
	int		nCmdShow = 0;				// Window Show State

	MSG	msg;								// Windows Message Structure
	BOOL	done = FALSE;							// Bool Variable To Exit Loop

													// Ask The User Which Screen Mode They Prefer
													//if (MessageBox(NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION)==IDNO) {
													//	fullscreen=FALSE;						// Windowed Mode
													//}
	fullscreen = FALSE;
	// Create Our OpenGL Window
	if (!CreateGLWindow("GL Dem Renderer", 640, 480, 16, fullscreen)) {
		return;							// Quit If Window Was Not Created
	}
	while (!done) {								// Loop That Runs Until done=TRUE
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {			// Is There A Message Waiting?
			if (msg.message == WM_QUIT) {				// Have We Received A Quit Message?
				done = TRUE;					// If So done=TRUE
			}
			else {							// If Not, Deal With Window Messages
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else {								// If There Are No Messages
											// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if (active) {						// Program Active?
				if (keys[VK_ESCAPE]) {				// Was ESC Pressed?
					done = TRUE;				// ESC Signalled A Quit
				}
				else {						// Not Time To Quit, Update Screen
					DrawGLScene(m, r, s);				// Draw The Scene
					SwapBuffers(hDC);			// Swap Buffers (Double Buffering)
				}
			}
			//if (keys[VK_F1]) {					// Is F1 Being Pressed?
			//	keys[VK_F1]=FALSE;				// If So Make Key FALSE
			//	KillGLWindow();					// Kill Our Current Window
			//	fullscreen=!fullscreen;				// Toggle Fullscreen / Windowed Mode
			//	// Recreate Our OpenGL Window
			//	if (!CreateGLWindow("GLdemRenderer",640,480,16,fullscreen)) {
			//		return;				// Quit If Window Was Not Created
			//	}
			//}
		}
	}
	// Shutdown
	KillGLWindow();								// Kill The Window
	return;							// Exit The Program
}
#endif
