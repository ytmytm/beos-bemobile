
#ifndef _CALVIEW_H
#define _CALVIEW_H

#include "CLVEasyItem.h"
#include "gsm.h"
#include "mobileview.h"

class BButton;
class BStatusBar;
class ColumnListView;
class dialNewEvent;

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
		BButton *refresh, *newevent, *edit;
		dialNewEvent *dnev;
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
			tmp = fEvent->timed ? fEvent->start_time.String() : "";
			SetColumnContent(2, fixDate(fEvent->start_date.String(),tmp.String()));
			SetColumnContent(3, durationName(fEvent->dur));
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
		// convert to YYYY/MM/DD HH:MM
		const char *fixDate(const char *date, const char *time) {
			static BString dn;
			BString d = date;
			dn = "";
			d.CopyInto(dn,6,4);
			dn += "/"; dn += d[0]; dn+= d[1];
			dn += "/"; dn += d[3]; dn+= d[4];
			dn += " "; dn += time;
			return dn.String();
		};
		// return duration in minutes in human format
		const char *durationName(int minutes) {
			static BString out;
			int weeks = 0, days = 0, hours = 0;
		
			out = "";
			if (minutes >= 10080) {
				weeks = minutes / 10080;
				minutes = minutes % 10080;
				out << weeks; out += _(" w");
			}
			if (minutes >= 1440) {
				days = minutes / 1440;
				minutes = minutes % 1440;
				out << days; out += _(" d");
			}
			if (minutes >= 60) {
				hours = minutes / 60;
				minutes = minutes % 60;
				if (days>0)
					out += " ";
				out << hours; out += _(" h");
			}
			if (minutes>0) {
				if ((hours>0)||(days>0))
					out += " ";
				out << minutes; out += _(" min.");
			}
			return out.String();
		};
};

#endif
