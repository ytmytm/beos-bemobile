//
// counter znaków (pochodna textview lub view z pulse)
//
// obsługa:
// przed wysłaniem: walidacja czy numer

#include <Alert.h>
#include <Box.h>
#include <Button.h>
#include <Font.h>
#include <List.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <Message.h>
#include <PopUpMenu.h>
#include <StringView.h>
#include <TextControl.h>
#include <TextView.h>

#include "globals.h"
#include "dialnewsms.h"
#include "gsm.h"

#include <stdio.h>

const uint32 NC				= 'DN00';
const uint32 MENU_SLOT		= 'DN01';
const uint32 MENU_NAME		= 'DN02';
const uint32 BUT_SENDNOW	= 'DN03';
const uint32 BUT_SAVE		= 'DN04';
const uint32 BUT_COMPRESS	= 'DN05';

dialNewSMS::dialNewSMS(const char *sl, GSM *g) : BWindow(
	BRect(260,330,760,520),
	_("New SMS"),
	B_TITLED_WINDOW, 0) {

	gsm = g;

	slotWrite = NULL;

printf("new in slot:%s\n",sl);

	view = new BView(this->Bounds(), "newSMSView", B_FOLLOW_ALL_SIDES,0);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(view);

	slotMenu = new BPopUpMenu(_("[select]"));
	BMessage *msg;
	BMenuItem *bi;

	int l = gsm->listMemSlotSMS->CountItems();
	struct memSlotSMS *slot;
	for (int i=0;i<l;i++) {
		slot = (struct memSlotSMS*)gsm->listMemSlotSMS->ItemAt(i);
		if ((slot->writable) && (slot->sname != "MT")) {
			msg = new BMessage(MENU_SLOT);
			msg->AddPointer("_slot",slot);
			slotMenu->AddItem(bi = new BMenuItem(slot->name.String(), msg));
			if (slot->sname == sl) {
				slotWrite = slot;
				bi->SetMarked(true);
			}
		}
	}
	BMenuField *mf;
	view->AddChild(mf = new BMenuField(BRect(15,15,130,40),"slotMenuf",_("Slot:"),slotMenu));
	mf->SetDivider(be_plain_font->StringWidth(mf->Label())+5);

	numberText = new BTextControl(BRect(150,15,320,40),"numberText",_("To:"),NULL,new BMessage(NC));
	numberText->SetDivider(be_plain_font->StringWidth(numberText->Label())+5);
	view->AddChild(numberText);

	pbMenu = new BPopUpMenu("");
	struct pbSlot *pbSlot;
	struct pbNum *pb;
	int k;
	BString tmp;
	BString *number, *name;
	l = gsm->listMemSlotPB->CountItems();
	for (int i=0;i<l;i++) {
		pbSlot = (struct pbSlot*)gsm->listMemSlotPB->ItemAt(i);
		if (! pbSlot->callreg) {
			k = pbSlot->pb->CountItems();
			for (int j=0;j<k;j++) {
				pb = (struct pbNum*)pbSlot->pb->ItemAt(j);
				msg = new BMessage(MENU_NAME);
				msg->AddPointer("_pb",pb);
				number = ((union pbVal*)pb->attr->ItemAt(0))->text;
				name = ((union pbVal*)pb->attr->ItemAt(1))->text;
				if (name->Length() > 0) {
					tmp = name->String(); tmp += " ("; tmp += number->String(); tmp += ")";
				} else
					tmp = number->String();
				pbMenu->AddItem(new BMenuItem(tmp.String(),msg));
			}
		}
	}
	view->AddChild(new BMenuField(BRect(340,15,370,40),"nameMenuf",NULL,pbMenu));

//	view->AddChild(countText = new BStringView(BRect(380,15,480,30),"countText",NULL));
//	countText->SetText("Count: XXX/YYY");

	BRect r = view->Bounds();
	r.InsetBy(20,20);
	r.top = 55;
	r.bottom = r.top + 80;
	BBox *box = new BBox(r,"newSMSBox");
	box->SetLabel("");
	view->AddChild(box);
	r = box->Bounds();
	r.InsetBy(10,10);
	BRect s = r; s.OffsetTo(0,0);
	box->AddChild(content = new BTextView(r, "smsContent", s, B_FOLLOW_LEFT|B_FOLLOW_TOP,B_WILL_DRAW));
	BFont f(be_fixed_font);
	content->SetFontAndColor(0,0,&f);
	content->SetWordWrap(true);
	content->SetMaxBytes(700);	// ???
	content->MakeEditable(true);
	content->MakeSelectable(true);

	BFont font(be_plain_font);
	r = view->Bounds();
	r.InsetBy(20,20);
	r.top = r.bottom - font.Size()*2;
	tmp = _("Send now");
	r.right = r.left + font.StringWidth(tmp.String()) + 20;
	view->AddChild(new BButton(r, "butSendNow", tmp.String(), new BMessage(BUT_SENDNOW), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));

	r.OffsetBy(r.Width()+10,0);
	tmp = _("Store");
	r.right = r.left + font.StringWidth(tmp.String()) + 20;
	view->AddChild(new BButton(r, "butSave", tmp.String(), new BMessage(BUT_SAVE), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));

	r = view->Bounds();
	r.InsetBy(20,20);
	r.top = r.bottom - font.Size()*2;
	tmp = _("Compress text");
	r.left = r.right - (font.StringWidth(tmp.String())+20);
	view->AddChild(new BButton(r, "butCompress", tmp.String(), new BMessage(BUT_COMPRESS), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));

	Show();
}

void dialNewSMS::MessageReceived(BMessage *Message) {
	void *ptr;
	switch (Message->what) {
		case MENU_SLOT:
			{
				if (Message->FindPointer("_slot",&ptr) == B_OK) {
					slotWrite = (struct memSlotSMS*)ptr;
				}
				break;
			}
		case MENU_NAME:
			{
				if (Message->FindPointer("_pb",&ptr) == B_OK) {
					struct pbNum *pb = (struct pbNum*)ptr;
					numberText->SetText(pb->number.String());
				}
				break;
			}
		case BUT_COMPRESS:
			{
				BString tmp;
				tmp = content->Text();
				tmp.CapitalizeEachWord();
				tmp.RemoveSet("\t\n ");
				content->Looper()->Lock();
				content->SetText(tmp.String());
				content->Looper()->Unlock();
				break;
			}
		case BUT_SAVE:
			{	// check for correct data entry
				if (!slotWrite) {
					BAlert *err = new BAlert(APP_NAME, _("Please select memory slot"), _("Ok"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
					err->Go();
					break;
				}
				if ( (!numberValid()) || (!contentValid()) )
					break;
				printf("slot:%s\n",slotWrite->sname.String());
				int ret = gsm->storeSMS(slotWrite->sname.String(),numberText->Text(),content->Text());
				// check return, update slot list?
				// close window
				if (ret == 0)
					Quit();
				else {
					BAlert *err = new BAlert(APP_NAME, _("There was an error and message probably hasn't been stored."), _("Ok"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
					err->Go();
					break;
				}
				break;
			}
		case BUT_SENDNOW:
			{
				BAlert *ask = new BAlert(APP_NAME, _("Do you really want to send this message now?"), _("Yes"), _("No"), NULL, B_WIDTH_AS_USUAL, B_IDEA_ALERT);
				if (ask->Go() != 0)
					break;
				if ( (!numberValid()) || (!contentValid()) )
					break;
				int ret = gsm->sendSMS(numberText->Text(), content->Text());
				// check return, update slot list?
				// close window
				if (ret == 0)
					Quit();
				else {
					BAlert *err = new BAlert(APP_NAME, _("There was an error and message probably hasn't been sent.\nYour phone may not support direct SMS sending,\ntry saving the message and sending from list window."), _("Ok"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
					err->Go();
					break;
				}
				break;
			}
		default:
			BWindow::MessageReceived(Message);
			break;
	}
}

bool dialNewSMS::numberValid(bool showalert = true) {
	BString tmp = numberText->Text();
	int j = tmp.Length();
	if (j == 0) {
		if (showalert) {
			BAlert *err = new BAlert(APP_NAME, _("Please enter destination number."), _("Ok"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			err->Go();
		}
		return false;
	}
	int l = 0;	// number of good chars
	for (int i=0; i<j; i++) {
		if ( ( (tmp[i]>='0') && (tmp[i]<='9') ) ||
			 ( (tmp[i]=='*') || (tmp[i]=='#') || (tmp[i]=='+') || (tmp[i]==' ') ) )
			l++;
	}
	// if every char is good return true
	bool ret = (l == j);
	// alert user
	if ((showalert) && (!ret)) {
		BAlert *err = new BAlert(APP_NAME, _("The number is invalid. It may only contain digits, *, #, + and spaces."), _("Ok"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		err->Go();
	}
	return ret;
}

bool dialNewSMS::contentValid(bool showalert = true) {
	BString tmp = content->Text();
	int j = tmp.Length();
	if (j == 0) {
		if (showalert) {
			BAlert *err = new BAlert(APP_NAME, _("Please enter message"), _("Ok"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			err->Go();
		}
		return false;
	}
	return true;
}
