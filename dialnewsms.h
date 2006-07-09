
#ifndef DIAL_NEWSMS
#define DIAL_NEWSMS

#include <Window.h>

class BPopUpMenu;
class BTextControl;
class BStringView;
class BTextView;
class GSM;

class dialNewSMS : public BWindow {
	public:
		dialNewSMS(const char *sl, GSM *g);
		virtual void MessageReceived (BMessage *Message);
	private:
		GSM *gsm;
		struct memSlotSMS *slot;

		BView *view;
		BPopUpMenu *slotMenu, *pbMenu;
		BTextControl *numberText;
		BStringView *countText;
		BTextView *content;
};

#endif
