
#include <Alert.h>
#include <Button.h>
#include <Font.h>
#include <StatusBar.h>
#include <TextControl.h>
#include <StringView.h>
#include "ColumnListView.h"
#include "globals.h"
#include "smsview.h"

#include <stdio.h>

#define SMS_MAIL_PATH "mail/SMS"

const uint32	SMSLIST_INV	= 'SL00';
const uint32	SMSLIST_SEL = 'SL01';
const uint32	SMSREFRESH	= 'SB00';
const uint32	SMSCBUTSET	= 'SB01';
const uint32	SMSEXPORTSLOT = 'SB02';

smsView::smsView(BRect r) : mobileView(r, "smsView") {
	caption->SetText(_("SMS summary"));

	BFont font(be_plain_font);
	float maxw, totalw = 0;
	BRect r = this->MyBounds();
	r.InsetBy(10,15);
	r.right -= B_V_SCROLL_BAR_WIDTH;
	r.bottom -= 100 + font.Size()*2 + 30;

	// add column list
	CLVContainerView *containerView;
	list = new ColumnListView(r, &containerView, NULL, B_FOLLOW_TOP_BOTTOM|B_FOLLOW_LEFT,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST, false, true, true, true,
		B_FANCY_BORDER);
	maxw = font.StringWidth(_("Folder"))*3+20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Folder"), maxw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	maxw = font.StringWidth(_("Writable"))*2+20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Writable"), maxw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	maxw = font.StringWidth(_("Unread"))*2+20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Unread"), maxw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	maxw = font.StringWidth(_("Total"))+20;
	list->AddColumn(new CLVColumn(_("Total"), r.right-B_V_SCROLL_BAR_WIDTH-4-totalw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->SetSortFunction(CLVEasyItem::CompareItems);
	this->AddChild(containerView);
	list->SetInvocationMessage(new BMessage(SMSLIST_INV));
	list->SetSelectionMessage(new BMessage(SMSLIST_SEL));

	r.top = r.bottom+font.Size()*2+10; r.bottom = r.top + font.Size()*2;
	r.right = r.left + font.StringWidth(_("SMS Centre")) + font.StringWidth("XXXXXXXXXXXXXXXX");
	this->AddChild(smsc = new BTextControl(r,"smsSMSC",_("SMS Centre number"),NULL,NULL,B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));
	smsc->SetDivider(font.StringWidth(_("SMS Centre number"))+5);
	r.left = r.right + 30; r.right = r.left + font.StringWidth(_("Set")) + 30;
	this->AddChild(setsmsc = new BButton(r, "smsSetSMSC", _("Set"), new BMessage(SMSCBUTSET), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));	

	r = this->MyBounds();
	r.InsetBy(10,15);
	r.top = r.bottom - font.Size()*4 - 30;
	r.bottom = r.top + font.Size()*2;
	this->AddChild(progress = new BStatusBar(r, "smsStatusBar"));
	progress->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
	progress->SetMaxValue(100);
	progress->Hide();

	BRect s, t;
	r = this->MyBounds();
	r.InsetBy(20,20);
	s = r; s.top = s.bottom - font.Size()*2; s.right = s.left + font.StringWidth("MMMMMMMMMM")+40;
	this->AddChild(refresh = new BButton(s, "smsRefresh", _("Refresh"), new BMessage(SMSREFRESH), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));
	t = s; t.right = r.right; t.left = t.right - (font.StringWidth("MMMMMMMMMM")+40);
	this->AddChild(exportslot = new BButton(t, "smsExport", _("Export"), new BMessage(SMSEXPORTSLOT), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM));
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
	int j = gsm->listMemSlotSMS->CountItems();
	struct memSlotSMS *sl;
	for (int i=0;i<j;i++) {
		sl = (struct memSlotSMS*)gsm->listMemSlotSMS->ItemAt(i);
		if ( (strcmp(sl->sname.String(),"MT")!=0) &&
			 (strcmp(sl->sname.String(),"IM")!=0) &&
			 (strcmp(sl->sname.String(),"OM")!=0) &&
			 (strcmp(sl->sname.String(),"DM")!=0) ) {
			list->AddItem(new smsListItem(sl));
		}
	}
}

void smsView::fullListRefresh(void) {
	struct memSlotSMS *sl, *mt, *me;
	struct SMS *sms;
	BString right;
	float delta;

	progress->Reset();
	progress->Show();
	// get number of all messages
	progress->Update(0, _("Checking number of messages..."));
	int msgnum = gsm->changeSMSMemSlot("MT");
	mt = gsm->getSMSSlot("MT");
	me = gsm->getSMSSlot("ME");

	delta = (msgnum > 0) ? 100/msgnum : 0;
	right = ""; right << msgnum;

	// read from slots
	int j = gsm->listMemSlotSMS->CountItems();
	for (int i=0;i<j;i++) {
		sl = (struct memSlotSMS*)gsm->listMemSlotSMS->ItemAt(i);
		if (sl != mt) {
			// change slot, read all messages
			gsm->getSMSList(sl->sname.String());
			int k = sl->msg->CountItems();
			for (int l=0;l<k;l++) {
				sms = (struct SMS*)sl->msg->ItemAt(l);
				gsm->getSMSContent(sms);
				if (sl != me)	// don't update for phone memory slot, it's virtual
					progress->Update(delta, sl->name.String(), right.String());
				else
					progress->Update(0, sl->name.String(),NULL);
			}
		}
	}
	progress->Hide();
	gsm->changeSMSMemSlot("MT");	// XXX configurable?
}

void smsView::smscRefresh(void) {
	smsc->SetText(gsm->getSMSC());
}

void smsView::smscSet(void) {
	BString tmp;
	tmp = smsc->Text();
	tmp.RemoveSet(" (),-.\t");
	if (tmp.Length()>0)
		gsm->setSMSC(tmp.String());
}

#include <Path.h>
#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <NodeInfo.h>

void smsView::smsExportSlot(struct memSlotSMS *slot) {
	if (slot->msg->CountItems() == 0) {
		gsm->getSMSList(slot->sname.String());
	}
	if (slot->msg->CountItems() == 0) {
		BAlert *err = new BAlert(APP_NAME, _("There are no messages to export."), _("Ok"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		err->Go();
		return;
	}

	// create directories for output
	BPath path;
	BDirectory dir;
	BFile file;
	status_t status;

	if ((status = find_directory(B_USER_DIRECTORY, &path)) != B_OK)
		return;	// no user directory? XXX

	// append relative path to slot export list
	path.Append(SMS_MAIL_PATH);
	status = dir.SetTo(path.Path());
	switch (status) {
		case B_ENTRY_NOT_FOUND:
			if ((status = dir.CreateDirectory(path.Path(), &dir)) != B_OK)
				return;		// couldn't be created? XXX
		case B_OK:
			break;
		default:
			return;
	}
	path.Append(slot->name.String());
	status = dir.SetTo(path.Path());
	switch (status) {
		case B_ENTRY_NOT_FOUND:
			if ((status = dir.CreateDirectory(path.Path(), &dir)) != B_OK)
				return;		// couldn't be created? XXX
		case B_OK:
			break;
		default:
			return;	// other error
	}

	// for each message in slot...
	// put a file w/ attributes & mimetype for each message
	BString fname, fname2, tmp, mime;
	uint32 flags;
	struct SMS *sms;
	bool draft = (slot->sname.Compare("DM") == 0);
	int k = slot->msg->CountItems();
	for (int l=0;l<k;l++) {
		// get SMS...
		sms = (struct SMS*)slot->msg->ItemAt(l);
		gsm->getSMSContent(sms);
		// create the file, if the name exists, find unique one
		flags = B_WRITE_ONLY | B_CREATE_FILE | B_FAIL_IF_EXISTS;
		// clear date
		tmp = sms->date.String();
		tmp.ReplaceAll("/",".");
		fname = sms->number.String(); fname << "_" << tmp.String() << "_" << sms->id;
		fname2 = BString(fname.String());
		for (int i=1; (status = file.SetTo(&dir, fname2.String(), flags)) != B_OK; i++) {
			if (status != B_FILE_EXISTS)
				return;	// other error
			else {
				fname2 = fname.String();
				fname2 << "_" << i;
			}
		}
		file.Write(sms->msg.String(), sms->msg.Length());
		// add attributes
		if (draft) {
			uint32 draftAttr = draft;
			file.WriteAttr("MAIL:draft",B_INT32_TYPE, 0, &draftAttr, sizeof(uint32));
		}
		tmp = gsm->smsNumberTextContent(sms);
		if (tmp.Length()>0) {
			// only for inbox
			if (slot->sname.Compare("IM") == 0) {
				file.WriteAttr("MAIL:from",B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
			}
			file.WriteAttr("MAIL:to",B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
			file.WriteAttr("MAIL:name",B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
		}
		// mimetype
//		mime = draft ? "text/x-vnd.Be-MailDraft" : "text/x-email";
		mime = "text/x-vnd.Be-MailDraft";
		BNodeInfo info(&file);
		info.SetType(mime.String());
	}

	tmp = _("Messages from this box have been exported to:");
	tmp << "\n" << path.Path();

	BAlert *nfo = new BAlert(APP_NAME, tmp.String(), _("Ok"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
	nfo->Go();
}

void smsView::Show(void) {
	BView::Show();
	BView::Flush();
	BView::Sync();
	fillList();
	smscRefresh();
}

void smsView::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case SMSREFRESH:
			smscRefresh();
			fullListRefresh();
			fillList();
			break;
		case SMSCBUTSET:
			smscSet();
		case SMSLIST_INV:
		case SMSLIST_SEL:
			break;
		case SMSEXPORTSLOT:
			{	int i = list->CurrentSelection(0);
				if (i>=0) {
					smsExportSlot(((smsListItem*)list->ItemAt(i))->Slot());
				}
				break;
			}
		default:
			mobileView::MessageReceived(Message);
			break;
	}
}
