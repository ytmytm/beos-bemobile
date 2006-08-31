
#include <Button.h>
#include <StringView.h>
#include <stdio.h>

#include "ColumnListView.h"
#include "phonelistslotview.h"

phoneListSlotView::phoneListSlotView(BRect r, const char *slot, GSM *g) : phoneListView(r, slot, "phoneListSlotView", g) {
	but_del->Show();
	but_edit->Show();
	but_new->Show();
	editable = true;
}

void phoneListSlotView::fillList(void) {
	struct pbSlot *sl = gsm->getPBSlot(memSlot.String());
	struct pbNum *num;

	clearList();

	int j = sl->pb->CountItems();
	for (int i=0;i<j;i++) {
		num = (struct pbNum*)sl->pb->ItemAt(i);
		list->AddItem(new phoneSlotListItem(num, gsm));
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
