
#include <StringView.h>
#include "globals.h"
#include "smsinboxview.h"

smsInboxView::smsInboxView(BRect r, const char *slot) : smsBoxView(r, slot) {

	caption->SetText(_("SMS Inbox"));

}

smsOutboxView::smsOutboxView(BRect r, const char *slot) : smsBoxView(r, slot) {

	caption->SetText(_("SMS Outbox"));

}

smsDraftboxView::smsDraftboxView(BRect r, const char *slot) : smsBoxView(r, slot) {

	caption->SetText(_("SMS Drafts"));

}
