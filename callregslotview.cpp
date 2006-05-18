
#include <Alert.h>
#include <Box.h>
#include <Button.h>
#include <Font.h>
#include <StatusBar.h>
#include <StringView.h>
#include "ColumnListView.h"
#include "globals.h"
#include "callregslotview.h"

#include <stdio.h>

const uint32	CRLIST_INV		= 'CR00';
const uint32	CRLIST_SEL 		= 'CR01';
const uint32	CRREFRESH		= 'CR02';
const uint32	CRCALL			= 'CR03';

callRegSlotView::callRegSlotView(BRect r, const char *slot) : mobileView(r, "callRegSlotView") {

	memSlot = slot;

	BFont font(be_plain_font);
	float maxw, totalw = 0;
	BRect r = this->MyBounds();
	r.InsetBy(10,15);
	r.right -= B_V_SCROLL_BAR_WIDTH;
	r.bottom -= 100;

	// add column list
	int flags = CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE;
	CLVContainerView *containerView;
	list = new ColumnListView(r, &containerView, NULL, B_FOLLOW_TOP_BOTTOM|B_FOLLOW_LEFT_RIGHT,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST, false, true, true, true,
		B_FANCY_BORDER);
	maxw = font.StringWidth("XXXX"); totalw += maxw;
	list->AddColumn(new CLVColumn("#", maxw, flags));
	maxw = font.StringWidth("+999999999") + 20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Number"), maxw, flags));
	maxw = font.StringWidth("MMMMMMMMMMMMMMMMMMMM") + 20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Name"), maxw, flags));
	maxw = font.StringWidth("MMMMMMMMMMMM") + 20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Type"), maxw, flags));
	list->AddColumn(new CLVColumn(_("Primary"), r.right-B_V_SCROLL_BAR_WIDTH-totalw, flags));
	list->SetSortFunction(CLVEasyItem::CompareItems);
	this->AddChild(containerView);
	list->SetInvocationMessage(new BMessage(CRLIST_INV));
	list->SetSelectionMessage(new BMessage(CRLIST_SEL));

	r = this->MyBounds();
	r.InsetBy(10,15);
	r.top = r.bottom - font.Size()*4 - 30;
	r.bottom = r.top + font.Size()*2;
	this->AddChild(progress = new BStatusBar(r, "crStatusBar"));
	progress->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
	progress->SetMaxValue(100);
	progress->Hide();

	r = this->MyBounds();
	r.InsetBy(20,20);
	BRect s, t;
	s = r; s.top = s.bottom - font.Size()*2; s.right = s.left + font.StringWidth("MMMMMMMMMM")+40;
	this->AddChild(refresh = new BButton(s, "crRefresh", _("Refresh"), new BMessage(CRREFRESH), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));
	t = r; t.top = s.top; t.bottom = s.bottom; t.right = r.right; t.left = t.right - (font.StringWidth("MMMMMMMMMM")+40);
	this->AddChild(call = new BButton(t, "crCall", _("Dial"), new BMessage(CRCALL), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM));
	call->SetEnabled(false);
}

void callRegSlotView::clearList(void) {
	if (list->CountItems()>0) {
		CLVEasyItem *anItem;
		for (int i=0; (anItem=(callRegSlotListItem*)list->ItemAt(i)); i++)
			delete anItem;
		if (!list->IsEmpty())
			list->MakeEmpty();
	}
}

void callRegSlotView::fillList(void) {
	struct pbSlot *sl = gsm->getPBSlot(memSlot.String());
	struct pbNum *num;

	clearList();

	int j = sl->pb->CountItems();
	for (int i=0;i<j;i++) {
		num = (struct pbNum*)sl->pb->ItemAt(i);
		list->AddItem(new callRegSlotListItem(num));
	}
}

void callRegSlotView::fullListRefresh(void) {
	progress->Reset();
	progress->Show();

	progress->Update(0, _("Reading phonebook..."));

	gsm->getPBList(memSlot.String());

	progress->Hide();
}

void callRegSlotView::Show(void) {
	BView::Show();
	BView::Flush();
	BView::Sync();
	if (list->CountItems()==0) {
		if (gsm->getPBSlot(memSlot.String())->pb->CountItems() == 0)
			fullListRefresh();
	}
	fillList();
}

void callRegSlotView::SetDevice(GSM *g) {
	mobileView::SetDevice(g);
	BString cap = caption->Text();
	if (cap.Length() == 0) {
		cap = _("Register: "); cap += (gsm->getPBSlot(memSlot.String()))->name.String();
		caption->SetText(cap.String());
	}
}

void callRegSlotView::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case CRREFRESH:
			fullListRefresh();
			fillList();
			break;
		case CRCALL:
			// XXX implement!
			break;
		case CRLIST_INV:
		case CRLIST_SEL:
//			{	int i = list->CurrentSelection(0);
//				if (i>=0) {
//					struct SMS *sms = ((smsBoxListItem*)list->ItemAt(list->CurrentSelection(0)))->Msg();
//					if (sms)
//						updatePreview(sms);
//
//				} else {
//					prv->SetText("");
//				}
//				break;
//			}
		default:
			break;
	}
}
