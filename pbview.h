
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
		void RefreshData(void);
		struct pbSlot *Slot(void) { return fSlot; }
	private:
		struct pbSlot *fSlot;
};

#endif
