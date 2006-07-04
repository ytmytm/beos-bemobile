#ifndef _WORKVIEW_H
#define _WORKVIEW_H

#include <Application.h>
#include <Resources.h>
#include <Bitmap.h>
#include <stdio.h>
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
		const char *iconNameForSlot(const char *slot);
		//
		BList *pageView;
		//
		int curView;
		BStringView *viewCaption;
		GSM *gsm;
		ColumnListView *list;
		// indices of list items
		int VV_DIALED, VV_DIALEDSIM, VV_MISSED, VV_RECEIVED;
		int VV_INBOX, VV_OUTBOX, VV_DRAFT;
};

class infoItem : public CLVEasyItem {
	public:
		infoItem(int id, const char *text, int level = 0, const char *icon_resname = NULL, bool superitem = false, bool expanded = false, float minwidth = 20.0) : CLVEasyItem(
			level, superitem, expanded, minwidth) {

			fId = id;
			fSlot = NULL;
			if (icon_resname) {
				BResources *res = be_app->AppResources();
				printf("search for res:%s\n",icon_resname);
				if (res->HasResource('BBMP',icon_resname)) {
					printf("has it\n");
					BBitmap *bmp;
					BMessage msg;
					size_t len;
					char *buf;
					buf = (char *)res->LoadResource('BBMP', icon_resname, &len);
//					printf("loaded,len=%i\n",len);
					msg.Unflatten(buf);
//					printf("unflatten\n");
					bmp = new BBitmap(&msg);
//					printf("new bmp\n");
					SetColumnContent(1,bmp);
//					printf("col set\n");
					delete [] buf;
				}
			}
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
