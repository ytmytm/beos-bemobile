
#ifndef _BEMOBILEAPP_H
#define _BEMOBILEAPP_H

#include <Application.h>

class dialAbout;
class initWindow;
class BeMobileMainWindow;
class GSM;

class BeMobileApp : public BApplication {
	public:
		BeMobileApp();
		~BeMobileApp();
		virtual void ReadyToRun();
		virtual void MessageReceived(BMessage *message);
		virtual void AboutRequested(void);

	private:
		GSM *gsm;
		initWindow *startWindow;
		BeMobileMainWindow *mainWindow;
		dialAbout *aboutDialog;

};

#endif
