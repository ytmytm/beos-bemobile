
#ifndef _PHONELISTVIEW_H
#define _PHONELISTVIEW_H

#include "CLVEasyItem.h"
#include "globals.h"
#include "gsm.h"
#include "mobileview.h"

class BButton;
class BStatusBar;
class ColumnListView;

class phoneListView : public mobileView {
	public:
		phoneListView(BRect r, const char *slot, const char *name, GSM *g);
		~phoneListView() { };
		void Show(void);
		virtual void MessageReceived(BMessage *Message);

	protected:
		virtual void clearList(void);
		virtual void fillList(void);
		void fullListRefresh(void);

		BString memSlot;

		ColumnListView *list;
		BStatusBar *progress;
		BButton *but_refresh, *but_new, *but_edit, *but_del, *but_dial;
		bool editable;
};

class phoneSlotListItem : public CLVEasyItem {
	public:
		phoneSlotListItem(struct pbNum *num, GSM *gsm);
		int Id(void) { return fId; };
		struct pbNum *Num(void) { return fNum; };
	private:
		int fId;
		struct pbNum *fNum;
};

#endif
