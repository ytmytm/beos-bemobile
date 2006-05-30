
#include <StringView.h>
#include <stdio.h>

#include "ColumnListView.h"
#include "callregslotview.h"

callRegSlotView::callRegSlotView(BRect r, const char *slot) : phoneListView(r, slot, "callRegSlotView") {

}

const char *callRegSlotView::stripLeadNum(const char *num) {
	static BString tmp;
	tmp = num;
	tmp.RemoveSet("+");
	while (tmp[0] == '0')
		tmp.Remove(0,1);
	return tmp.String();
}

void callRegSlotView::fillList(void) {
	struct pbSlot *sl = gsm->getPBSlot(memSlot.String());
	struct pbNum *num;
	BString crnum;

	clearList();

	int j = sl->pb->CountItems();
	for (int i=0;i<j;i++) {
		num = (struct pbNum*)sl->pb->ItemAt(i);
		// a tutaj magia i wyszukiwanie
		crnum = stripLeadNum(num->number.String());
		// search in all Phonebook slots
		int k, l = gsm->listMemSlotPB->CountItems();
		struct pbSlot *pb;
		for (k=0;k<l;k++) {
			pb = (struct pbSlot*)gsm->listMemSlotPB->ItemAt(k);
			if (!pb->callreg) {
				// search in all items
				int m, n = pb->pb->CountItems();
				struct pbNum *pn;
				for (m=0;m<n;m++) {
					pn = (struct pbNum*)pb->pb->ItemAt(m);
//					if (strippedNumbersMatch(crnum.String(),pn->number.String())) {
					if (crnum.Compare(stripLeadNum(pn->number.String())) == 0) {
						printf("%i:[%s]:[%s]\n",pn->id,pn->number.String(),crnum.String());
						list->AddItem(new phoneSlotListItem(pn));
						goto skip;
					}
				}
			}
		}
		list->AddItem(new phoneSlotListItem(num));
skip:	;
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
