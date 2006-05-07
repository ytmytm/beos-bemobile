
#ifndef _SMSINBOXVIEW_H
#define _SMSINBOXVIEW_H

#include "smsboxview.h"

class smsInboxView : public smsBoxView {
	public:
		smsInboxView(BRect r, const char *slot);
};

class smsOutboxView : public smsBoxView {
	public:
		smsOutboxView(BRect r, const char *slot);
};

class smsDraftboxView : public smsBoxView {
	public:
		smsDraftboxView(BRect r, const char *slot);
};

#endif
