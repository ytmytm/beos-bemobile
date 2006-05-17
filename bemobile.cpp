
#include <Alert.h>
#include <stdio.h>

#include "bemobile.h"
#include "dialabout.h"
#include "globals.h"
#include "gsm.h"
#include "initwindow.h"
#include "mainwindow.h"

BeMobileApp::BeMobileApp() : BApplication(APP_SIGNATURE) {

	// startup stuff
	gsm = new GSM();
	startWindow = new initWindow(APP_NAME);
	if (startWindow != NULL) {
		startWindow->Show();
	} else {
		AppReturnValue = B_NO_MEMORY;
		be_app->PostMessage(B_QUIT_REQUESTED);
	}
}

BeMobileApp::~BeMobileApp() {
	if (mainWindow != NULL)
		if (mainWindow->LockWithTimeout(30000000) == B_OK)
			mainWindow->Quit();
	delete gsm;
}

void BeMobileApp::ReadyToRun() {
	// sth etc. just before running Run()
}

void BeMobileApp::MessageReceived(BMessage *msg) {
	const char *dev;
	bool log, term;
	switch(msg->what) {
		case MSG_INITMSG:
			{	dev = NULL; log = false; term = false;
				msg->FindString("_dev",&dev);
				msg->FindBool("_log",&log);
				msg->FindBool("_term",&term);
				startWindow->Hide();
// XXX show window that setup is going on
				if (gsm->initDevice(dev,log,term)) {
					mainWindow = new BeMobileMainWindow(APP_NAME,gsm);
					if (mainWindow != NULL) {
						mainWindow->Show();
					} else {
						AppReturnValue = B_NO_MEMORY;
						be_app->PostMessage(B_QUIT_REQUESTED);
					}
				} else {
					BAlert *err = new BAlert(APP_NAME, _("Could not open port or device doesn't respond."), _("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
					err->Go();
					startWindow->Show();
				}
			}
			break;
		default:
			BApplication::MessageReceived(msg);
			break;
	}
}

void BeMobileApp::AboutRequested(void) {
	aboutDialog = new dialAbout(_("About"));
	aboutDialog->SetApplicationName(APP_NAME);
	aboutDialog->SetVersionNumber(APP_VERSION);
	aboutDialog->SetCopyrightString(_(B_UTF8_COPYRIGHT"2006 by Maciej Witkowiak"));
	aboutDialog->SetText(_("About text..."));
	aboutDialog->Show();
}
