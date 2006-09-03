
#ifndef _PBVIEW_H
#define _PBVIEW_H

#include "CLVEasyItem.h"
#include "gsm.h"
#include "mobileview.h"

class BButton;
class BStatusBar;
class ColumnListView;

class pbView : public mobileView {
	public:
		pbView(BRect r);
		~pbView() { };
		void Show(void);
		void MessageReceived(BMessage *Message);
	private:
		void clearList(void);
		void fillList(void);
		void fullListRefresh(void);

		ColumnListView *list;
		BStatusBar *progress;
		BButton *refresh, *pbexport;
};

#include <stdio.h>

class pbListItem : public CLVEasyItem {
	public:
		pbListItem(struct pbSlot *slot) : CLVEasyItem(0, false, false, 20.0) {
			fSlot = slot;
			RefreshData();
		}
		void RefreshData(void) {
			BString tmp;
			char percent[10];
			float u, t;

			SetColumnContent(0, fSlot->name.String());
			tmp = ""; tmp << (fSlot->max - fSlot->min + 1);
			SetColumnContent(2, tmp.String());
			if (fSlot->initialized) {
				tmp = "";
				tmp << fSlot->pb->CountItems();
				SetColumnContent(1, tmp.String());
				u = fSlot->pb->CountItems();
				t = fSlot->max - fSlot->min + 1;
				sprintf(percent, "%2.0f", (u/t)*100);
				tmp = ""; tmp << percent; tmp += "%";
				SetColumnContent(3, tmp.String());
			} else {
				SetColumnContent(1, "?");
				SetColumnContent(3, "?");
			}
		}
		struct pbSlot *Slot(void) { return fSlot; }
		private:
			struct pbSlot *fSlot;
};

#endif
