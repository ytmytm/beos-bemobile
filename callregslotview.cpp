
#include <StringView.h>
#include <stdio.h>

#include "ColumnListView.h"
#include "callregslotview.h"

callRegSlotView::callRegSlotView(BRect r, const char *slot) : phoneListView(r, slot, "callRegSlotView") {

}

void callRegSlotView::fillList(void) {
	struct pbSlot *sl = gsm->getPBSlot(memSlot.String());
	struct pbNum *num;

	clearList();

	int j = sl->pb->CountItems();
	for (int i=0;i<j;i++) {
		num = (struct pbNum*)sl->pb->ItemAt(i);
		list->AddItem(new phoneSlotListItem(gsm->matchNumFromPB(num)));
	}
}

void callRegSlotView::SetDevice(GSM *g) {
	phoneListView::SetDevice(g);
	BString cap = caption->Text();
	if (cap.Length() == 0) {
		cap = _("Register: "); cap += (gsm->getPBSlot(memSlot.String()))->name.String();
		caption->SetText(cap.String());
	}
}
