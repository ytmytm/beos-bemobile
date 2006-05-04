
#include "bemobile.h"

int AppReturnValue(0);

int main (void) {
	AppReturnValue = B_ERROR;
	BeMobileApp myApp;
	myApp.Run();
	return AppReturnValue;
}
