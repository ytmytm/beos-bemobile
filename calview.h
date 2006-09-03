
#ifndef _CALVIEW_H
#define _CALVIEW_H

#include "CLVEasyItem.h"
#include "gsm.h"
#include "mobileview.h"

class BButton;
class BStatusBar;
class ColumnListView;

class calView : public mobileView {
	public:
		calView(BRect r);
		~calView() { };
		void Show(void);
		void MessageReceived(BMessage *Message);
	private:
		void clearList(void);
		void fillList(void);
		void fullListRefresh(void);

		ColumnListView *list;
		BStatusBar *progress;
		BButton *refresh;
};

class calListItem : public CLVEasyItem {
	public:
		calListItem(struct calEvent *event) : CLVEasyItem(0, false, false, 20.0) {
			fEvent = event;
			RefreshData();
		}
		void RefreshData(void) {
			BString tmp;
			tmp = ""; tmp << fEvent->id;
			SetColumnContent(0, tmp.String());
			SetColumnContent(1, fEvent->title.String());
			SetColumnContent(2, fixDate(fEvent->start_date.String(),fEvent->start_time.String()));
			tmp = ""; tmp << fEvent->dur; tmp += _(" min.");
			SetColumnContent(3, tmp.String());
			if (fEvent->alarm)
				SetColumnContent(4, fixDate(fEvent->alarm_date.String(),fEvent->alarm_time.String()));
			else
				SetColumnContent(4, _("(no alarm set)"));
			switch (fEvent->repeat) {
				case GSM::CAL_DAILY:
					tmp = _("daily"); break;
				case GSM::CAL_WEEKLY:
					tmp = _("weekly"); break;
				case GSM::CAL_MONTH_ON_DATE:
					tmp = _("monthly on date"); break;
				case GSM::CAL_MONTH_ON_DAY:
					tmp = _("monthly on day"); break;
				case GSM::CAL_YEARLY:
					tmp = _("yearly"); break;
				case GSM::CAL_NONE:
				default:
					tmp = _("none"); break;
			}
			SetColumnContent(5, tmp.String());
		}
		struct calEvent *Event(void) { return fEvent; };
	private:
		struct calEvent *fEvent;
		// convert to ISO YYYY-MM-DD HH:MM
		const char *fixDate(const char *date, const char *time) {
			static BString dn;
			BString d = date;
			dn = "";
			d.CopyInto(dn,6,4);
			dn += "-"; dn += d[0]; dn+= d[1];
			dn += "-"; dn += d[3]; dn+= d[4];
			dn += " "; dn += time;
			return dn.String();
		};
};

#endif
