
#ifndef _SMSBOXVIEW_H
#define _SMSBOXVIEW_H

#include "CLVEasyItem.h"
#include "gsm.h"
#include "mobileview.h"

class BButton;
class BStatusBar;
class BTextView;
class ColumnListView;

class smsBoxView : public mobileView {
	public:
		smsBoxView(BRect r, const char *slot);
		~smsBoxView() { };
		void Show(void);
		void MessageReceived(BMessage *Message);
		void SetDevice(GSM *g);

	protected:
		void updatePreview(struct SMS *sms);
		void clearList(void);
		void fillList(void);
		void fullListRefresh(void);

		BString memSlot;

		ColumnListView *list;
		BTextView *prv;
		BStatusBar *progress;
		BButton *refresh, *del, *newsms;
};

class smsBoxListItem : public CLVEasyItem {
	public:
		smsBoxListItem(struct SMS *sms) : CLVEasyItem(0, false, false, 20.0) {
			BString tmp;

			fSMS = sms;
			fId = sms->id;
			SetColumnContent(1,sms->number.String());
			switch (sms->type) {
				case GSM::REC_UNREAD:
				case GSM::STO_UNSENT:
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
