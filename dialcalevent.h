
#ifndef DIAL_NEWEVENT
#define DIAL_NEWEVENT

#include <Window.h>

class BBox;
class BButton;
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
		int GetData(void);
		int durToUnitValue(int duration);
		int durToUnit(int duration);
		int durToMinutes(int duration, int unit);

		GSM *gsm;
		struct calEvent *ev;
		bool newevent;
		int curDurationUnit;
		int curRepeat;

		BView *view;
		BPopUpMenu *durUnit, *repeat;
		BMenuItem *units[4], *reps[6];
		BTextControl *title, *sDate, *sTime, *aDate, *aTime, *duration;
		BCheckBox *timed, *alarmed;
		BBox *sBox, *aBox;
		BButton *aDateBut;
};

#endif
