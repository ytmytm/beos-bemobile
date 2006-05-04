
#include <List.h>
#include <Message.h>
#include "CLVEasyItem.h"
#include "ColumnListView.h"
#include "globals.h"
#include "gsm.h"
#include "workview.h"

#include <stdio.h>

const uint32 LIST_INV 	= 'WL00';
const uint32 LIST_SEL 	= 'WL01';

workView::workView(BRect r) : BView(r, "workView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW) {

	// initialize data
	gsm = NULL;

	this->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	BRect r = this->Bounds();

	// add column list
	CLVContainerView *containerView;
	r.right = 150-15;
	list = new ColumnListView(r, &containerView, NULL, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST, true, true, true, true,
		B_FANCY_BORDER);
	list->AddColumn(new CLVColumn(NULL,20,CLV_EXPANDER|CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE));
	list->AddColumn(new CLVColumn(NULL,20,CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE|
		CLV_NOT_RESIZABLE|CLV_PUSH_PASS|CLV_MERGE_WITH_RIGHT));
	list->AddColumn(new CLVColumn(_("Devices"), 150-20-20, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->SetSortFunction(CLVEasyItem::CompareItems);
	this->AddChild(containerView);
	list->SetInvocationMessage(new BMessage(LIST_INV));
	list->SetSelectionMessage(new BMessage(LIST_SEL));

	curView = 0;
}

void workView::SetDevice(GSM *g) {
	gsm = g;

	if (!gsm) return;
	gsm->getPhoneData();

	// clear list
	if (list->CountItems()>0) {
		CLVEasyItem *anItem;
		for (int i=0; (anItem=(CLVEasyItem*)list->ItemAt(i)); i++)
			delete anItem;
		if (!list->IsEmpty())
			list->MakeEmpty();
	}
	// add main item w/ mobile manuf+model
	infoItem *item;
	BString tmp;
	tmp = gsm->getManuf();
	tmp = tmp.Truncate(tmp.FindFirst(" ")+1);
	tmp += gsm->getModel();
	item = new infoItem(1,tmp.String(),0,true,true);
	list->AddItem(item);
	item = new infoItem(2,_("Phonebook"),1,true);
	list->AddItem(item);
	item = new infoItem(3,_("Contacts"),2);
	list->AddItem(item);
	item = new infoItem(4,_("Last Calls"),2);
	list->AddItem(item);
	item = new infoItem(5,_("SMS"),1,true);
	list->AddItem(item);
	item = new infoItem(6,_("Inbox"),2);
	list->AddItem(item);
	item = new infoItem(7,_("Outbox"),2);
	list->AddItem(item);
	//	- Calendar	XXX
	//	icons		XXX
	// reset current right-hand view to curView
	SetCurView(0);

	// XXX dopiero na zadanie reload lub przy pierwszym otwarciu strony z smsami
	gsm->getSMSList(((struct memSlotSMS*)gsm->listMemSlotSMS->ItemAt(0))->sname.String());
}

void workView::SetCurView(int v) {
	if (v == curView)
		return;
	curView = v;
	printf("selected: %i\n",v);
	// XXX do something with right-hand view
}

void workView::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case LIST_INV:
		case LIST_SEL:
			{	int i = list->CurrentSelection(0);
				if (i>=0) {
					i = ((infoItem*)list->ItemAt(list->CurrentSelection(0)))->Id();
					if (i>=0) {
						// changed selection
						SetCurView(i);
					}
				}
				break;
			}
		default:
			break;
	}
}
