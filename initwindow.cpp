
#include <Application.h>
#include <Button.h>
#include <CheckBox.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <Screen.h>
#include <SerialPort.h>
#include <stdio.h>

#include "globals.h"
#include "initwindow.h"

const uint32	MSG_IWCBLOG		= 'IW01';
const uint32	MSG_IWCBTERM	= 'IW02';
const uint32	MSG_IWBUTOK		= 'IW03';
const uint32	MSG_IWBUTQUIT	= 'IW04';
const uint32	MSG_IWDEVICE	= 'IW05';

initWindow::initWindow(const char *name) : BWindow(
		BRect(100,100,430,280),
		name,
		B_FLOATING_WINDOW_LOOK,
		B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE) {

	// initialize w/ defaults (or read config here)
	log = false;
	term = false;
	device = "";
	// my stuff
	log = term = true; device = "usb0";

	// center onscreen
	BScreen	*screen = new BScreen();
	MoveTo((int)((screen->Frame().Width()-Bounds().Width())/2), (int)((screen->Frame().Height()-Bounds().Height())/2));
	delete screen;

	BView *view = new BView(this->Bounds(), "initView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	this->AddChild(view);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	BRect r = view->Bounds();
	r.InsetBy(16,10);
	BRect s = r;
	s.top = 25; s.bottom = s.top + 30;

	// enumerate ports to populate popupmenu
	BSerialPort *port = new BSerialPort;
	nports = port->CountDevices();

	BPopUpMenu *menu = new BPopUpMenu(_("[no device]"));
	BMessage *msg;
	BMenuItem *item;
	int n, i=0;
	char devName[B_OS_NAME_LENGTH];

	for (n = nports-1; n >=0; n--, i++) {
		port->GetDeviceName(n, devName);
		msg = new BMessage(MSG_IWDEVICE);
		msg->AddString("_dev",devName);
		item = new BMenuItem(devName,msg);
		if (devName == device)
			item->SetMarked(true);
		menu->AddItem(item);
	}
	BMenuField *devmf = new BMenuField(s,"iw_devmf",_("Select serial port:"), menu, B_FOLLOW_LEFT|B_FOLLOW_TOP, B_WILL_DRAW);
	devmf->SetAlignment(B_ALIGN_RIGHT);
	view->AddChild(devmf);

	BRect t = r;
	t.top = 80; t.bottom = t.top + 15;
	view->AddChild(cb_log = new BCheckBox(t, "iw_cblog", _("Save communication log to ~/bemobile.log"), new BMessage(MSG_IWCBLOG)));
	t.OffsetBy(0,25);
	view->AddChild(cb_term = new BCheckBox(t, "iw_cbterm", _("Show terminal log window"), new BMessage(MSG_IWCBTERM)));

	view->AddChild(but_quit = new BButton(BRect(10,140,100,170), "iw_butquit", _("Quit"), new BMessage(MSG_IWBUTQUIT)));
	view->AddChild(but_ok = new BButton(BRect(230,140,320,170), "iw_butok", _("OK"), new BMessage(MSG_IWBUTOK)));
	but_ok->MakeDefault(true);

	// update
	cb_log->SetValue(log ? B_CONTROL_ON : B_CONTROL_OFF);
	cb_term->SetValue(term ? B_CONTROL_ON : B_CONTROL_OFF);
}

initWindow::~initWindow() {

}

void initWindow::MessageReceived(BMessage *msg) {
	const char *dev;
	if (msg) {
		switch (msg->what) {
			case MSG_IWDEVICE:
				if (msg->FindString("_dev",&dev) == B_OK)
					device = dev;
				break;
			case MSG_IWBUTQUIT:
				be_app->PostMessage(B_QUIT_REQUESTED);
				break;
			case MSG_IWCBLOG:
				log = (cb_log->Value() == B_CONTROL_ON);
				break;
			case MSG_IWCBTERM:
				term = (cb_term->Value() == B_CONTROL_ON);
				break;
			case MSG_IWBUTOK:
				DoFinish();
				break;
			default:
				break;
		}
	}
}

void initWindow::DoFinish(void) {
	// pass back values
	BMessage *msg = new BMessage(MSG_INITMSG);
	// prepare message
	msg->AddBool("_log",log);
	msg->AddBool("_term",term);
	msg->AddString("_dev",device.String());
	// pass
	be_app->PostMessage(msg);
}

bool initWindow::QuitRequested() {
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}
