
#include "bemobile.h"
#include "dialabout.h"
#include "globals.h"
#include "mainwindow.h"

BeMobileApp::BeMobileApp() : BApplication(APP_SIGNATURE) {
	mainWindow = new BeMobileMainWindow(APP_NAME);
	if (mainWindow != NULL) {
		mainWindow->Show();
	} else {
		AppReturnValue = B_NO_MEMORY;
		be_app->PostMessage(B_QUIT_REQUESTED);
	}
}

BeMobileApp::~BeMobileApp() {
	if (mainWindow != NULL)
		if (mainWindow->LockWithTimeout(30000000) == B_OK)
			mainWindow->Quit();
}

void BeMobileApp::ReadyToRun() {
	// sth etc. just before running Run()
}

void BeMobileApp::MessageReceived(BMessage *msg) {
	switch(msg->what) {
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
