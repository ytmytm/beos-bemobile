
#ifndef _BEMOBILEAPP_H
#define _BEMOBILEAPP_H

#include <Application.h>

class dialAbout;
class BeMobileMainWindow;

class BeMobileApp : public BApplication {
	public:
		BeMobileApp();
		~BeMobileApp();
		virtual void ReadyToRun();
		virtual void MessageReceived(BMessage *message);
		virtual void AboutRequested(void);

	private:
		BeMobileMainWindow *mainWindow;
		dialAbout *aboutDialog;

};

#endif
