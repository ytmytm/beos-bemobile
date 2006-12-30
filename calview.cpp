
#include <Alert.h>
#include <Button.h>
#include <Font.h>
#include <StatusBar.h>
#include <StringView.h>
#include "ColumnListView.h"
#include "globals.h"
#include "calview.h"
#include "dialcalevent.h"

#include <stdio.h>

const uint32	CALIST_INV	= 'CL00';
const uint32	CALIST_SEL	= 'CL01';
const uint32	CAREFRESH	= 'CA00';
const uint32	CAEDIT		= 'CA01';
const uint32	CANEW		= 'CA02';

calView::calView(BRect r) : mobileView(r, "calView") {
	caption->SetText(_("Calendar events"));

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
	maxw = font.StringWidth(_("999"))+20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("#"), maxw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	maxw = font.StringWidth(_("Title"))*7+20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Title"), maxw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	maxw = font.StringWidth(_("9999-99-99 99:99"))+20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Start"), maxw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	maxw = font.StringWidth(_("Duration"))*1+20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Duration"), maxw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	maxw = font.StringWidth(_("9999-99-99 99:99"))+20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Alarm"), maxw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->AddColumn(new CLVColumn(_("Repeat"), r.right-B_V_SCROLL_BAR_WIDTH-4-totalw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->SetSortFunction(CLVEasyItem::CompareItems);
	this->AddChild(containerView);
	list->SetInvocationMessage(new BMessage(CALIST_INV));
	list->SetSelectionMessage(new BMessage(CALIST_SEL));

	r = this->MyBounds();
	r.InsetBy(10,15);
	r.top = r.bottom - font.Size()*4 - 30;
	r.bottom = r.top + font.Size()*2;
	this->AddChild(progress = new BStatusBar(r, "caStatusBar"));
	progress->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
	progress->SetMaxValue(100);
	progress->Hide();

	r = this->MyBounds();
	r.InsetBy(20,20);
	BRect s;
	s = r; s.top = s.bottom - font.Size()*2;
	float len = s.Width()/5;	
	s.right = s.left + len - 10;
	this->AddChild(refresh = new BButton(s, "caRefresh", _("Refresh"), new BMessage(CAREFRESH), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));
	s.OffsetBy(len,0);
	this->AddChild(newevent = new BButton(s, "caNew", _("New"), new BMessage(CANEW), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));
	s.OffsetBy(len,0);
	this->AddChild(edit = new BButton(s, "caEdit", _("Edit"), new BMessage(CAEDIT), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));
//	s.OffsetBy(len*4,0);
//	this->AddChild(refresh = new BButton(s, "pbExport", _("Export"), new BMessage(PBEXPORT), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM));
}

void calView::clearList(void) {
	// clear list
	if (list->CountItems()>0) {
		CLVEasyItem *anItem;
		for (int i=0; (anItem=(calListItem*)list->ItemAt(i)); i++)
			delete anItem;
		if (!list->IsEmpty())
			list->MakeEmpty();
	}
}

void calView::fillList(void) {
	clearList();
	struct calEvent *ce;
	int j = gsm->listCalEvent->CountItems();
	for (int i=0; i<j; i++) {
		ce = (struct calEvent*)gsm->listCalEvent->ItemAt(i);
		list->AddItem(new calListItem(ce));
	}
}

void calView::fullListRefresh(void) {
	progress->Reset();
	progress->Show();
	progress->Update(0, _("Reading calendar events..."));

	gsm->getCalendarEvents();

	progress->Hide();
}

void calView::Show(void) {
	BView::Show();
	BView::Flush();
	BView::Sync();
	if (list->CountItems()==0) {
		fullListRefresh();
	}
	fillList();
}

void calView::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case CAREFRESH:
			fullListRefresh();
			fillList();
			break;
		case CANEW:
			dnev = new dialNewEvent(gsm);
			break;
		case CALIST_INV:
		case CAEDIT:
			{	int i = list->CurrentSelection(0);
				if (i>=0) {
					dnev = new dialNewEvent(gsm, ((calListItem*)list->ItemAt(i))->Event());
				}
				break;
			}
		case CALIST_SEL:
			break;
		default:
			mobileView::MessageReceived(Message);
			break;
	}
}
