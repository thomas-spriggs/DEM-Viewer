/* Lexer for the Digital Elevation Modeller Scripting Language */
/* for use with flex */

  #include "stdafx.h"
  
  #ifndef NULL
  #define NULL 0
  #endif

  #define YY_NEVER_INTERACTIVE 1
  
  int line_num = 1;
  command currentCommand;
  
  enum {
    PARAMETER_NONE,
    PARAMETER_FILE,
    PARAMETER_LON,
    PARAMETER_LAT,
    PARAMETER_RAD,
		PARAMETER_MIN,
		PARAMETER_LEVEL,
		PARAMETER_ANGLE
  } currentParameter;

  char * StripQuotes( char * quoted ) {
    int length = (int)strlen( quoted );
    if( quoted[length-1] == '\n' ) {
      length--;
      quoted[length] = 0;
    }
    if( quoted[length-1] != '\"' ) {
      printf("Error: missing \" on line %d\n", line_num);
    } else {
      quoted[length-1] = 0;
    }
    while( quoted[0] != '\"' ) {
      ++quoted;
    }
    quoted++;
    return quoted;
  }

  void issueCommand() {
    switch( currentCommand.type ) {
    case COMMAND_TYPE_NONE:
      break;
    case COMMAND_TYPE_LOAD_DEM:
      execCommand( currentCommand );
      if( currentCommand.data.loadDem.fileName != NULL ) {
        free( currentCommand.data.loadDem.fileName );
        currentCommand.data.loadDem.fileName = NULL;
      }
      break;
		case COMMAND_TYPE_SAVE_RENDER:
      execCommand( currentCommand );
      if( currentCommand.data.saveRender.fileName != NULL ) {
        free( currentCommand.data.saveRender.fileName );
        currentCommand.data.saveRender.fileName = NULL;
      }
      break;
    case COMMAND_TYPE_SET_POSITION:
      execCommand( currentCommand );
      break;
    case COMMAND_TYPE_SET_LIGHT:
      execCommand( currentCommand );
      break;
		case COMMAND_TYPE_SCALE_RADIUS:
			execCommand( currentCommand );
      break;
		case COMMAND_TYPE_ROTATE_CAMERA:
			if( currentCommand.data.rotateCamera.angle != 0 ) {
				execCommand( currentCommand );
			}
      break;
		case COMMAND_TYPE_FOV:
			execCommand( currentCommand );
      break;
    default:
      break;
    }
    currentCommand.type = COMMAND_TYPE_NONE;
  }

  //int yylex();

	int yywrap() {
		return 1;
	}

	#define YY_SKIP_YYWRAP

%%
stop |
<<EOF>>									{
													issueCommand();
													yyterminate();
													return 0;
												}
^#.*\n									{ /* comment */ }
\n											{
													++line_num;
													issueCommand();
												}
[ \t]*									/* ignore white space */ ;
LoadDem									{
													printf("LoadDem encountered\n");
													issueCommand();
													currentCommand.type = COMMAND_TYPE_LOAD_DEM;
													currentCommand.data.loadDem.fileName = NULL;
												}
SaveRender							{
													issueCommand();
													currentCommand.type = COMMAND_TYPE_SAVE_RENDER;
													currentCommand.data.saveRender.fileName = NULL;
													currentCommand.data.saveRender.width = currentCommand.data.saveRender.height = 0;
												}
ScaleRadius							{
													printf("ScaleRadius encountered\n");
													issueCommand();
													currentCommand.type = COMMAND_TYPE_SCALE_RADIUS;
												}
SetPosition							{
													printf("SetPosition encountered\n");
													issueCommand();
													currentCommand.type = COMMAND_TYPE_SET_POSITION;
												}
SetLight								{
													issueCommand();
													currentCommand.type = COMMAND_TYPE_SET_LIGHT;
													currentCommand.data.setLight.lon = 0;
													currentCommand.data.setLight.lat = 0;
												}
RotateCameraX						{
													issueCommand();
													currentCommand.type = COMMAND_TYPE_ROTATE_CAMERA;
													currentCommand.data.rotateCamera.direction = linear_math::x_axis;
													currentCommand.data.rotateCamera.angle = 0;
												}
RotateCameraY						{
													issueCommand();
													currentCommand.type = COMMAND_TYPE_ROTATE_CAMERA;
													currentCommand.data.rotateCamera.direction = linear_math::y_axis;
													currentCommand.data.rotateCamera.angle = 0;
												}
RotateCameraZ |
RotateCameraPhi					{
													issueCommand();
													currentCommand.type = COMMAND_TYPE_ROTATE_CAMERA;
													currentCommand.data.rotateCamera.direction = linear_math::z_axis;
													currentCommand.data.rotateCamera.angle = 0;
												}
SetZoom									{
													issueCommand();
													currentCommand.type = COMMAND_TYPE_FOV;
												}
file |
File										{
													printf("Start of file parameter specification encountered\n");
													currentParameter = PARAMETER_FILE;
												}
Longitude |
lon											{
													printf("Longitude specification encountered\n");
													currentParameter = PARAMETER_LON;
												}
Latitude |
lat											{
													printf("Latitude specification encountered\n");
													currentParameter = PARAMETER_LAT;
												}
Radius |
rad											{
													printf("Radius specification encountered\n");
													currentParameter = PARAMETER_RAD;
												}
min											{
													currentParameter = PARAMETER_MIN;
												}
level										{
													currentParameter = PARAMETER_LEVEL;
												}
angle										{
													currentParameter = PARAMETER_ANGLE;
												}
=[ \t]*\"[^"\n]*["\n]		{ /* Parameter */
                          /*printf("Matched parameter :%s",yytext);*/ 
                          char * value = StripQuotes(yytext);
                          printf("Parameter value: \"%s\"\n",value);
                          switch( currentParameter ) {
														case PARAMETER_NONE:
															break;
														case PARAMETER_FILE: 
															if( currentCommand.type == COMMAND_TYPE_LOAD_DEM ) {
																int length = (int)strlen (value);
																currentCommand.data.loadDem.fileName = (char *) malloc( length+1 );
																strncpy( currentCommand.data.loadDem.fileName, value, length );
																currentCommand.data.loadDem.fileName[length] = 0;
															}
															break;
														case PARAMETER_LON:
															if( currentCommand.type == COMMAND_TYPE_SET_POSITION ) {
																currentCommand.data.setPosition.lon = atof( value );
															}
															if( currentCommand.type == COMMAND_TYPE_SET_LIGHT ) {
																currentCommand.data.setLight.lon = atof( value );
															}
															break;
														case PARAMETER_LAT:
															if( currentCommand.type == COMMAND_TYPE_SET_POSITION ) {
																currentCommand.data.setPosition.lat = atof( value );
															}
															if( currentCommand.type == COMMAND_TYPE_SET_LIGHT ) {
																currentCommand.data.setLight.lat = atof( value );
															}
															break;
														case PARAMETER_RAD:
															if( currentCommand.type == COMMAND_TYPE_SET_POSITION ) {
																currentCommand.data.setPosition.rad = atof( value );
															}
															break;
														case PARAMETER_MIN:
															if( currentCommand.type == COMMAND_TYPE_SCALE_RADIUS ) {
																currentCommand.data.scaleRadius.min = atof( value );
															}
															break;
														case PARAMETER_LEVEL:
															if( currentCommand.type == COMMAND_TYPE_SCALE_RADIUS ) {
																currentCommand.data.scaleRadius.step = atof( value );
															}
															break;
														case PARAMETER_ANGLE:
															if( currentCommand.type == COMMAND_TYPE_ROTATE_CAMERA ) {
																currentCommand.data.rotateCamera.angle = atof( value );
															} else if( currentCommand.type == COMMAND_TYPE_FOV ) {
																currentCommand.data.fov.angle = atof( value );
															}
															break;
														default:
															break;
                          }
                        }
.												{printf("Unexpected input on line %d\n", line_num) /* junk anything else */ ;}

%%

void runLexer() {
	currentCommand.type = COMMAND_TYPE_NONE;
	line_num = 1;
	yylex();
}

void runLexer( FILE * input ) {
  yyin = input;
  runLexer();
}

//int main() {
//  runLexer();
//  return 0;
//}