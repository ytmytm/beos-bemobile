
#ifndef _SMSVIEW_H
#define _SMSVIEW_H

#include "CLVEasyItem.h"
#include "gsm.h"
#include "mobileview.h"

class BButton;
class BStatusBar;
class BTextControl;
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
		void fullListRefresh(void);
		void smscRefresh(void);
		void smscSet(void);
		void smsExportSlot(struct memSlotSMS *slot);

		ColumnListView *list;
		BTextControl *smsc;
		BStatusBar *progress;
		BButton *refresh, *setsmsc, *exportslot;
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
			tmp = fSlot->writable ? _("Yes") : _("No");
			SetColumnContent(1,tmp.String());
			tmp = "";
			if (fSlot->unread>=0) tmp << fSlot->unread; else tmp = "?";
			if (fSlot->unread>0) SetBold(true);
			SetColumnContent(2,tmp.String());
			tmp = ""; tmp << fSlot->items;
			SetColumnContent(3,tmp.String());
		}
		const char *SName(void) { return fSName.String(); };
		struct memSlotSMS *Slot(void) { return fSlot; };
	private:
		BString fSName;
		struct memSlotSMS *fSlot;
};

#endif
