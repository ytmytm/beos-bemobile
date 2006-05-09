
#ifndef _CALLREGSLOTVIEW_H
#define _CALLREGSLOTVIEW_H

#include "CLVEasyItem.h"
#include "gsm.h"
#include "mobileview.h"

class BButton;
class BStatusBar;
class ColumnListView;

class callRegSlotView : public mobileView {
	public:
		callRegSlotView(BRect r, const char *slot);
		~callRegSlotView() { };
		void Show(void);
		void MessageReceived(BMessage *Message);
		void SetDevice(GSM *g);

	protected:
		void clearList(void);
		void fillList(void);
		void fullListRefresh(void);

		BString memSlot;

		ColumnListView *list;
		BStatusBar *progress;
		BButton *refresh, *call;
};

class callRegSlotListItem : public CLVEasyItem {
	public:
		callRegSlotListItem(struct pbNum *num) : CLVEasyItem(0, false, false, 20.0) {
			BString tmp;

			fNum = num;
			fId = num->id;
			tmp = ""; tmp << num->id;
			SetColumnContent(0,tmp.String());
			// use type/kind to get icon before number
			SetColumnContent(1,num->number.String());
			SetColumnContent(2,num->name.String());
			switch (num->kind) {
				case GSM::PK_WORK:	tmp = _("Work"); break;
				case GSM::PK_HOME:	tmp = _("Home"); break;
				case GSM::PK_MAIN:	tmp = _("General"); break;
				case GSM::PK_FAX:	tmp = _("Fax"); break;
				case GSM::PK_PAGER:	tmp = _("Pager"); break;
				case GSM::PK_EMAIL:	tmp = _("E-mail"); break;
				case GSM::PK_MAILLIST:	tmp = _("Mailing list");
				case GSM::PK_MOBILE:
				default:
					tmp = _("Mobile");
					break;
			}
			SetColumnContent(3,tmp.String());
			num->primary ? tmp = _("Yes") : tmp = _("No");
			SetColumnContent(4,tmp.String());
		}
		int Id(void) { return fId; };
		struct pbNum *Num(void) { return fNum; };
	private:
		int fId;
		struct pbNum *fNum;
};

#endif
