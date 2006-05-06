
#ifndef _SMSINBOXVIEW_H
#define _SMSINBOXVIEW_H

#include "gsm.h"
#include "mobileview.h"

class BButton;
class BStatusBar;
class BTextView;
class ColumnListView;

class smsInboxView : public mobileView {
	public:
		smsInboxView(BRect r, const char *slot);
		~smsInboxView() { };
		void Show(void);
		void MessageReceived(BMessage *Message);

	private:
		void updatePreview(struct SMS *sms);
		void clearList(void);
		void fillList(void);

		BString memSlot;

		ColumnListView *list;
		BTextView *prv;
		BStatusBar *progress;
		BButton *refresh, *del, *newsms;
};

class smsInboxListItem : public CLVEasyItem {
	public:
		smsInboxListItem(struct SMS *sms) : CLVEasyItem(0, false, false, 20.0) {
			BString tmp;

			fSMS = sms;
			fId = sms->id;
			SetColumnContent(1,sms->number.String());
			switch (sms->type) {
				case GSM::REC_UNREAD:
					SetColumnContent(0,"!");
					break;
				default:
					break;
			}
			SetColumnContent(2,sms->date.String());
			tmp = sms->msg.String();
			tmp.ReplaceSet("\r\n\t"," ");
			SetColumnContent(3,tmp.String());
		}
		int Id(void) { return fId; };
		struct SMS *Msg(void) { return fSMS; };
	private:
		int fId;
		struct SMS *fSMS;
};

#endif
