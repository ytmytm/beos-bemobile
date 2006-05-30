
#include <StringView.h>
#include <stdio.h>

#include "ColumnListView.h"
#include "phonelistslotview.h"

phoneListSlotView::phoneListSlotView(BRect r, const char *slot) : phoneListView(r, slot, "phoneListSlotView") {

}

void phoneListSlotView::fillList(void) {
	struct pbSlot *sl = gsm->getPBSlot(memSlot.String());
	struct pbNum *num;

	clearList();

	int j = sl->pb->CountItems();
	for (int i=0;i<j;i++) {
		num = (struct pbNum*)sl->pb->ItemAt(i);
		list->AddItem(new phoneSlotListItem(num));
	}
}

void phoneListSlotView::SetDevice(GSM *g) {
	phoneListView::SetDevice(g);
	BString cap = caption->Text();
	if (cap.Length() == 0) {
		cap = _("Phonebook slot: "); cap += (gsm->getPBSlot(memSlot.String()))->name.String();
		caption->SetText(cap.String());
	}
}
