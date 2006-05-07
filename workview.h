#ifndef _WORKVIEW_H
#define _WORKVIEW_H

#include <View.h>
#include "CLVEasyItem.h"

class BStringView;
class ColumnListView;
class GSM;
class mobileView;

class workView : public BView {
	public:
		workView(BRect r);
		~workView() { };
		void MessageReceived(BMessage *Message);
		//
		void SetDevice(GSM *g);
		void RefreshStatus(void);
		//
	private:
bool hasSMSSlot(const char *slot);
		void SetCurView(int v = 0);
		//
		BList *pageView;
		//
		int curView;
		BStringView *viewCaption;
		GSM *gsm;
		ColumnListView *list;
		// indices of list items
		int VV_INBOX, VV_OUTBOX, VV_DRAFT;
};

class infoItem : public CLVEasyItem {
	public:
		infoItem(int id, const char *text, int level = 0, bool superitem = false, bool expanded = false, float minwidth = 20.0) : CLVEasyItem(
			level, superitem, expanded, minwidth) {

			fId = id;
			fSlot = NULL;
			SetColumnContent(2,text);
		}
		int Id(void) { return fId; };
		const char *Slot(void) { return fSlot; };
		void SetSlot(const char *slot) { fSlot = slot; };
	private:
		int fId;
		const char *fSlot;
};

#endif
