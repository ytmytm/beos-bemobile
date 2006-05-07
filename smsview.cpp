//
// after refresh - do get infos for all lists
//

#include <Button.h>
#include <Font.h>
#include <StatusBar.h>
#include <StringView.h>
#include "CLVEasyItem.h"
#include "ColumnListView.h"
#include "globals.h"
#include "smsview.h"

#include <stdio.h>

const uint32	SMSLIST_INV	= 'SL00';
const uint32	SMSLIST_SEL = 'SL01';
const uint32	SMSREFRESH	= 'SB00';

smsView::smsView(BRect r) : mobileView(r, "smsView") {
	caption->SetText(_("SMS summary"));

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
	maxw = font.StringWidth(_("Folder"))*3+20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Folder"), maxw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	maxw = font.StringWidth(_("Unread"))*2+20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Unread"), maxw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	maxw = font.StringWidth(_("Total"))+20;
	list->AddColumn(new CLVColumn(_("Total"), r.right-B_V_SCROLL_BAR_WIDTH-4-totalw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->SetSortFunction(CLVEasyItem::CompareItems);
	this->AddChild(containerView);
	list->SetInvocationMessage(new BMessage(SMSLIST_INV));
	list->SetSelectionMessage(new BMessage(SMSLIST_SEL));

	r = this->MyBounds();
	r.InsetBy(10,15);
	r.top = r.bottom - font.Size()*4 - 30;
	r.bottom = r.top + font.Size()*2;
	this->AddChild(progress = new BStatusBar(r, "smsStatusBar"));
	progress->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
	progress->SetMaxValue(100);
	progress->Hide();

	BRect s;
	r = this->MyBounds();
	r.InsetBy(20,20);
	s = r; s.top = s.bottom - font.Size()*2; s.right = s.left + font.StringWidth("MMMMMMMMMM")+40;
	this->AddChild(refresh = new BButton(s, "smsRefresh", _("Refresh"), new BMessage(SMSREFRESH), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));
//	t = r; t.top = s.top; t.bottom = s.bottom; t.right = r.right; t.left = t.right - (font.StringWidth("MMMMMMMMMM")+40);
//	this->AddChild(del = new BButton(t, "smsDelete", _("Delete"), new BMessage(SMSDELETE), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM));
}

void smsView::clearList(void) {
	// clear list
	if (list->CountItems()>0) {
		CLVEasyItem *anItem;
		for (int i=0; (anItem=(smsListItem*)list->ItemAt(i)); i++)
			delete anItem;
		if (!list->IsEmpty())
			list->MakeEmpty();
	}
}

void smsView::fillList(void) {

	clearList();
//	progress->Reset();
//	progress->Show();
//
//	progress->Update(0, _("Checking number of messagess..."));
	gsm->updateSMSInfo();
	// add SMS folders in order: IM, OM-unsent, OM-sent, Drafts, rest
	if (gsm->hasSMSSlot("IM")) {
		list->AddItem(new smsListItem(gsm->getSMSSlot("IM")));
	}
	if (gsm->hasSMSSlot("OM")) {
		list->AddItem(new smsListItem(gsm->getSMSSlot("OM")));
	}
	if (gsm->hasSMSSlot("DM")) {
		list->AddItem(new smsListItem(gsm->getSMSSlot("DM")));
	}
	// the rest
	int i;
	int j = gsm->listMemSlotSMS->CountItems();
	struct memSlotSMS *sl;
	for (i=0;i<j;i++) {
		sl = (struct memSlotSMS*)gsm->listMemSlotSMS->ItemAt(i);
		if ( (strcmp(sl->sname.String(),"MT")!=0) &&
			 (strcmp(sl->sname.String(),"IM")!=0) &&
			 (strcmp(sl->sname.String(),"OM")!=0) &&
			 (strcmp(sl->sname.String(),"DM")!=0) ) {
			list->AddItem(new smsListItem(sl));
		}
	}
//	progress->Hide();
}

#include <Window.h>

void smsView::Show(void) {
printf("show!\n");
	BView::Show();
	fillList();
//	if (list->CountItems()==0) {
//		BView::Flush();
//		BView::Sync();
//		this->Window()->Flush();
//		this->Window()->Sync();
//		fillList();	// XXX what's the mode here?
//	}
}

void smsView::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case SMSREFRESH:
			fillList();
			break;
		case SMSLIST_INV:
		case SMSLIST_SEL:
			{	int i = list->CurrentSelection(0);
				if (i>=0) {
//					struct SMS *sms = ((smsListItem*)list->ItemAt(list->CurrentSelection(0)))->Msg();
//					if (sms)
//						updatePreview(sms);

				} else {
//					prv->SetText("");
				}
				break;
			}
		default:
			break;
	}
}
