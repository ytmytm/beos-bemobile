
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
	switch(msg->what) {
		case MSG_INITMSG:
			{
				startWindow->Hide();
// XXX show window that setup is going on
				if (gsm->initDevice(msg)) {
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
	aboutDialog->SetText(_("Mobile phone manager for BeOS\n\nDo you like this program? Do you want to help?\n\nPlease donate via http://www.moneybookers.com\nto <mw4096@gmail.com>"));
	aboutDialog->Show();
}
