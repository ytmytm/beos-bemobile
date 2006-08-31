
#include <Application.h>
#include <Button.h>
#include <CheckBox.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <Screen.h>
#include <SerialPort.h>
#include <TextControl.h>
#include <stdio.h>

#include "globals.h"
#include "initwindow.h"

const uint32	MSG_IWCBLOG		= 'IW01';
const uint32	MSG_IWCBTERM	= 'IW02';
const uint32	MSG_IWBUTOK		= 'IW03';
const uint32	MSG_IWBUTQUIT	= 'IW04';
const uint32	MSG_IWDEVICE	= 'IW05';
const uint32	MSG_IWINITSTR	= 'IW06';
// settings
const uint32	SET_PARITY		= 'SPAR';
const uint32	SET_DATABITS	= 'SDAT';
const uint32	SET_STOPBITS	= 'SSTO';
const uint32	SET_FLOWCONTROL	= 'SFLO';
const uint32	SET_BAUDRATE	= 'SBAU';
const uint32	SET_DTR			= 'SDTR';
const uint32	SET_RTS			= 'SRTS';

initWindow::initWindow(const char *name) : BWindow(
		BRect(100,100,430,280),
		name,
		B_FLOATING_WINDOW_LOOK,
		B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE) {

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

	device = "usb0";	// sane default, possibly read from config
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

	BRect u = r;
	u.top = s.bottom+5; u.bottom = u.top + 15;
	view->AddChild(initStr = new BTextControl(u,"iw_initstring",_("Init string"),NULL,new BMessage(MSG_IWINITSTR)));
	initStr->SetDivider(80);

	BRect t = r;
	t.top = 80; t.bottom = t.top + 15;
	view->AddChild(cb_log = new BCheckBox(t, "iw_cblog", _("Save communication log to ~/bemobile.log"), new BMessage(MSG_IWCBLOG)));
	t.OffsetBy(0,25);
	view->AddChild(cb_term = new BCheckBox(t, "iw_cbterm", _("Show terminal log window"), new BMessage(MSG_IWCBTERM)));

	view->AddChild(but_quit = new BButton(BRect(10,140,100,170), "iw_butquit", _("Quit"), new BMessage(MSG_IWBUTQUIT)));
	view->AddChild(but_ok = new BButton(BRect(230,140,320,170), "iw_butok", _("OK"), new BMessage(MSG_IWBUTOK)));
	but_ok->MakeDefault(true);

	// serial port settings menu
	r = view->Bounds(); r.bottom = 20;
	BMenuBar *menuBar = new BMenuBar(r, "initMenuBar");
	view->AddChild(menuBar);
	BMenu *setMenu = new BMenu(_("Serial port settings"), B_ITEMS_IN_COLUMN);
	BMenu *mParity = new BMenu(_("Parity"), B_ITEMS_IN_COLUMN);
	msg = new BMessage(SET_PARITY); msg->AddInt32("_parity", B_NO_PARITY);
	mParity->AddItem(parItems[0] = new BMenuItem(_("None"), msg));
	msg = new BMessage(SET_PARITY); msg->AddInt32("_parity", B_EVEN_PARITY);
	mParity->AddItem(parItems[1] = new BMenuItem(_("Even"), msg));
	msg = new BMessage(SET_PARITY); msg->AddInt32("_parity", B_ODD_PARITY);
	mParity->AddItem(parItems[2] = new BMenuItem(_("Odd"), msg));
	BMenu *mDataBits = new BMenu(_("Data bits"), B_ITEMS_IN_COLUMN);
	msg = new BMessage(SET_DATABITS); msg->AddInt32("_databits", B_DATA_BITS_8);
	mDataBits->AddItem(datItems[0] = new BMenuItem(_("8"), msg));
	msg = new BMessage(SET_DATABITS); msg->AddInt32("_databits", B_DATA_BITS_7);
	mDataBits->AddItem(datItems[1] = new BMenuItem(_("7"), msg));
	BMenu *mStopBits = new BMenu(_("Stop bits"), B_ITEMS_IN_COLUMN);
	msg = new BMessage(SET_STOPBITS); msg->AddInt32("_stopbits", B_STOP_BITS_1);
	mStopBits->AddItem(stoItems[0] = new BMenuItem(_("1"), msg));
	msg = new BMessage(SET_STOPBITS); msg->AddInt32("_stopbits", B_STOP_BITS_2);
	mStopBits->AddItem(stoItems[1] = new BMenuItem(_("2"), msg));
	BMenu *mFlowCtrl = new BMenu(_("Flow control"), B_ITEMS_IN_COLUMN);
	msg = new BMessage(SET_FLOWCONTROL); msg->AddInt32("_flowcontrol", B_HARDWARE_CONTROL);
	mFlowCtrl->AddItem(floItems[0] = new BMenuItem(_("Hardware"), msg));
	msg = new BMessage(SET_FLOWCONTROL); msg->AddInt32("_flowcontrol", B_SOFTWARE_CONTROL);
	mFlowCtrl->AddItem(floItems[1] = new BMenuItem(_("Software"), msg));
	msg = new BMessage(SET_FLOWCONTROL); msg->AddInt32("_flowcontrol", B_HARDWARE_CONTROL+B_SOFTWARE_CONTROL);
	mFlowCtrl->AddItem(floItems[2] = new BMenuItem(_("Both"), msg));
	msg = new BMessage(SET_FLOWCONTROL); msg->AddInt32("_flowcontrol", 0);
	mFlowCtrl->AddItem(floItems[3] = new BMenuItem(_("None"), msg));
	BMenu *mBaudrate = new BMenu(_("Baud rate"), B_ITEMS_IN_COLUMN);
	msg = new BMessage(SET_BAUDRATE); msg->AddInt32("_baudrate", B_115200_BPS);
	mBaudrate->AddItem(bauItems[0] = new BMenuItem(_("115200"), msg));
	msg = new BMessage(SET_BAUDRATE); msg->AddInt32("_baudrate", B_57600_BPS);
	mBaudrate->AddItem(bauItems[1] = new BMenuItem(_("57600"), msg));
	msg = new BMessage(SET_BAUDRATE); msg->AddInt32("_baudrate", B_38400_BPS);
	mBaudrate->AddItem(bauItems[2] = new BMenuItem(_("38400"), msg));
	msg = new BMessage(SET_BAUDRATE); msg->AddInt32("_baudrate", B_19200_BPS);
	mBaudrate->AddItem(bauItems[3] = new BMenuItem(_("19200"), msg));
	msg = new BMessage(SET_BAUDRATE); msg->AddInt32("_baudrate", B_9600_BPS);
	mBaudrate->AddItem(bauItems[4] = new BMenuItem(_("9600"), msg));
	msg = new BMessage(SET_BAUDRATE); msg->AddInt32("_baudrate", B_4800_BPS);
	mBaudrate->AddItem(bauItems[5] = new BMenuItem(_("4800"), msg));
	msg = new BMessage(SET_BAUDRATE); msg->AddInt32("_baudrate", B_2400_BPS);
	mBaudrate->AddItem(bauItems[6] = new BMenuItem(_("2400"), msg));

	menuBar->AddItem(setMenu);
	setMenu->AddItem(mParity);
	setMenu->AddItem(mDataBits);
	setMenu->AddItem(mStopBits);
	setMenu->AddItem(mFlowCtrl);
	setMenu->AddItem(mBaudrate);
	setMenu->AddItem(dtrItem = new BMenuItem(_("DTR active"), new BMessage(SET_DTR)));
	setMenu->AddItem(rtsItem = new BMenuItem(_("RTS active"), new BMessage(SET_RTS)));

	// defaults, possibly read from config (if any)
	// initialize w/ defaults (or read config here)
	// [device] is set earlier (see menu construction)
	log = term = true;
	//
	initStr->SetText("ATZ");
	// defaults for 19200,8N1,hard,dtr+rts
	parity = B_NO_PARITY;
	parItems[0]->SetMarked(true);
	databits = B_DATA_BITS_8;
	datItems[0]->SetMarked(true);
	stopbits = B_STOP_BITS_1;
	stoItems[0]->SetMarked(true);
	flowcontrol = B_HARDWARE_CONTROL;
	floItems[0]->SetMarked(true);
	baudrate = B_19200_BPS;
	bauItems[3]->SetMarked(true);
	dtr = true;
	dtrItem->SetMarked(dtr);
	rts = true;
	rtsItem->SetMarked(rts);
	// log state
	cb_log->SetValue(log ? B_CONTROL_ON : B_CONTROL_OFF);
	cb_term->SetValue(term ? B_CONTROL_ON : B_CONTROL_OFF);
}

initWindow::~initWindow() {

}

void initWindow::MessageReceived(BMessage *msg) {
	const char *dev;
	int32 val;
	void *ptr;
	if (msg) {
		switch (msg->what) {
			case MSG_IWDEVICE:
				if (msg->FindString("_dev",&dev) == B_OK)
					device = dev;
				break;
			case MSG_IWINITSTR:
				// validate somehow?
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
			case SET_PARITY:
				if (msg->FindInt32("_parity",&val) == B_OK) {
					parity = val;
					if (msg->FindPointer("source",&ptr) == B_OK) {
						for (unsigned int i=0;i<sizeof(parItems)/sizeof(parItems[0]);i++)
							parItems[i]->SetMarked(false);
						BMenuItem *item = static_cast<BMenuItem*>(ptr);
						item->SetMarked(true);
					}
				}
				break;
			case SET_DATABITS:
				if (msg->FindInt32("_databits",&val) == B_OK) {
					databits = val;
					if (msg->FindPointer("source",&ptr) == B_OK) {
						for (unsigned int i=0;i<sizeof(datItems)/sizeof(datItems[0]);i++)
							datItems[i]->SetMarked(false);
						BMenuItem *item = static_cast<BMenuItem*>(ptr);
						item->SetMarked(true);
					}
				}
				break;
			case SET_STOPBITS:
				if (msg->FindInt32("_stopbits",&val) == B_OK) {
					stopbits = val;
					if (msg->FindPointer("source",&ptr) == B_OK) {
						for (unsigned int i=0;i<sizeof(stoItems)/sizeof(stoItems[0]);i++)
							stoItems[i]->SetMarked(false);
						BMenuItem *item = static_cast<BMenuItem*>(ptr);
						item->SetMarked(true);
					}
				}
				break;
			case SET_FLOWCONTROL:
				if (msg->FindInt32("_flowcontrol",&val) == B_OK) {
					flowcontrol = val;
					if (msg->FindPointer("source",&ptr) == B_OK) {
						for (unsigned int i=0;i<sizeof(floItems)/sizeof(floItems[0]);i++)
							floItems[i]->SetMarked(false);
						BMenuItem *item = static_cast<BMenuItem*>(ptr);
						item->SetMarked(true);
					}
				}
				break;
			case SET_BAUDRATE:
				if (msg->FindInt32("_baudrate",&val) == B_OK) {
					baudrate = val;
					if (msg->FindPointer("source",&ptr) == B_OK) {
						for (unsigned int i=0;i<sizeof(bauItems)/sizeof(bauItems[0]);i++)
							bauItems[i]->SetMarked(false);
						BMenuItem *item = static_cast<BMenuItem*>(ptr);
						item->SetMarked(true);
					}
				}
				break;
			case SET_DTR:
				if (msg->FindPointer("source",&ptr) == B_OK) {
					BMenuItem *item = static_cast<BMenuItem*>(ptr);
					dtr = ! item->IsMarked();
					item->SetMarked(dtr);
				}
				break;				
			case SET_RTS:
				if (msg->FindPointer("source",&ptr) == B_OK) {
					BMenuItem *item = static_cast<BMenuItem*>(ptr);
					rts = ! item->IsMarked();
					item->SetMarked(rts);
				}
				break;				
			default:
				BWindow::MessageReceived(msg);
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
	msg->AddInt32("_parity",parity);
	msg->AddInt32("_databits",databits);
	msg->AddInt32("_stopbits",stopbits);
	msg->AddInt32("_flowcontrol",flowcontrol);
	msg->AddInt32("_baudrate",baudrate);
	msg->AddBool("_dtr",dtr);
	msg->AddBool("_rts",rts);
	msg->AddString("_initstring",initStr->Text());
	// pass
	be_app->PostMessage(msg);
}

bool initWindow::QuitRequested() {
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}
