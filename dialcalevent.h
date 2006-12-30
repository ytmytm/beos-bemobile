
#ifndef DIAL_NEWEVENT
#define DIAL_NEWEVENT

#include <Window.h>

class BBox;
class BCheckBox;
class BPopUpMenu;
class BTextControl;
class GSM;

class dialNewEvent : public BWindow {
	public:
		dialNewEvent(GSM *g, struct calEvent *event=NULL);
		~dialNewEvent();
		virtual void MessageReceived (BMessage *Message);
	private:
		void SetData(void);

		GSM *gsm;
		struct calEvent *ev;
		bool newevent;

		BView *view;
		BPopUpMenu *durUnit, *repeat;
		BTextControl *title, *sDate, *sTime, *aDate, *aTime, *duration;
		BCheckBox *timed, *alarmed;
		BBox *sBox, *aBox;
};

#endif
