
#include <Box.h>
#include <CheckBox.h>
#include <StatusBar.h>
#include <StringView.h>
#include "globals.h"
#include "gsm.h"
#include "statusview.h"

#include <stdio.h>

const uint32 MSG_ACBUT	= 'M001';
const uint32 MSG_BABUT	= 'M002';

statusView::statusView(BRect r) : BView(r, "statusView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW|B_PULSE_NEEDED) {

	// initialize data
	gsm = NULL;

	// initialize widgets
	BStringView *tmp;

	this->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// box1
	this->AddChild(box1 = new BBox(BRect(10,10,420,66),"Box1"));
	box1->SetLabel(_("Phone information"));
	box1->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	// box1 stuff
	{
		BRect tl(10,15,90,30);
		BRect tr(95,15,200,30);
		box1->AddChild(tmp = new BStringView(tl, "manufDesc", _("Manufacturer:"), B_FOLLOW_LEFT|B_FOLLOW_TOP));
		tmp->SetAlignment(B_ALIGN_RIGHT);
		box1->AddChild(manufVal = new BStringView(tr, "manufVal", NULL, B_FOLLOW_LEFT|B_FOLLOW_TOP));
		tl.OffsetBy(0, tl.Height()); tr.OffsetBy(0, tr.Height());
		box1->AddChild(tmp = new BStringView(tl, "modelDesc", _("Model:"), B_FOLLOW_LEFT|B_FOLLOW_TOP));
		tmp->SetAlignment(B_ALIGN_RIGHT);
		box1->AddChild(modelVal = new BStringView(tr, "modelVal", NULL, B_FOLLOW_LEFT|B_FOLLOW_TOP));
	}
	{
		BRect tl(210,15,280,30); BRect tr(285,15,400,30);	
		box1->AddChild(tmp = new BStringView(tl, "gsmDesc", _("GSM version:"), B_FOLLOW_RIGHT|B_FOLLOW_TOP));
		tmp->SetAlignment(B_ALIGN_RIGHT);
		box1->AddChild(gsmVal = new BStringView(tr, "gsmVal", NULL, B_FOLLOW_RIGHT|B_FOLLOW_TOP));
		tl.OffsetBy(0, tl.Height()); tr.OffsetBy(0, tr.Height());
		box1->AddChild(tmp = new BStringView(tl, "imeiDesc", _("IMEI:"), B_FOLLOW_RIGHT|B_FOLLOW_TOP));
		tmp->SetAlignment(B_ALIGN_RIGHT);
		box1->AddChild(imeiVal = new BStringView(tr, "imeiVal", NULL, B_FOLLOW_RIGHT|B_FOLLOW_TOP));
	}	
	// box2
	this->AddChild(box2 = new BBox(BRect(10,76,420,220),"Box2"));
	box2->SetLabel(_("Status"));
	box2->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP_BOTTOM);
	// box2-stuff
	{
		BRect rbar(10,15,400,57);
		box2->AddChild(signalBar = new BStatusBar(rbar, "signalBar"));
		signalBar->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
		rbar.OffsetBy(0, rbar.Height()+10);
		box2->AddChild(batteryBar = new BStatusBar(rbar, "batteryBar"));
		batteryBar->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
		rbar.OffsetBy(0, rbar.Height()+5);
		rbar.right = 280;
		rbar.bottom = rbar.top+20; rbar.right = rbar.left+100;
		box2->AddChild(acPowerBut = new BCheckBox(rbar, "acPowerBut", _("Charger attached"), new BMessage(MSG_ACBUT), B_FOLLOW_LEFT|B_FOLLOW_TOP));
		rbar.OffsetBy(rbar.Width(),0);
		box2->AddChild(batPowerBut = new BCheckBox(rbar, "batPowerBut", _("Battery attached"), new BMessage(MSG_BABUT), B_FOLLOW_RIGHT|B_FOLLOW_TOP));
	}
	// box3
	this->AddChild(box3 = new BBox(BRect(10,225,420,270),"Box3"));
	box3->SetLabel(_("SMS informations"));
	box3->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
	// box3-stuff
	{
		box3->AddChild(smsStatus = new BStringView(BRect(10,15,400,30), "smsStatus", NULL, B_FOLLOW_LEFT|B_FOLLOW_TOP));
	}
	signalBar->SetMaxValue(100);
	batteryBar->SetMaxValue(100);
}

void statusView::RefreshStatus(void) {
	BString tmp;
	signalBar->Reset();
	batteryBar->Reset();
	if (!gsm) return;

	acPowerBut->SetValue(gsm->getACPower() ? B_CONTROL_ON : B_CONTROL_OFF);
	batPowerBut->SetValue(gsm->getBatPower() ? B_CONTROL_ON : B_CONTROL_OFF);
	tmp = ""; tmp << gsm->getSignal(); tmp << "%";
	signalBar->Update(gsm->getSignal(), _("Signal level"), tmp.String());
	tmp = ""; tmp << gsm->getCharge(); tmp << "%";
	batteryBar->Update(gsm->getCharge(), _("Battery level"), tmp.String());
	smsStatus->SetText(gsm->getSMSInfo());
}

void statusView::Pulse(void) {
printf("ping!\n");
	if (!gsm) return;
	gsm->getPhoneStatus();
	RefreshStatus();
}

void statusView::SetDevice(GSM *g) {
	gsm = g;

	if (!gsm) return;
	manufVal->SetText(gsm->getManuf());
	modelVal->SetText(gsm->getModel());
	gsmVal->SetText(gsm->getGSMVer());
	imeiVal->SetText(gsm->getIMEI());
	Pulse();
}

void statusView::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case MSG_ACBUT:
		case MSG_BABUT:
			RefreshStatus();
			break;
		default:
			break;
	}
}
