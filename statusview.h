#ifndef _STATUSVIEW_H
#define _STATUSVIEW_H

#include "mobileview.h"

class BBox;
class BCheckBox;
class BStatusBar;
class BStringView;

class statusView : public mobileView {
	public:
		statusView(BRect r);
		~statusView() { };
		void MessageReceived(BMessage *Message);
		void Pulse(void);
		//
		void SetDevice(GSM *g);
		void RefreshStatus(void);
		//
	private:
		BBox *box1, *box2, *box3;
		BStringView *manufVal, *modelVal, *gsmVal, *imeiVal, *softVal, *imsiVal;
		BStringView *smsStatus, *dateTime;
		BStatusBar *signalBar, *batteryBar;
		BCheckBox *acPowerBut, *batPowerBut;
};

#endif
