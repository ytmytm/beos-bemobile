
#ifndef _globals_h
#define _globals_h
	// application signature
	#define APP_NAME "BeMobile"
	// bump it on every release
	#define APP_VERSION "0.1"
	// sync it to sqlschema CVS revision
	#define APP_SIGNATURE "application/x-vnd.generic-BeMobile"
	extern int AppReturnValue;
	// general max() function
	#define max(x,y) ( ((x)>(y)) ? (x) : (y) )
	// gettext-style translation helper
	#define _(x) (x)
//	#define _(x) SpTranslate(x)
	// Qt-style translation helper
//	#define tr(x) SpTranslate(x)
#endif
