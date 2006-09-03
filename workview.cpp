
#include <List.h>
#include <Message.h>
#include <Window.h>
#include "CLVEasyItem.h"
#include "ColumnListView.h"
#include "globals.h"
#include "gsm.h"
#include "callregslotview.h"
#include "calview.h"
#include "pbview.h"
#include "phonelistslotview.h"
#include "smsview.h"
#include "smsinboxview.h"
#include "statusview.h"
#include "workview.h"

#include <stdio.h>

#define VSPLIT	200

const uint32 LIST_INV 	= 'WL00';
const uint32 LIST_SEL 	= 'WL01';

workView::workView(BRect r) : BView(r, "workView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW) {

	// initialize data
	gsm = NULL;

	this->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	BRect r = this->Bounds();

	// add column list
	CLVContainerView *containerView;
	r.right = VSPLIT-15;
	list = new ColumnListView(r, &containerView, NULL, B_FOLLOW_TOP_BOTTOM|B_FOLLOW_LEFT,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST, true, true, true, true,
		B_FANCY_BORDER);
	list->AddColumn(new CLVColumn(NULL,20,CLV_EXPANDER|CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE));
	list->AddColumn(new CLVColumn(NULL,20,CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE|
		CLV_NOT_RESIZABLE|CLV_PUSH_PASS|CLV_MERGE_WITH_RIGHT));
	list->AddColumn(new CLVColumn(_("Devices"), VSPLIT-20-20, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->SetSortFunction(CLVEasyItem::CompareItems);
	this->AddChild(containerView);
	list->SetInvocationMessage(new BMessage(LIST_INV));
	list->SetSelectionMessage(new BMessage(LIST_SEL));

	curView = 0;

	pageView = new BList();
}

void workView::SetDevice(GSM *g) {
	gsm = g;

	if (!gsm) return;
	gsm->getPhoneData();

	// clear list
	// XXX do the same with views?
	if (list->CountItems()>0) {
		CLVEasyItem *anItem;
		for (int i=0; (anItem=(CLVEasyItem*)list->ItemAt(i)); i++)
			delete anItem;
		if (!list->IsEmpty())
			list->MakeEmpty();
	}

	mobileView *cv;
	infoItem *item;
	BRect r = this->Bounds();
	r.left = VSPLIT;
	int curitem = 0;

	// main item with mobile manuf+model
	BString tmp;
	tmp = gsm->getManuf();
	tmp = tmp.Truncate(tmp.FindFirst(" ")+1);
	tmp += gsm->getModel();
	list->AddItem(item = new infoItem(curitem++,tmp.String(),0,"Img:Phone",true,true));
	this->AddChild(cv = new statusView(r));
	pageView->AddItem(cv);
//	cv->Hide();			// XXX if hidden -> will never show up, why??
	cv->SetDevice(gsm);

	// phonebook items
	// superitem for PB
	list->AddItem(item = new infoItem(curitem++,_("Phonebook"),1,"Img:Contacts",true));
	this->AddChild(cv = new pbView(r));
	pageView->AddItem(cv);
	cv->Hide();
	cv->SetDevice(gsm);
	// pb items, except call register
	// add the rest of slots: except ??,??,??
	{
	int j = gsm->listMemSlotPB->CountItems();
	struct pbSlot *sl;
	bool hadcompositepb = false;
	BString pb;
	for (int i=0;i<j;i++) {
		sl = (struct pbSlot*)gsm->listMemSlotPB->ItemAt(i);
		if (!sl->callreg) {
			pb = sl->sname;
			if ( (pb != "??") && (pb != "??") && (pb != "??") && (pb != "??") ) {
				if (hadcompositepb && ((pb == "MT")||(pb == "AD"))) {
				} else {
				hadcompositepb = true;
				list->AddItem(item = new infoItem(curitem++, gsm->getPBMemSlotName(sl->sname.String()),2,iconNameForSlot(pb.String())));
				item->SetSlot(sl->sname.String());
				this->AddChild(cv = new phoneListSlotView(r,sl->sname.String(),gsm));
				pageView->AddItem(cv);
				cv->Hide();
				}
			}
		}
	}
	}
	// superitem for Call register
	list->AddItem(item = new infoItem(curitem++,_("Call register"),1,"Img:MobilePhone",true));
	pageView->AddItem(NULL);
	VV_DIALED = VV_DIALEDSIM = VV_MISSED = VV_RECEIVED = -1;
	if (gsm->hasPBSlot("DC")) {
		VV_DIALED = curitem++;
		list->AddItem(item = new infoItem(VV_DIALED,gsm->getPBMemSlotName("DC"),2,iconNameForSlot("DC")));
		item->SetSlot("DC");
		this->AddChild(cv = new callRegSlotView(r,"DC",gsm));
		pageView->AddItem(cv);
		cv->Hide();
	}
	if (gsm->hasPBSlot("LD")) {
		VV_DIALEDSIM = curitem++;
		list->AddItem(item = new infoItem(VV_DIALEDSIM,gsm->getPBMemSlotName("LD"),2,iconNameForSlot("LD")));
		item->SetSlot("LD");
		this->AddChild(cv = new callRegSlotView(r,"LD",gsm));
		pageView->AddItem(cv);
		cv->Hide();
	}
	if (gsm->hasPBSlot("MC")) {
		VV_MISSED = curitem++;
		list->AddItem(item = new infoItem(VV_MISSED,gsm->getPBMemSlotName("MC"),2,iconNameForSlot("MC")));
		item->SetSlot("MC");
		this->AddChild(cv = new callRegSlotView(r,"MC",gsm));
		pageView->AddItem(cv);
		cv->Hide();
	}
	if (gsm->hasPBSlot("RC")) {
		VV_RECEIVED = curitem++;
		list->AddItem(item = new infoItem(VV_RECEIVED,gsm->getPBMemSlotName("RC"),2,iconNameForSlot("RC")));
		item->SetSlot("RC");
		this->AddChild(cv = new callRegSlotView(r,"RC",gsm));
		pageView->AddItem(cv);
		cv->Hide();
	}
	// superitem for SMS
	list->AddItem(item = new infoItem(curitem++,_("SMS"),1,"Img:MailGeneric",true));
	this->AddChild(cv = new smsView(r));
	pageView->AddItem(cv);
	cv->Hide();
	cv->SetDevice(gsm);
	// SMS folders, except MT, in order: IM, OM-unsent, sent, draft, pozostałe
	VV_INBOX = VV_OUTBOX = VV_DRAFT = -1;

	if (gsm->hasSMSSlot("IM")) {
		VV_INBOX = curitem++;
		list->AddItem(item = new infoItem(VV_INBOX,gsm->getSMSMemSlotName("IM"),2,iconNameForSlot("IM")));
		item->SetSlot("IM");
		this->AddChild(cv = new smsInboxView(r,"IM"));
		pageView->AddItem(cv);
		cv->Hide();
		cv->SetDevice(gsm);
	}
	if (gsm->hasSMSSlot("OM")) {
		VV_OUTBOX = curitem++;
		list->AddItem(item = new infoItem(VV_OUTBOX,gsm->getSMSMemSlotName("OM"),2,iconNameForSlot("OM")));
		item->SetSlot("OM");
		this->AddChild(cv = new smsOutboxView(r,"OM"));
		pageView->AddItem(cv);
		cv->Hide();
		cv->SetDevice(gsm);
	}
	if (gsm->hasSMSSlot("DM")) {
		VV_DRAFT = curitem++;
		list->AddItem(item = new infoItem(VV_DRAFT,gsm->getSMSMemSlotName("DM"),2,iconNameForSlot("DM")));
		item->SetSlot("DM");
		this->AddChild(cv = new smsDraftboxView(r,"DM"));
		pageView->AddItem(cv);
		cv->Hide();
		cv->SetDevice(gsm);
	}
	// add the rest of slots: except MT, IM, OM, DM
	{
	int j = gsm->listMemSlotSMS->CountItems();
	struct memSlotSMS *sl;
	BString sn;
	for (int i=0;i<j;i++) {
		sl = (struct memSlotSMS*)gsm->listMemSlotSMS->ItemAt(i);
		sn = sl->sname;
		if ( (sn != "MT") && (sn != "IM") && (sn != "OM") && (sn != "DM") ) {
			list->AddItem(item = new infoItem(curitem++, gsm->getSMSMemSlotName(sl->sname.String()),2,iconNameForSlot(sn.String())));
			item->SetSlot(sl->sname.String());
			this->AddChild(cv = new smsBoxView(r,sl->sname.String()));
			pageView->AddItem(cv);
			cv->Hide();
			cv->SetDevice(gsm);
		}
	}
	}
	// superitem for Calendar
	list->AddItem(item = new infoItem(curitem++,_("Calendar"),1,"Img:Date"));
	this->AddChild(cv = new calView(r));
	pageView->AddItem(cv);
	cv->Hide();
	cv->SetDevice(gsm);

	// reset current right-hand view to curView
	SetCurView(0);
}

void workView::SetCurView(int v) {
	if (v == curView)
		return;
	printf("selected: %i\n",v);
	mobileView *vv;
	vv = (mobileView *)pageView->ItemAt(curView);
	if (vv) {
		vv->Hide();
		vv->Sync();
		this->Window()->Sync();
	}
	curView = v;
	vv = (mobileView *)pageView->ItemAt(curView);
	if (vv) {
		vv->Show();
		vv->Sync();
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
			if (pageView->ItemAt(curView))
				((mobileView *)pageView->ItemAt(curView))->MessageReceived(Message);
			break;
	}
}

const char *workView::iconNameForSlot(const char *slot) {
	BString sl = slot;
	if (sl == "IM") return "Img:Inbox";
	if (sl == "OM") return "Img:Outbox";
	if (sl == "BM") return "Img:Info";
	if (sl == "DM") return "Img:Notes";
	return NULL;
}
