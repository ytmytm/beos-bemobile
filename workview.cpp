
#include <List.h>
#include <Message.h>
#include <Window.h>
#include "CLVEasyItem.h"
#include "ColumnListView.h"
#include "globals.h"
#include "gsm.h"
#include "smsview.h"
#include "statusview.h"
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
	list = new ColumnListView(r, &containerView, NULL, B_FOLLOW_TOP_BOTTOM|B_FOLLOW_LEFT,
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
	item = new infoItem(V_SUMMARY,tmp.String(),0,true,true);
	list->AddItem(item);
	item = new infoItem(V_PHONEBOOK,_("Phonebook"),1,true);
	list->AddItem(item);
	item = new infoItem(V_CONTACTS,_("Contacts"),2);
	list->AddItem(item);
	item = new infoItem(V_LASTCALLS,_("Last Calls"),2);
	list->AddItem(item);
	item = new infoItem(V_SMS,_("SMS"),1,true);
	list->AddItem(item);
	item = new infoItem(V_INBOX,_("Inbox"),2);
	list->AddItem(item);
	item = new infoItem(V_OUTBOX,_("Outbox"),2);
	list->AddItem(item);
	//	- Calendar	XXX
	//	icons		XXX
	// create views
	for (int i=0;i<V_MAX;i++) pageView[i] = NULL;

	BRect r = this->Bounds();
	r.left = 150;
	this->AddChild(pageView[V_SUMMARY] = new statusView(r));
//	pageView[V_SUMMARY]->Hide();	// XXX if hidden -> will never show up, why??
	pageView[V_SUMMARY]->SetDevice(gsm);
	this->AddChild(pageView[V_SMS] = new smsView(r));
	pageView[V_SMS]->Hide();
	pageView[V_SMS]->SetDevice(gsm);

	// reset current right-hand view to curView
	SetCurView(V_SUMMARY);
//	SetCurView(V_SMS);

	// XXX dopiero na zadanie reload lub przy pierwszym otwarciu strony z smsami
//	gsm->getSMSList(((struct memSlotSMS*)gsm->listMemSlotSMS->ItemAt(0))->sname.String());
}

void workView::SetCurView(int v) {
	if (v == curView)
		return;
	printf("selected: %i\n",v);
	if (pageView[curView]) {
		pageView[curView]->Hide();
		pageView[curView]->Sync();
		this->Window()->Sync();
	}
	curView = v;
	if (pageView[curView]) {
		pageView[curView]->Show();
		pageView[curView]->Sync();
		this->Window()->Sync();
	}
	this->Sync();
	this->Window()->Sync();
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
			if (pageView[curView])
				pageView[curView]->MessageReceived(Message);
			break;
	}
}
