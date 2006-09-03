
#include <Alert.h>
#include <Button.h>
#include <Font.h>
#include <StatusBar.h>
#include <StringView.h>
#include "ColumnListView.h"
#include "globals.h"
#include "pbview.h"

#include <stdio.h>

const uint32	PBLIST_INV	= 'PL00';
const uint32	PBLIST_SEL	= 'PL01';
const uint32	PBREFRESH	= 'PB00';
const uint32	PBEXPORT	= 'PB01';

pbView::pbView(BRect r) : mobileView(r, "pbView") {
	caption->SetText(_("Phonebooks summary"));

	BFont font(be_plain_font);
	float maxw, totalw = 0;
	BRect r = this->MyBounds();
	r.InsetBy(10,15);
	r.right -= B_V_SCROLL_BAR_WIDTH;
	r.bottom -= 100;

	// add column list
	CLVContainerView *containerView;
	list = new ColumnListView(r, &containerView, NULL, B_FOLLOW_TOP_BOTTOM|B_FOLLOW_LEFT,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST, false, true, true, true,
		B_FANCY_BORDER);
	maxw = font.StringWidth(_("Folder"))*5+20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Folder"), maxw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	maxw = font.StringWidth(_("Used"))*2+20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Used"), maxw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	maxw = font.StringWidth(_("Total"))*2+20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Total"), maxw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	maxw = font.StringWidth(_("Used (%)"))+20;
	list->AddColumn(new CLVColumn(_("Used (%)"), r.right-B_V_SCROLL_BAR_WIDTH-4-totalw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->SetSortFunction(CLVEasyItem::CompareItems);
	this->AddChild(containerView);
	list->SetInvocationMessage(new BMessage(PBLIST_INV));
	list->SetSelectionMessage(new BMessage(PBLIST_SEL));

	r = this->MyBounds();
	r.InsetBy(10,15);
	r.top = r.bottom - font.Size()*4 - 30;
	r.bottom = r.top + font.Size()*2;
	this->AddChild(progress = new BStatusBar(r, "pbStatusBar"));
	progress->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
	progress->SetMaxValue(100);
	progress->Hide();

	r = this->MyBounds();
	r.InsetBy(20,20);
	BRect s;
	s = r; s.top = s.bottom - font.Size()*2;
	float len = s.Width()/5;	
	s.right = s.left + len - 10;
	this->AddChild(refresh = new BButton(s, "pbRefresh", _("Refresh"), new BMessage(PBREFRESH), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));
	s.OffsetBy(len*4,0);
	this->AddChild(refresh = new BButton(s, "pbExport", _("Export"), new BMessage(PBEXPORT), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM));
}

void pbView::clearList(void) {
	// clear list
	if (list->CountItems()>0) {
		CLVEasyItem *anItem;
		for (int i=0; (anItem=(pbListItem*)list->ItemAt(i)); i++)
			delete anItem;
		if (!list->IsEmpty())
			list->MakeEmpty();
	}
}

void pbView::fillList(void) {

	clearList();
	// add items in order of pages in workView, then the rest
	int j = gsm->listMemSlotPB->CountItems();
	struct pbSlot *sl;
	bool hadcompositepb = false;
	BString pb;
	for (int i=0; i<j; i++) {
		sl = (struct pbSlot*)gsm->listMemSlotPB->ItemAt(i);
		if (!sl->callreg) {
			pb = sl->sname;
			if (hadcompositepb && ((pb == "MT")||(pb == "AD"))) {
			} else {
				hadcompositepb = true;
				list->AddItem(new pbListItem(sl));
			}
		}
	}
}

void pbView::fullListRefresh(void) {

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

void pbView::Show(void) {
	BView::Show();
	BView::Flush();
	BView::Sync();
	fillList();
}

void pbView::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case PBREFRESH:
			fullListRefresh();
			fillList();
			break;
		case PBEXPORT:
			{	int i = list->CurrentSelection(0);
				if (i>=0) {
					struct pbSlot *sl = ((pbListItem*)list->ItemAt(i))->Slot();
					BString tmp = "About to export slot: "; tmp += sl->sname; tmp += " IMPLEMENT ME!";
					BAlert *a = new BAlert(APP_NAME, tmp.String(), _("Ok"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
					a->Go();
				}
			}
		case PBLIST_INV:
		case PBLIST_SEL:
			break;
		default:
			mobileView::MessageReceived(Message);
			break;
	}
}
