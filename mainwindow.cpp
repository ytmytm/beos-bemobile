
#include <Application.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>

#include "gsm.h"
#include "mainwindow.h"
#include "workview.h"
#include "statusview.h"

const uint32 MENU_ABOUT			= 'BM01';
const uint32 MENU_SETDATETIME	= 'BM02';
const uint32 MENU_UPDSTATUS		= 'BM03';

BeMobileMainWindow::BeMobileMainWindow(const char *windowTitle, GSM *g) : BWindow(
	BRect(150, 150, 990, 630), windowTitle, B_DOCUMENT_WINDOW, B_OUTLINE_RESIZE, B_CURRENT_WORKSPACE ) {

	BRect r;
	r = this->Bounds();
	r.top = 20;
	mainView = new workView(r);

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

	menu = new BMenu(_("File"), B_ITEMS_IN_COLUMN);
	menu->AddItem(new BMenuItem(_("About"), new BMessage(MENU_ABOUT)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(_("Quit"), new BMessage(B_QUIT_REQUESTED), 'Q'));
	menuBar->AddItem(menu);

	menu = new BMenu(_("Settings"), B_ITEMS_IN_COLUMN);
	menu->AddItem(new BMenuItem(_("Set date and time"), new BMessage(MENU_SETDATETIME)));
	menu->AddItem(updStatusItem = new BMenuItem(_("Monitor status"), new BMessage(MENU_UPDSTATUS)));
	menuBar->AddItem(menu);

	gsm = g;

	mainView->SetDevice(gsm);

	this->SetPulseRate(1000000);
	gsm->setStatusUpdate(true);
	updStatusItem->SetMarked(true);
}

BeMobileMainWindow::~BeMobileMainWindow() {

}

void BeMobileMainWindow::MessageReceived(BMessage *Message) {
	this->DisableUpdates();
	switch (Message->what) {
		case MENU_SETDATETIME:
			gsm->setDateTime();
			break;
		case MENU_UPDSTATUS:
			{
				bool s = !gsm->statusUpdateEnabled();
				gsm->setStatusUpdate(s);
				updStatusItem->SetMarked(s);
				break;
			}
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

bool BeMobileMainWindow::QuitRequested() {
//	config->position = this->Frame();
//	config->save();
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}
