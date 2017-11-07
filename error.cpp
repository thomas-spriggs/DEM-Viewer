#include "error.h"

void error_warning( string description )
{
	dialog_error( description );
}

void error_critical( string description )
{
	dialog_error( description );
	exit( 0 );
}
