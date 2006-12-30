//
// TODO: (inspiration in callistitem)
// - input validation (time, date, duration(number))
// - alarm must be before event time
// - duration units in menu (encoded/decoded)
// - repeat units in menu (encoded/decoded)
//

#include <Alert.h>
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <Font.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <Message.h>
#include <PopUpMenu.h>
#include <TextControl.h>

#include "globals.h"
#include "dialcalevent.h"
#include "gsm.h"

#include <stdio.h>

const uint32 BUT_SAVE	= 'CV00';
const uint32 TCTITLE	= 'CV01';
const uint32 TCDUR		= 'CV02';
const uint32 TCSDATE	= 'CV03';
const uint32 TCSTIME	= 'CV04';
const uint32 CBTIMED	= 'CV05';
const uint32 CBALARMED	= 'CV06';
const uint32 TCADATE	= 'CV07';
const uint32 TCATIME	= 'CV08';

dialNewEvent::dialNewEvent(GSM *g, struct calEvent *event) : BWindow(
	BRect(400,300,650,690),
	_("New calendar event"),
	B_TITLED_WINDOW, 0) {

	gsm = g;

	if (event != NULL) {
		this->SetTitle(_("Edit calendar event"));
		ev = event;
		newevent = false;
	} else {
		ev = new struct calEvent;
		ev->id = -1;
		ev->title = "";
		ev->timed = false;
		ev->alarm = false;
		ev->start_time = "";
		ev->start_date = "";
		ev->dur = 0;
		ev->alarm_time = "";
		ev->alarm_date = "";
		ev->repeat = 0;
		newevent = true;
	}

	view = new BView(this->Bounds(), "newCalEvView", B_FOLLOW_ALL_SIDES,0);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(view);

	BRect r;
	BString tmp;
	BFont font(be_plain_font);

	r = view->Bounds();
	r.InsetBy(20,20);
	r.bottom = r.top + 20;
	tmp = _("Event title");
	title = new BTextControl(r, "calEventTitle", tmp.String(), NULL, new BMessage(TCTITLE));
	title->SetDivider(font.StringWidth(tmp.String())+10);
	view->AddChild(title);

	r.top = r.bottom + 10;
	r.bottom = r.top + 100;
	sBox = new BBox(r, "calSBox");
	sBox->SetLabel(_("Start"));
	view->AddChild(sBox);

	float maxw;
	// sbox contents
	{
	BRect s = sBox->Bounds();
	s.InsetBy(10,15);
	s.bottom = s.top + 20;
	sDate = new BTextControl(s, "calSDate", _("Date"), NULL, new BMessage(TCSDATE));
	s.OffsetBy(0,30);
	timed = new BCheckBox(s, "calTimed", _("Timed"), new BMessage(CBTIMED));
	s.OffsetBy(0,30);
	sTime = new BTextControl(s, "calSTime", _("Time"), NULL, new BMessage(TCSTIME));
	maxw = max(font.StringWidth(_("Date")), font.StringWidth(_("Time")));
	maxw = maxw + 5;
	sDate->SetDivider(maxw);
	sTime->SetDivider(maxw);
	sBox->AddChild(sDate);
	sBox->AddChild(timed);
	sBox->AddChild(sTime);
	}

	r.top = r.bottom + 10;
	r.bottom = r.top + 100;
	aBox = new BBox(r, "calABox");
	aBox->SetLabel(_("Alarm"));
	view->AddChild(aBox);
	
	// abox contents
	{
	BRect s = aBox->Bounds();
	s.InsetBy(10,15);
	s.bottom = s.top + 20;
	alarmed = new BCheckBox(s, "calAlarmed", _("Alarm enabled"), new BMessage(CBALARMED));
	s.OffsetBy(0,30);
	aDate = new BTextControl(s, "calADate", _("Date"), NULL, new BMessage(TCADATE));
	s.OffsetBy(0,30);
	aTime = new BTextControl(s, "calATime", _("Time"), NULL, new BMessage(TCATIME));
	aDate->SetDivider(maxw);
	aTime->SetDivider(maxw);
	aBox->AddChild(alarmed);
	aBox->AddChild(aDate);
	aBox->AddChild(aTime);
	}

	r.top = r.bottom + 10;
	r.bottom = r.top + 20;
	r.right = r.right - 80;
	tmp = _("Duration");
	duration = new BTextControl(r, "calDuration", tmp.String(), NULL, new BMessage(TCDUR));
	duration->SetDivider(font.StringWidth(tmp.String())+10);
	view->AddChild(duration);

	// menu units for duration

	// menu with repeats

	r = view->Bounds();
	r.InsetBy(20,20);
	r.top = r.bottom - font.Size()*2;
	tmp = _("Save");
	r.left = r.right - (font.StringWidth(tmp.String())+30);	
	view->AddChild(new BButton(r, "butSave", tmp.String(), new BMessage(BUT_SAVE), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM));

	SetData();

	Show();
}

dialNewEvent::~dialNewEvent() {
	// delete ev if it was created here...
	if (newevent)
		delete ev;
}

void dialNewEvent::SetData(void) {
	BString tmp;

	title->SetText(ev->title.String());
	sDate->SetText(ev->start_date.String());
	sTime->SetText(ev->start_time.String());
	sTime->SetEnabled(ev->timed);
	timed->SetValue(ev->timed ? B_CONTROL_ON : B_CONTROL_OFF);
	aDate->SetText(ev->alarm_date.String());
	aTime->SetText(ev->alarm_time.String());
	alarmed->SetValue(ev->alarm);
	aDate->SetEnabled(ev->alarm);
	aTime->SetEnabled(ev->alarm);

	tmp = ""; tmp << ev->dur;
	// duration/durmenu - set like in the list
	duration->SetText(tmp.String());
	// menu repeat
}

void dialNewEvent::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case CBTIMED:
			sTime->SetEnabled(timed->Value() == B_CONTROL_ON);
			break;
		case CBALARMED:
			{ 	bool b = alarmed->Value() == B_CONTROL_ON;
				aDate->SetEnabled(b);
				aTime->SetEnabled(b);
				break;
			}			
	}
}
