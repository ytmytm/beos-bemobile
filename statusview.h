#ifndef _STATUSVIEW_H
#define _STATUSVIEW_H

#include <View.h>

class BBox;
class BCheckBox;
class BStatusBar;
class BStringView;
class GSM;

class statusView : public BView {
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
		GSM *gsm;

		BBox *box1, *box2, *box3;
		BStringView *manufVal, *modelVal, *gsmVal, *imeiVal;
		BStringView *smsStatus;
		BStatusBar *signalBar, *batteryBar;
		BCheckBox *acPowerBut, *batPowerBut;
};

#endif
