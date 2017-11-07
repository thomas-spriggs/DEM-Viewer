#pragma once
#include "stdafx.h"

void error_critical( string description );
void error_warning( string description );

enum error_code{
	IMAGE_IS_EMPTY,
	INVALID_IMAGE_FILE,
	OUT_OF_MEMORY
};

struct error {
	error_code code;
	error( error_code e ) : code( e ) {};
};

