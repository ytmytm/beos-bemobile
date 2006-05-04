
#ifndef _SMSVIEW_H
#define _SMSVIEW_H

#include "gsm.h"
#include "mobileview.h"

class BButton;
class BStatusBar;
class BTextView;
class ColumnListView;

enum { SMS_ALLBOXEN = 0, SMS_INBOX = 1, SMS_OUTBOX = 2 };

class smsView : public mobileView {
	public:
		smsView(BRect r);
		~smsView() { };
		void Show(void);
		void MessageReceived(BMessage *Message);

	private:
		void updatePreview(struct SMS *sms);
		void clearList(void);
		void fillList(int mode = SMS_ALLBOXEN);

		ColumnListView *list;
		BTextView *prv;
		BStatusBar *progress;
		BButton *refresh, *del, *newsms;
};

class smsListItem : public CLVEasyItem {
	public:
		smsListItem(struct SMS *sms) : CLVEasyItem(0, false, false, 20.0) {
			BString tmp;

			fSMS = sms;
			fId = sms->id;
			switch (sms->type) {
				case GSM::STO_SENT:
				case GSM::STO_UNSENT:
					SetColumnContent(1,sms->number.String());
					break;
				case GSM::REC_READ:
				case GSM::REC_UNREAD:
				default:
					SetColumnContent(0,sms->number.String());
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
