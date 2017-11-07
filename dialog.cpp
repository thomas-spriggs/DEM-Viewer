#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN 
#include <commdlg.h>

void dialog_error( string error )
{
	MessageBoxA (NULL, error.c_str(), "Error!", MB_ICONERROR);
}

void dialog_debug( string error )
{
#ifdef _DEBUG
	MessageBoxA (NULL, error.c_str(), "Debug message", MB_ICONINFORMATION);
#endif
}

void dialog_information( string info )
{
	MessageBoxA (NULL, info.c_str(), "For your information", MB_ICONINFORMATION);
}

bool valid_gif( SDL_RWops *&file ) {
	if( file == 0 ) {
		dialog_error( "unable to open file" );
		return false;
	}
	int valid = IMG_isGIF( file );
	if( valid = 0 ) {
		dialog_error( "invalid gif file" );
		SDL_RWclose( file );
		SDL_FreeRW( file );
		file = 0;
		return false;
	}
	return true;
}

string dialog_open_gif( string title )
{
	const int FILE_NAME_BUFFER_SIZE = 1<<10;
	char file_name_buffer[FILE_NAME_BUFFER_SIZE];
	SDL_RWops *gif = 0;
	do {
		file_name_buffer[0] = 0;
		OPENFILENAME file_info = {
			sizeof( OPENFILENAME ),              // lStructSize
			0,                                   // hwndOwner
			0,                                   // hInstance
			//"gif images\0*.gif\0tiff images\0*.tiff;*.tif\08 bit png images\0*.png\0DEM Renderer Script\0*.drs\0all files\0*.*\0\0", // lpstrFilter
			"DEM Renderer Script\0*.drs\0tiff images\0*.tiff;*.tif\0gif images\0*.gif\0all supported file formats\0*.drs;*.tiff;*.tif;*.gif\0all files\0*.*\0\0", // lpstrFilter
			0,                                   // lpstrCustomFilter
			0,                                   // nMaxCustFilter
			1,                                   // nFilterIndex
			0,                                   // lpstrFile
			0,                                   // nMaxFile
			file_name_buffer,                    // lpstrFileTitle
			FILE_NAME_BUFFER_SIZE,               // nMaxFileTitle
			0,                                   // lpstrInitialDir
			title.c_str(),                       // lpstrTitle
			OFN_FILEMUSTEXIST |                  // Flags
			OFN_EXPLORER |
			OFN_FILEMUSTEXIST |
			OFN_HIDEREADONLY,
			0,                                   // nFileOffset
			0,                                   // nFileExtension
			0,                                   // lpstrDefExt
			0,                                   // lCustData
			0,                                   // lpfnHook
			0                                    // lpTemplateName
			#if (_WIN32_WINNT >= 0x0500)
		 ,0,                                   // pvReserved
			0,                                   // dwReserved
			0                                    // FlagsEx
			#endif // (_WIN32_WINNT >= 0x0500)
		};
		BOOL open_file_success = GetOpenFileName( &file_info );
		if( !open_file_success ) {
			dialog_error( "file open error" );
			return "";
		}
		gif = SDL_RWFromFile( file_name_buffer, "r" );
	}	while( !valid_gif( gif ) );
	return file_name_buffer;
}
