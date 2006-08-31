
#ifndef _PHONELISTSLOTVIEW_H
#define _PHONELISTSLOTVIEW_H

#include "phonelist.h"

class phoneListSlotView : public phoneListView {
	public:
		phoneListSlotView(BRect r, const char *slot, GSM *g);
		~phoneListSlotView() { };
//		void MessageReceived(BMessage *Message);
		void SetDevice(GSM *g);

	protected:
		void fillList(void);
};

#endif
