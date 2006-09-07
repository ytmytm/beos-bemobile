
#ifndef _PBBYNAMEVIEW_H
#define _PBBYNAMEVIEW_H

#include "CLVEasyItem.h"
#include "gsm.h"
#include "mobileview.h"

class BButton;
class BList;
class BStatusBar;
class ColumnListView;

class pbByNameView : public mobileView {
	public:
		pbByNameView(BRect r);
		~pbByNameView() { };
		void Show(void);
		void MessageReceived(BMessage *Message);
	private:
		void clearList(void);
		void fillList(void);
		void fullListRefresh(void);

		ColumnListView *list;
		BStatusBar *progress;
		BButton *refresh;

		BList *byNameList;
};

class pbByNameListItem : public CLVEasyItem {
	public:
		pbByNameListItem(struct pbNum *num, int level = 0, bool superitem = false) : CLVEasyItem(
			level, superitem, false, 20.0) {
			fNum = num;
			RefreshData(superitem);
		}
		void RefreshData(bool superitem = false) {
			BString *tmp;
			if (superitem) {
				// insert name
				tmp = ((union pbVal*)fNum->attr->ItemAt(1))->text;
				SetColumnContent(1, tmp->String());
			} else {
				// insert number and other attrs (if present!)
				tmp = ((union pbVal*)fNum->attr->ItemAt(0))->text;
				SetColumnContent(2, tmp->String());
				// set type on column 1
				SetColumnContent(1, fNum->slot.String());
			}
		}
		struct pbNum *Num(void) { return fNum; };
	private:
		struct pbNum *fNum;
};

#endif
