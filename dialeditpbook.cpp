
#include <Alert.h>
//#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <Font.h>
//#include <List.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <Message.h>
#include <PopUpMenu.h>
#include <StringView.h>
#include <TextControl.h>
#include <TextView.h>

#include "globals.h"
#include "dialeditpbook.h"
#include "gsm.h"

#include <stdio.h>

const uint32 NC			= 'DP00';
const uint32 MENU_SLOT	= 'DP01';
const uint32 MENU_TYPE	= 'DP02';
const uint32 BUT_REVERT = 'DP03';
const uint32 BUT_SAVE	= 'DP04';

// this (quietly) depends on PK_* enum in GSM!!! (also revertData and msgrcvd)
const char *numTypes[] = { "Work", "Home", "Main", "Mobile", "Fax", "Pager", "E-mail", "Mailing list", NULL };

dialEditPB::dialEditPB(const char *sl, GSM *g, struct pbNum *p = NULL) : BWindow(
	BRect(350,250,650,250+145),
	_("Phonebook entry"),
	B_TITLED_WINDOW, 0) {

	gsm = g;

	slotWrite = NULL;
	if (p)
		printf("edit in slot [%s]\n",sl);
	else
		printf("new in slot [%s]\n",sl);

	view = new BView(this->Bounds(), "editPBView", B_FOLLOW_ALL_SIDES, 0);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(view);

	float left = view->Bounds().left+10;
	float right = view->Bounds().right-10;
	float half = left+(view->Bounds().Width()-20)/2-10;
	float quarters3 = left+(view->Bounds().Width()-20)*3/4-10;

	slotMenu = new BPopUpMenu(_("[select]"));
	BMessage *msg;
	BMenuItem *bi;

	int l = gsm->listMemSlotPB->CountItems();
	mSlots = new BMenuItem*[l+1];
	struct pbSlot *slot;
	for (int i=0;i<l;i++) {
		slot = (struct pbSlot*)gsm->listMemSlotPB->ItemAt(i);
		if ((slot->writable) && (slot->sname != "MT")) {
			msg = new BMessage(MENU_SLOT);
			msg->AddPointer("_slot",slot);
			slotMenu->AddItem(bi = new BMenuItem(slot->name.String(), msg));
			if (slot->sname == sl) {
				iniSlotNum = i;
				iniSlot = slot;
				slotWrite = slot;
				bi->SetMarked(true);
			}
			mSlots[i] = bi;
		}
	}

	// XXX at this point slotWrite can't be NULL! assert or do sth about it

	BMenuField *mf;
	view->AddChild(mf = new BMenuField(BRect(15,15,quarters3,40),"slotMenuf",_("Slot:"),slotMenu));
	mf->SetDivider(be_plain_font->StringWidth(mf->Label())+5);

	view->AddChild(idText = new BStringView(BRect(quarters3+5,15,right,30),"idText","ID:"));

	BString tmp;
	BRect r = view->Bounds();
	r.left = left;
	r.right = right;
	r.top = 55;
	r.bottom = r.top + 30;

	view->AddChild(tcNumber = new BTextControl(BRect(r),"tcNumber",_("Number"),NULL,new BMessage(NC)));
	tcNumber->SetDivider(50);
	r.OffsetBy(0,30);
	view->AddChild(tcName = new BTextControl(BRect(r),"tcName",_("Name"),NULL,new BMessage(NC)));
	tcName->SetDivider(50);
	r.OffsetBy(0,30);

	if (slotWrite->has_phtype || true) {
		this->ResizeBy(0, 30);
		r.right = half;
		r.top += 5;
		view->AddChild(cbPrimary = new BCheckBox(BRect(r),"cbPrimary",_("Primary number"),new BMessage(NC)));
		r.top -= 5;

		typeMenu = new BPopUpMenu(_("[select]"));
		for (int i=0;numTypes[i];i++) {
			msg = new BMessage(MENU_TYPE);
			msg->AddInt32("_id", i);
			mTypes[i] = new BMenuItem(numTypes[i], msg);
			typeMenu->AddItem(mTypes[i]);
		}
		r.OffsetBy(half,0);
		view->AddChild(mf = new BMenuField(r, "typeMenuf", _("Type"), typeMenu));
		mf->SetDivider(be_plain_font->StringWidth(mf->Label())+5);
	}

	BFont font(be_plain_font);
	this->ResizeBy(0,font.Size()*2+5);

	r = view->Bounds();
	r.InsetBy(20,20);
	r.top = r.bottom - font.Size()*2;
	tmp = _("Revert");
	r.right = r.left + font.StringWidth(tmp.String())+40;
	view->AddChild(new BButton(r, "butRevert", tmp.String(), new BMessage(BUT_REVERT), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));

	r = view->Bounds();
	r.InsetBy(20,20);
	r.top = r.bottom - font.Size()*2;
	tmp = _("Save");
	r.left = r.right - (font.StringWidth(tmp.String())+40);
	view->AddChild(new BButton(r, "butSave", tmp.String(), new BMessage(BUT_SAVE), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM));

	if (p) {
		num = p;
	} else {
		num = new pbNum;
		num->id = -1;
		num->kind = GSM::PK_MOBILE;
		num->primary = false;
	}
	revertData();

	Show();
}

void dialEditPB::revertData(void) {
	// restore slot
	mSlots[iniSlotNum]->SetMarked(true);
	slotWrite = iniSlot;
	// restore data
	tcNumber->SetText(num->number.String());
	tcName->SetText(num->name.String());
	id = num->id;
	updateIdText();
	// extended attributes
	if (slotWrite->has_phtype || true) {
		if ((num->kind>=GSM::PK_WORK) && (num->kind<=GSM::PK_MAILLIST))
			curType = num->kind;
		else
			curType = GSM::PK_MOBILE;
		mTypes[curType]->SetMarked(true);
		cbPrimary->SetValue(num->primary ? B_CONTROL_ON : B_CONTROL_OFF);
	}
}

void dialEditPB::validateData(void) {
	BString tmp;
	tmp = tcNumber->Text(); tmp.Truncate(slotWrite->numlen); tcNumber->SetText(tmp.String());
	tmp = tcName->Text(); tmp.Truncate(slotWrite->numlen); tcName->SetText(tmp.String());
}

void dialEditPB::updateIdText(void) {
	BString tmp;
	if (id > 0) {
		tmp = _("ID:"); tmp += " "; tmp << id;
	} else {
		tmp = _("New entry");
	}
	idText->SetText(tmp.String());
}

void dialEditPB::switchToSlot(struct pbSlot *slot) {
	id = -1;
printf("new slot:[%s]\n",slot->sname.String());
	slotWrite = slot;
printf("min:%i,max:%i\n",slotWrite->min,slotWrite->max);
	if (slotWrite == iniSlot)
		id = num->id;
	updateIdText();
	// check slot caps and limits
}

bool dialEditPB::saveData(void) {
	validateData();
// XXX implement me!
	// switch slot
	// send data
	// check result, show error
	return true;
}

void dialEditPB::MessageReceived(BMessage *Message) {
	int32 type = GSM::PK_MOBILE;
	void *ptr;

	switch (Message->what) {
		case NC:
			validateData();
			break;
		case BUT_REVERT:
			revertData();
			break;
		case BUT_SAVE:
			if (saveData())
				Quit();
			break;
		case MENU_TYPE:
			if (Message->FindInt32("_id", &type) == B_OK)
				curType = type;
			break;
		case MENU_SLOT:
			if (Message->FindPointer("_slot",&ptr) == B_OK)
				switchToSlot((struct pbSlot*)ptr);
			break;
		default:
			break;
	}
}
