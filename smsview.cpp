
#include <StringView.h>
#include "globals.h"
#include "smsview.h"

smsView::smsView(BRect r) : mobileView(r, "smsView") {
	caption->SetText(_("SMS summary"));
	// summary of inbox/outbox
	// refresh button
	// new sms facility
}
