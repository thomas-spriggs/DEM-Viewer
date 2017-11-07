#pragma once
#include "stdafx.h"
void loadDem( string fileName );
void setPosition( double lon, double lat, double rad );
void setLight( double lon, double lat );
void scaleRadius( double min, double step );
void rotateCamera( linear_math::axis axis, double angle );
void setFOV( double angle );
void saveRender( string fileName, uint width, uint height );
