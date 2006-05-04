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
		void SetCurView(int v = 0);
		//
		mobileView *pageView[8];
		//
		int curView;
		BStringView *viewCaption;
		GSM *gsm;
		ColumnListView *list;
};

class infoItem : public CLVEasyItem {
	public:
		infoItem(int id, const char *text, int level = 0, bool superitem = false, bool expanded = false, float minwidth = 20.0) : CLVEasyItem(
			level, superitem, expanded, minwidth) {

			fId = id;
			SetColumnContent(2,text);
		}
		int Id(void) { return fId; };
	private:
		int fId;
};

#endif
