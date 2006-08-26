
#ifndef DIAL_EDITPBOOK_H
#define DIAL_EDITPBOOK_H

#include <Window.h>
#include "gsm.h"

class BCheckBox;
class BPopUpMenu;
class BTextControl;
class BStringView;
class GSM;

class dialEditPB : public BWindow {
	public:
		dialEditPB(const char *sl, GSM *g, struct pbNum *p = NULL);
		virtual void MessageReceived (BMessage *Message);
	private:
		void validateData(void);
		void revertData(void);
		void updateIdText(void);
		void switchToSlot(struct pbSlot *slot);

		bool saveData(void);

		GSM *gsm;
		struct pbNum *num;
		struct pbSlot *slotWrite, *iniSlot;
		int curType;	// current number type
		int iniSlotNum; // number of menu w/ default slot
		int id; // if >0 then store new, otherwise update

		BView *view;
		BPopUpMenu *slotMenu, *typeMenu;
		BMenuItem **mSlots, *mTypes[8];
		BCheckBox *cbPrimary;
		BTextControl *tcNumber, *tcName, *tcAddress1, *tcAddress2;
		BTextControl *tcCity, *tcState, *tcZip, *tcCountry, *tcNick, *tcBday;
		BStringView *idText;
};

#endif