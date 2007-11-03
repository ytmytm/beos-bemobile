#ifndef _STATUSVIEW_H
#define _STATUSVIEW_H

#include <Point.h>
#include "mobileview.h"

class BBitmap;
class BBox;
class BCheckBox;
class BStatusBar;
class BStringView;

class statusView : public mobileView {
	public:
		statusView(BRect r);
		~statusView();
		void Draw(BRect updateRect);
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

		BPoint batIconPos;
		BBitmap *batOk, *batCaution;
};

#endif
