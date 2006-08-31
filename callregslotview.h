
#ifndef _CALLREGSLOTVIEW_H
#define _CALLREGSLOTVIEW_H

#include "phonelist.h"

class callRegSlotView : public phoneListView {
	public:
		callRegSlotView(BRect r, const char *slot, GSM *g);
		~callRegSlotView() { };
//		void MessageReceived(BMessage *Message);
		void SetDevice(GSM *g);

	protected:
		void fillList(void);
};

#endif
