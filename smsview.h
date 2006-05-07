
#ifndef _SMSVIEW_H
#define _SMSVIEW_H

#include "gsm.h"
#include "mobileview.h"

class BButton;
class BStatusBar;
class BTextView;
class ColumnListView;

enum { SMS_ALLBOXEN = 0, SMS_INBOX = 1, SMS_OUTBOX = 2 };

class smsView : public mobileView {
	public:
		smsView(BRect r);
		~smsView() { };
		void Show(void);
		void MessageReceived(BMessage *Message);

	private:
		void clearList(void);
		void fillList(void);

		ColumnListView *list;
		BStatusBar *progress;
		BButton *refresh, *del, *newsms;
};

class smsListItem : public CLVEasyItem {
	public:
		smsListItem(struct memSlotSMS *slot) : CLVEasyItem(0, false, false, 20.0) {
			fSName = slot->sname.String();
			fSlot = slot;
			RefreshData();
		}
		void RefreshData(void) {
			BString tmp;

			SetColumnContent(0,fSlot->name.String());
			tmp = ""; tmp << fSlot->items;
			SetColumnContent(2,tmp.String());
			tmp = "";
			if (fSlot->unread>=0) tmp << fSlot->unread; else tmp = "?";
			SetColumnContent(1,tmp.String());
		}
		const char *SName(void) { return fSName.String(); };
		struct memSlotSMS *Slot(void) { return fSlot; };
	private:
		BString fSName;
		struct memSlotSMS *fSlot;
};

#endif
