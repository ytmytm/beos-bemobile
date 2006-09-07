
#include <Alert.h>
#include <Button.h>
#include <Font.h>
#include <StatusBar.h>
#include <StringView.h>
#include "ColumnListView.h"
#include "globals.h"
#include "pbbynameview.h"

#include <stdio.h>

const uint32	PBNLIST_INV	= 'PBN0';
const uint32	PBNLIST_SEL	= 'PBN1';
const uint32	PBNREFRESH	= 'PBN2';

pbByNameView::pbByNameView(BRect r) : mobileView(r, "pbByNameView") {
	caption->SetText(_("Phonebook by name"));

	BFont font(be_plain_font);
	float maxw, totalw = 0;
	BRect r = this->MyBounds();
	r.InsetBy(10,15);
	r.right -= B_V_SCROLL_BAR_WIDTH;
	r.bottom -= 100;

	// add column list
	CLVContainerView *containerView;
	list = new ColumnListView(r, &containerView, NULL, B_FOLLOW_TOP_BOTTOM|B_FOLLOW_LEFT,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST, true, true, true, true,
		B_FANCY_BORDER);
	totalw = 20;
	list->AddColumn(new CLVColumn(NULL,20,CLV_EXPANDER|CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE));
	maxw = font.StringWidth(_("M"))*17+20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Name"), maxw, CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE|CLV_PUSH_PASS|CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE));
//	maxw = font.StringWidth(_("9"))*12+20; totalw += maxw;
//	list->AddColumn(new CLVColumn(_("Number"), maxw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->AddColumn(new CLVColumn(_("Number"), r.right-B_V_SCROLL_BAR_WIDTH-4-totalw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->SetSortFunction(CLVEasyItem::CompareItems);
	this->AddChild(containerView);
	list->SetInvocationMessage(new BMessage(PBNLIST_INV));
//	list->SetSelectionMessage(new BMessage(CALIST_SEL));

	r = this->MyBounds();
	r.InsetBy(10,15);
	r.top = r.bottom - font.Size()*4 - 30;
	r.bottom = r.top + font.Size()*2;
	this->AddChild(progress = new BStatusBar(r, "pbnStatusBar"));
	progress->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
	progress->SetMaxValue(100);
	progress->Hide();

	r = this->MyBounds();
	r.InsetBy(20,20);
	BRect s;
	s = r; s.top = s.bottom - font.Size()*2;
	float len = s.Width()/5;	
	s.right = s.left + len - 10;
	this->AddChild(refresh = new BButton(s, "pbnRefresh", _("Refresh"), new BMessage(PBNREFRESH), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));
//	s.OffsetBy(len*4,0);
//	this->AddChild(refresh = new BButton(s, "pbExport", _("Export"), new BMessage(PBEXPORT), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM));

	byNameList = new BList;
}

void pbByNameView::clearList(void) {
	// clear visible list
	if (list->CountItems()>0) {
		CLVEasyItem *anItem;
		for (int i=0; (anItem=(pbByNameListItem*)list->ItemAt(i)); i++)
			delete anItem;
		if (!list->IsEmpty())
			list->MakeEmpty();
	}
}

void pbByNameView::fillList(void) {
	pbByNameListItem *item;

	clearList();
	// insert master items (with names) and secondaries (with numbers and other info)
	// go through list and insert new master item for every name change
	int j = byNameList->CountItems();
	if (j==0)
		return;
	struct pbNum *c = (struct pbNum*)byNameList->ItemAt(0);
	// add 1st item (master and secondary)
	list->AddItem(new pbByNameListItem(c, 0, true));
	list->AddItem(new pbByNameListItem(c, 1, false));
	BString *last = ((union pbVal*)c->attr->ItemAt(1))->text;
	BString *cur;
	// go through the list and insert new master on each change
	for (int i=1;i<j;i++) {
		c = (struct pbNum*)byNameList->ItemAt(i);
		cur = ((union pbVal*)c->attr->ItemAt(1))->text;
		if (cur->Compare(last->String()) != 0) {
			last = cur;
			// there is name change! insert new master item with new name
			item = new pbByNameListItem(c, 0, true);
			list->AddItem(item);
		}
		// insert new secondary item
		item = new pbByNameListItem(c, 1, false);
		list->AddItem(item);
	}
}

void pbByNameView::fullListRefresh(void) {
	/// this is exactly as in pbView
	{
	progress->Reset();
	progress->Show();
	progress->Update(0, _("Reading phonebooks..."));

	int j = gsm->listMemSlotPB->CountItems();
	
	struct pbSlot *sl;
	BString pb;
	// count number of PB slots first
	int k = 0;
	for (int i=0; i<j; i++) {
		sl = (struct pbSlot*)gsm->listMemSlotPB->ItemAt(i);
		if (!sl->callreg) {
			pb = sl->sname;
			if (!(pb == "MT")||(pb == "AD"))
				k++;
		}
	}
	// calculate delta for progress bar
	float delta = (k > 0) ? 100/k : 0;
	// go through list again and fetch contents
	for (int i=0; i<j; i++) {
		sl = (struct pbSlot*)gsm->listMemSlotPB->ItemAt(i);
		if (!sl->callreg) {
			pb = sl->sname;
			if (!((pb == "MT")||(pb == "AD"))) {
				progress->Update(delta, sl->name.String());
				gsm->getPBList(pb.String());
			}
		}
	}
	progress->Hide();
	}
	///

	// clear sorted list, don't delete its contents
	if (byNameList->CountItems()>0) {
		if (!byNameList->IsEmpty())
			byNameList->MakeEmpty();
	}
	// go through phonebooks in phone (not callreg) and add pointers to one list
	BString n;
	struct pbSlot *sl;
	int j = gsm->listMemSlotPB->CountItems();
	for (int i=0;i<j;i++) {
		sl = (struct pbSlot*)gsm->listMemSlotPB->ItemAt(i);
		if (!sl->callreg) {
			// real stuff
			struct pbNum *pb;
			int k = sl->pb->CountItems();
			for (int l=0;l<k;l++) {
				pb = (struct pbNum*)sl->pb->ItemAt(l);
				byNameList->AddItem(pb);
			}
		}
	}
	// now sort list by name (just like in pbview to get uniques)
	byNameList->SortItems(&pbNumCompareByName);
}

void pbByNameView::Show(void) {
	BView::Show();
	BView::Flush();
	BView::Sync();
	if (list->CountItems()==0) {
		fullListRefresh();
	}
	fillList();
}

void pbByNameView::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case PBNREFRESH:
			fullListRefresh();
			fillList();
			break;
		case PBNLIST_INV:
			{	int i = list->CurrentSelection(0);
				if (i>=0)
					list->Expand((pbByNameListItem*)list->ItemAt(i));
			}
			break;
		case PBNLIST_SEL:
			break;
		default:
			mobileView::MessageReceived(Message);
			break;
	}
}
