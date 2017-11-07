#pragma once
#include "stdafx.h"

void runLexer( FILE * input );

void runScript( string fileName ) {
	if( file_exists( fileName.c_str() ) ) {
		FILE * f = fopen( fileName.c_str(), "rt" );
		if( f != NULL ) {
			runLexer( f );
			clearerr( f );
			int tmp = fclose( f );
			dialog_debug("file closed");
			f = NULL;
		}
	}
}

void execCommand( command c ) {
	switch( c.type ) {
		case COMMAND_TYPE_LOAD_DEM:
			dialog_debug("script executed load dem");
			loadDem( c.data.loadDem.fileName );
			break;
		case COMMAND_TYPE_SAVE_RENDER:
			saveRender(  c.data.saveRender.fileName != NULL ? c.data.saveRender.fileName : "" , c.data.saveRender.width, c.data.saveRender.height );
			break;
		case COMMAND_TYPE_SET_POSITION:
			dialog_debug("script executed set position");
			setPosition( c.data.setPosition.lon, c.data.setPosition.lat, c.data.setPosition.rad );
			break;
		case COMMAND_TYPE_SET_LIGHT:
			setLight( c.data.setLight.lon, c.data.setLight.lat );
			break;
		case COMMAND_TYPE_SCALE_RADIUS:
			scaleRadius( c.data.scaleRadius.min, c.data.scaleRadius.step );
			break;
		case COMMAND_TYPE_ROTATE_CAMERA:
			rotateCamera( c.data.rotateCamera.direction, -c.data.rotateCamera.angle * data::PI / 180 );
			break;
		case COMMAND_TYPE_FOV:
			setFOV( c.data.fov.angle /* * data::PI / 180*/ );
			break;
		default:
			error_warning("Unexpected command");
			break;
	}
  return;
}
