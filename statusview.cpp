
#include <Bitmap.h>
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <Font.h>
#include <StatusBar.h>
#include <StringView.h>
#include "globals.h"
#include "gsm.h"
#include "statusview.h"

#include <stdio.h>

const uint32 MSG_ACBUT	= 'M001';
const uint32 MSG_BABUT	= 'M002';
const uint32 BUT_SETDATETIME = 'M003';

statusView::statusView(BRect r) : mobileView(r, "statusView") {

	// bitmap stuff
	batOk = getIconFromResources("Img:Battery");
	batCaution = getIconFromResources("Img:BatteryCaution");

	// initialize widgets
	BStringView *tmp;
	BRect r;
	//
	caption->SetText(_("Phone status summary"));
	BFont font(be_plain_font);
	float lineH = font.Size()*1.5;
	// box1
	r = this->MyBounds();
	r.InsetBy(10,10);
	r.bottom = r.top + 3*lineH+2*lineH;
	this->AddChild(box1 = new BBox(r,"statBox1"));
	box1->SetLabel(_("Phone information"));
	box1->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	// box1 stuff
	{
		float wide = font.StringWidth(_("Manufacturer:"));
		BRect tl = box1->Bounds(); tl.InsetBy(10,15);
		tl.right = tl.left+wide+10;
		tl.bottom = tl.top+lineH;
		BRect tr = tl; tr.OffsetBy(tl.Width()+10, 0);
		tr.right = box1->Bounds().Width()/2-10;
		box1->AddChild(tmp = new BStringView(tl, "manufDesc", _("Manufacturer:"), B_FOLLOW_LEFT|B_FOLLOW_TOP));
		tmp->SetAlignment(B_ALIGN_RIGHT);
		box1->AddChild(manufVal = new BStringView(tr, "manufVal", NULL, B_FOLLOW_LEFT|B_FOLLOW_TOP));
		tl.OffsetBy(0, tl.Height()); tr.OffsetBy(0, tr.Height());
		box1->AddChild(tmp = new BStringView(tl, "modelDesc", _("Model:"), B_FOLLOW_LEFT|B_FOLLOW_TOP));
		tmp->SetAlignment(B_ALIGN_RIGHT);
		box1->AddChild(modelVal = new BStringView(tr, "modelVal", NULL, B_FOLLOW_LEFT|B_FOLLOW_TOP));
		tl.OffsetBy(0, tl.Height()); tr.OffsetBy(0, tr.Height());
		box1->AddChild(tmp = new BStringView(tl, "softDesc", _("Software:"),B_FOLLOW_LEFT|B_FOLLOW_TOP));
		tmp->SetAlignment(B_ALIGN_RIGHT);
		box1->AddChild(softVal = new BStringView(tr, "softVal", NULL, B_FOLLOW_LEFT|B_FOLLOW_TOP));
	}
	{
		float wide = font.StringWidth(_("GSM version:"));
		BRect tl = box1->Bounds(); tl.InsetBy(10,15); tl.right = tl.left + tl.Width()/2;
		tl.OffsetBy(tl.Width(),0);
		tl.right = tl.left+wide+10;
		tl.bottom = tl.top+lineH;
		BRect tr = tl; tr.OffsetBy(tl.Width()+10, 0);
		tr.right = box1->Bounds().Width()-10;
		box1->AddChild(tmp = new BStringView(tl, "gsmDesc", _("GSM version:"), B_FOLLOW_RIGHT|B_FOLLOW_TOP));
		tmp->SetAlignment(B_ALIGN_RIGHT);
		box1->AddChild(gsmVal = new BStringView(tr, "gsmVal", NULL, B_FOLLOW_RIGHT|B_FOLLOW_TOP));
		tl.OffsetBy(0, tl.Height()); tr.OffsetBy(0, tr.Height());
		box1->AddChild(tmp = new BStringView(tl, "imeiDesc", _("IMEI:"), B_FOLLOW_RIGHT|B_FOLLOW_TOP));
		tmp->SetAlignment(B_ALIGN_RIGHT);
		box1->AddChild(imeiVal = new BStringView(tr, "imeiVal", NULL, B_FOLLOW_RIGHT|B_FOLLOW_TOP));
		tl.OffsetBy(0, tl.Height()); tr.OffsetBy(0, tr.Height());
		box1->AddChild(tmp = new BStringView(tl, "imsiDesc", _("IMSI:"), B_FOLLOW_RIGHT|B_FOLLOW_TOP));
		tmp->SetAlignment(B_ALIGN_RIGHT);
		box1->AddChild(imsiVal = new BStringView(tr, "imsiVal", NULL, B_FOLLOW_RIGHT|B_FOLLOW_TOP));
	}	
	// box2
	r.OffsetBy(0,r.Height()+15);
	r.bottom = r.top + 144;
	this->AddChild(box2 = new BBox(r,"statBox2"));
	box2->SetLabel(_("Status"));
	box2->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	// box2-stuff
	{
		BRect rbar = box2->Bounds(); rbar.InsetBy(10,15);
		rbar.bottom = rbar.top + 42;
		box2->AddChild(signalBar = new BStatusBar(rbar, "signalBar"));
		signalBar->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
		rbar.OffsetBy(0, rbar.Height()+10);
		box2->AddChild(batteryBar = new BStatusBar(rbar, "batteryBar"));
		batteryBar->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
		rbar.OffsetBy(0, rbar.Height()+5);
		float mwid = font.StringWidth(_("Charger attached"));
		rbar.right = rbar.left + mwid + 20;
		rbar.bottom = rbar.top+20; rbar.right = rbar.left+100;
		box2->AddChild(acPowerBut = new BCheckBox(rbar, "acPowerBut", _("Charger attached"), new BMessage(MSG_ACBUT), B_FOLLOW_LEFT|B_FOLLOW_TOP));
		rbar.OffsetBy(rbar.Width(),0);
		box2->AddChild(batPowerBut = new BCheckBox(rbar, "batPowerBut", _("Battery attached"), new BMessage(MSG_BABUT), B_FOLLOW_LEFT|B_FOLLOW_TOP));
	}
	// box3
	r.OffsetBy(0,r.Height()+15);
	r.bottom = r.top + 45;
	this->AddChild(box3 = new BBox(r,"statBox3"));
	box3->SetLabel(_("SMS informations"));
	box3->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	// box3-stuff
	{
		box3->AddChild(smsStatus = new BStringView(BRect(10,15,400,30), "smsStatus", NULL, B_FOLLOW_LEFT|B_FOLLOW_TOP));
	}
	// other
	r.OffsetBy(0,r.Height()+15);
	BRect s = r; s.bottom = s.top + font.Size() + 5; s.left = s.right - 200;
	this->AddChild(dateTime = new BStringView(s, "dateTime", NULL, B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP));
	dateTime->SetAlignment(B_ALIGN_RIGHT);
	//
	batIconPos = BPoint(r.left,r.top);
	//
	signalBar->SetMaxValue(100);
	batteryBar->SetMaxValue(100);
	//
	r = this->MyBounds();
	r.InsetBy(20,20);
	s = r; s.top = s.bottom - font.Size()*2; s.right = s.left + font.StringWidth("MMMMMMMMMM")+40;
	this->AddChild(new BButton(s, "butSetDateTime", _("Set date and time"), new BMessage(BUT_SETDATETIME), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));
}

statusView::~statusView() {
	delete batOk;
	delete batCaution;
}

void statusView::Draw(BRect updateRect) {
	static BBitmap *lastBattery = batOk;
	BBitmap *battery;

	BRect s = lastBattery->Bounds();
	BRect r(batIconPos.x,batIconPos.y,batIconPos.x+s.Width(),batIconPos.y+s.Height());

	if (r.Intersects(updateRect)) {
		if (gsm->getCharge() < 10)
			battery = batCaution;
		else
			battery = batOk;
		if (battery != lastBattery) {
			lastBattery = battery;
			Invalidate(r);
		}
		SetDrawingMode(B_OP_OVER);
		DrawBitmap(lastBattery, batIconPos);
	}
}

void statusView::RefreshStatus(void) {
	BString tmp;
	if ((this->Window() == NULL) || (this->IsHidden()))
		return;

	signalBar->Reset();
	batteryBar->Reset();
	if (!gsm)
		return;

	acPowerBut->SetValue(gsm->getACPower() ? B_CONTROL_ON : B_CONTROL_OFF);
	batPowerBut->SetValue(gsm->getBatPower() ? B_CONTROL_ON : B_CONTROL_OFF);
	tmp = ""; tmp << gsm->getSignal(); tmp << "%";
	signalBar->Update(gsm->getSignal(), _("Signal level"), tmp.String());
	tmp = ""; tmp << gsm->getCharge(); tmp << "%";
	batteryBar->Update(gsm->getCharge(), _("Battery level"), tmp.String());
	smsStatus->SetText(gsm->getSMSInfo());
	dateTime->SetText(gsm->getDateTime());
}

void statusView::Pulse(void) {
	if ((this->Window() == NULL) || (this->IsHidden()))
		return;

	if (!gsm)
		return;

	if (gsm->statusUpdateEnabled()) {
		gsm->getPhoneStatus();
		RefreshStatus();
	}
}

void statusView::SetDevice(GSM *g) {
	gsm = g;
	if (!gsm)
		return;
	manufVal->SetText(gsm->getManuf());
	modelVal->SetText(gsm->getModel());
	gsmVal->SetText(gsm->getGSMVer());
	imeiVal->SetText(gsm->getIMEI());
	softVal->SetText(gsm->getSoftwareVer());
	imsiVal->SetText(gsm->getIMSI());
	Pulse();
}

void statusView::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case BUT_SETDATETIME:
			gsm->setDateTime();
			break;
		case MSG_ACBUT:
		case MSG_BABUT:
			RefreshStatus();
			break;
		default:
			mobileView::MessageReceived(Message);
			break;
	}
}
