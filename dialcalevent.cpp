//
// TODO:
// - alarm if set, must be earlier than event!
// - duration units in menu (decoded)
// - repeat units in menu (decoded)
// - encode/decode date (present to user in YYYY/MM/DD format only)
//		- only in GSM driver; we use YYYY[-/]MM[-/]DD here only
//		- THE SAME APPLIES FOR BDAY dates! (note pbedit/export)
// - input validation (time, date, duration(number))
//		- XXX date validation; pass to calendardialog today if string invalid/empty
// - event w/o time must last exactly 1 day
// -- NOTE
// when adding new event, there is no hint as to date/time format!
//		- paste today

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
#include "dialcalendar.h"
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
const uint32 MENU_DUR	= 'CV09';
const uint32 MENU_REPEAT	= 'CV10';
const uint32 BCSDATE	= 'CV11';
const uint32 BCADATE	= 'CV12';

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
	BMessage *msg;

	// sbox contents
	{
	BRect s = sBox->Bounds();
	s.InsetBy(10,15);
	s.bottom = s.top + 20;
	BRect s2(s); s2.right -= 30; 
	sDate = new BTextControl(s2, "calSDate", _("Date"), NULL, new BMessage(TCSDATE));
	s2.left = s2.right+4; s2.right = s.right;
	msg = new BMessage(BCSDATE);
	msg->AddPointer("_datefield", sDate);
	sBox->AddChild(new BButton(s2, "calBDate", "+", msg));
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
	BRect s2(s); s2.right -= 30; 
	aDate = new BTextControl(s2, "calADate", _("Date"), NULL, new BMessage(TCADATE));
	s2.left = s2.right+4; s2.right = s.right;
	msg = new BMessage(BCADATE);
	msg->AddPointer("_datefield", aDate);
	aBox->AddChild(aDateBut = new BButton(s2, "calADate", "+", msg));
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

	r.left = r.right+5;
	r.right = r.right+80;
	// menu units for duration
	durUnit = new BPopUpMenu("");
	msg = new BMessage(MENU_DUR); msg->AddInt32("unit",0);
	durUnit->AddItem(units[0] = new BMenuItem(_("min"), msg));
	msg = new BMessage(MENU_DUR); msg->AddInt32("unit",1);
	durUnit->AddItem(units[1] = new BMenuItem(_("h"), msg));
	msg = new BMessage(MENU_DUR); msg->AddInt32("unit",2);
	durUnit->AddItem(units[2] = new BMenuItem(_("d"), msg));
	msg = new BMessage(MENU_DUR); msg->AddInt32("unit",3);
	durUnit->AddItem(units[3] = new BMenuItem(_("w"), msg));
	view->AddChild(new BMenuField(r,"durMenuf",NULL,durUnit));
	// menu with repeats
	r.OffsetBy(0,30); r.left = view->Bounds().left+20; r.right = view->Bounds().right-20;
	repeat = new BPopUpMenu("");
// XXX warning - the order of items must be the same as CAL_* in gsm.h
	msg = new BMessage(MENU_REPEAT); msg->AddInt32("repeat",GSM::CAL_NONE);
	repeat->AddItem(reps[0] = new BMenuItem(_("none"), msg));
	msg = new BMessage(MENU_REPEAT); msg->AddInt32("repeat",GSM::CAL_DAILY);
	repeat->AddItem(reps[1] = new BMenuItem(_("daily"), msg));
	msg = new BMessage(MENU_REPEAT); msg->AddInt32("repeat",GSM::CAL_WEEKLY);
	repeat->AddItem(reps[2] = new BMenuItem(_("weekly"), msg));
	msg = new BMessage(MENU_REPEAT); msg->AddInt32("repeat",GSM::CAL_MONTH_ON_DATE);
	repeat->AddItem(reps[3] = new BMenuItem(_("monthly on date"), msg));
	msg = new BMessage(MENU_REPEAT); msg->AddInt32("repeat",GSM::CAL_MONTH_ON_DAY);
	repeat->AddItem(reps[4] = new BMenuItem(_("monthly on day"), msg));
	msg = new BMessage(MENU_REPEAT); msg->AddInt32("repeat",GSM::CAL_YEARLY);
	repeat->AddItem(reps[5] = new BMenuItem(_("yearly"), msg));
	tmp = _("Repeat");
	BMenuField *mf = new BMenuField(r,"repMenuf",tmp.String(),repeat);
	mf->SetDivider(font.StringWidth(tmp.String())+5);
	view->AddChild(mf);

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
	if (ev->timed)
		sTime->SetText(ev->start_time.String());
	else
		sTime->SetText("");
	sTime->SetEnabled(ev->timed);
	timed->SetValue(ev->timed ? B_CONTROL_ON : B_CONTROL_OFF);
	if (ev->alarm) {
		aDate->SetText(ev->alarm_date.String());
		aTime->SetText(ev->alarm_time.String());
	} else {
		aDate->SetText("");
		aTime->SetText("");
	}
	alarmed->SetValue(ev->alarm);
	aDate->SetEnabled(ev->alarm);
	aTime->SetEnabled(ev->alarm);
	aDateBut->SetEnabled(ev->alarm);

	tmp = ""; tmp << durToUnitValue(ev->dur);
	duration->SetText(tmp.String());
	curDurationUnit = durToUnit(ev->dur);
	units[curDurationUnit]->SetMarked(true);

	reps[ev->repeat]->SetMarked(true);
	curRepeat = ev->repeat;
}

// XXX validate user input before calling this!
int dialNewEvent::GetData(void) {
	// fetch data from widgets into calEvent struct
	ev->title = title->Text();
	ev->timed = (timed->Value() == B_CONTROL_ON);
	ev->alarm = (alarmed->Value() == B_CONTROL_ON);
	ev->start_date = sDate->Text();	// XXX encode!
	if (ev->timed)
		ev->start_time = sTime->Text();
	else
		ev->start_time = "00:00";
	if (ev->alarm) {
		ev->alarm_date = aDate->Text();	// XXX encode!
		ev->alarm_time = aTime->Text();
	} else {
		ev->alarm_date = "00-00-2000";
		ev->alarm_time = "00:00";
	}
	ev->dur = durToMinutes(toint(duration->Text()), curDurationUnit);
	ev->repeat = curRepeat;

	return 0;	//XXX if format/data invalid - return error
}

int dialNewEvent::durToUnitValue(int duration) {
	if (duration>=10080)
		return duration / 10080;
	else if (duration>=1440)
		return duration / 1440;
	else if (duration>=60)
		return duration / 60;
	else
		return duration;
}

int dialNewEvent::durToUnit(int duration) {
	if (duration>=10080)		// week
		return 3;
	else if (duration>=1440)	// day
		return 2;
	else if (duration>=60)		// hour
		return 1;
	else
		return 0;				// minute
}

int dialNewEvent::durToMinutes(int duration, int unit) {
	int min = duration;
	switch (unit) {
		case 3:
			min *= 10080;
			break;
		case 2:
			min *= 1440;
			break;
		case 1:
			min *= 60;
			break;
		case 0:
			break;
	}
	return min;
}

void dialNewEvent::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case BCSDATE:
		case BCADATE:
			{	void *ptr;
				BTextControl *dateField;
				BString curDate;
				if (Message->FindPointer("_datefield", &ptr) == B_OK) {
					dateField = static_cast<BTextControl*>(ptr);
//	XXX				curDate = validateDate(dateField->Text());
					curDate = dateField->Text();
					uint32 msgint;
					if (Message->what == BCADATE)
						msgint = TCSDATE;
					else
						msgint = TCADATE;
					dialCalendar *calendarDialog = new dialCalendar(curDate.String(), dateField, msgint, this);
					calendarDialog->Show();
				}
				break;
			}
		case CBTIMED:
			sTime->SetEnabled(timed->Value() == B_CONTROL_ON);
			break;
		case CBALARMED:
			{ 	bool b = alarmed->Value() == B_CONTROL_ON;
				aDate->SetEnabled(b);
				aTime->SetEnabled(b);
				aDateBut->SetEnabled(b);
				break;
			}
		case MENU_DUR:
			{	int32 i;
				if (Message->FindInt32("unit",&i) == B_OK)
					curDurationUnit = i;
				break;
			}
		case MENU_REPEAT:
			{	int32 i;
				if (Message->FindInt32("repeat",&i) == B_OK)
					curRepeat = i;
				break;
			}
		case BUT_SAVE:
			{
				BString tmp;
				//if (validateData())...
				if (GetData()<0) {
					printf("input format error\n");
				} else {
					printf("%s\n",tmp.String());
					if (gsm->storeCalendarEvent(ev) < 0) {
						// XXX error!
						printf("error!\n");
					} else
						Quit();
				}
				break;
			}
	}
}
