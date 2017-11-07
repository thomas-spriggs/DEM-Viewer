#pragma once
#include "stdafx.h"

void dialog_error( string error );
void dialog_debug( string error );
void dialog_information( string info );
//string dialog_open_gif();
string dialog_open_gif( string title = "open Digital Elevation Model data" );
