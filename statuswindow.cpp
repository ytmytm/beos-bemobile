
#include <Application.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>

#include "gsm.h"
#include "statuswindow.h"
#include "statusview.h"

const uint32 MENU_ABOUT	= 'BS01';
const uint32 MENU_SETDATETIME = 'BS02';

StatusWindow::StatusWindow(const char *windowTitle, GSM *g) : BWindow(
	BRect(200, 200, 630, 550), windowTitle, B_DOCUMENT_WINDOW, B_OUTLINE_RESIZE, B_CURRENT_WORKSPACE ) {

	BRect r;
	r = this->Bounds();
	r.top = 20;
	mainView = new statusView(r);

	if (mainView == NULL) {
		AppReturnValue = B_NO_MEMORY;
		be_app->PostMessage(B_QUIT_REQUESTED);
		return;
	}

	this->AddChild(mainView);

	r = this->Bounds();
	r.bottom = 20;
	menuBar = new BMenuBar(r, "menuBar");
	this->AddChild(menuBar);

	menu = new BMenu(_("Settings"), B_ITEMS_IN_COLUMN);
	menu->AddItem(new BMenuItem(_("Set date and time"), new BMessage(MENU_SETDATETIME)));
	menuBar->AddItem(menu);

	this->SetPulseRate(1000000);

	if (g != NULL)
		this->SetDevice(g);
}

StatusWindow::~StatusWindow() {
//	delete gsm;
}

void StatusWindow::SetDevice(GSM *g) {
	gsm = g;
	mainView->SetDevice(gsm);
}

void StatusWindow::MessageReceived(BMessage *Message) {
	this->DisableUpdates();
	switch (Message->what) {
		case MENU_SETDATETIME:
			gsm->setDateTime();
			break;
		case MENU_ABOUT:
			// really should go with B_ABOUT_REQUESTED, but it doesn't work...
			be_app->AboutRequested();
			break;
		default:
			break;
	}
	mainView->MessageReceived(Message);
	BWindow::MessageReceived(Message);
	this->EnableUpdates();
}

bool StatusWindow::QuitRequested() {
//	config->position = this->Frame();
//	config->save();
//	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}
