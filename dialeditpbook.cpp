
#include <Alert.h>
#include <Button.h>
#include <CheckBox.h>
#include <Font.h>
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
const uint32 MENU_FIELD	= 'DP02';
const uint32 BUT_REVERT = 'DP03';
const uint32 BUT_SAVE	= 'DP04';

dialEditPB::dialEditPB(const char *sl, GSM *g, struct pbNum *p = NULL) : BWindow(
	BRect(350,250,650,250+85),
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
//	float half = left+(view->Bounds().Width()-20)/2-10;
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
	if (p) {
		num = p;
	} else {
		// create an empty pbNum
		num = new pbNum;
		num->id = -1;
		num->kind = GSM::PK_MOBILE;
		num->primary = false;
		// create attribute list
		num->attr = new BList;
		int j = slotWrite->fields->CountItems();
		struct pbField *pf;
		union pbVal *v;
		for (int i=0; i<j; i++) {
			pf = (struct pbField*)slotWrite->fields->ItemAt(i);
			v = new pbVal;
			switch (pf->type) {
				case GSM::PF_PHONE:
				case GSM::PF_PHONEEMAIL:
				case GSM::PF_TEXT:
					v->text = new BString("");
					break;
				case GSM::PF_BOOL:
					v->b = false;
					break;
				case GSM::PF_COMBO:
					v->v = -1;
					break;
			}
			num->attr->AddItem(v);
		}
	}

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

	// process slot fields
	int j = slotWrite->fields->CountItems();
	struct pbField *pf;
	BString id;
	BView *c;
	attr = new BList;
	union pbVal *pv;
	for (int i=0; i<j; i++) {
		// provide placeholder for menu value
		attr->AddItem(pv = new pbVal);
		pf = (struct pbField*)slotWrite->fields->ItemAt(i);
		id = "pf"; id << i;
		c = NULL;
		switch (pf->type) {
			case GSM::PF_PHONE:
			case GSM::PF_PHONEEMAIL:
				c = new BTextControl(BRect(r),id.String(),pf->name.String(),NULL,new BMessage(NC));
				break;
			case GSM::PF_TEXT:
				c = new BTextControl(BRect(r),id.String(),pf->name.String(),NULL,new BMessage(NC));
				break;
			case GSM::PF_BOOL:
				c = new BCheckBox(BRect(r),id.String(),pf->name.String(),new BMessage(NC));
				break;
			case GSM::PF_COMBO:
				{
					BPopUpMenu *m = new BPopUpMenu(_("[select]"));
					int l = pf->cb->CountItems();
					struct pbCombo *pc;
					for (int k=0; k<l; k++) {
						pc = (struct pbCombo*)pf->cb->ItemAt(k);
						msg = new BMessage(MENU_FIELD);
						msg->AddInt32("_id",i);
						msg->AddInt32("_value",pc->v);
						m->AddItem(new BMenuItem(pc->text.String(), msg));
					}
					c = new BMenuField(r, id.String(), pf->name.String(), m);
					// remember initial state
					pv->v = ((union pbVal*)num->attr->ItemAt(i))->v;
				}
				break;
		}
		if (c) {
			this->ResizeBy(0,30);
			view->AddChild(c);
			r.OffsetBy(0,30);
		}
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

	revertData();

	Show();
}

void dialEditPB::revertData(void) {
	// restore slot
	mSlots[iniSlotNum]->SetMarked(true);
	slotWrite = iniSlot;
	// restore data
	id = num->id;
	updateIdText();
	// extended attributes
	int j = slotWrite->fields->CountItems();
	struct pbField *pf;
	union pbVal *v;
	BView *c;
	BString id;
	for (int i=0; i<j; i++) {
		pf = (struct pbField*)slotWrite->fields->ItemAt(i);
		v = (union pbVal*)num->attr->ItemAt(i);
		id = "pf"; id << i;
		c = view->FindView(id.String());
		switch (pf->type) {
			case GSM::PF_PHONEEMAIL:
			case GSM::PF_PHONE:
			case GSM::PF_TEXT:
				((BTextControl*)c)->SetText(v->text->String());
				break;
			case GSM::PF_BOOL:
				((BCheckBox*)c)->SetValue(v->b ? B_CONTROL_ON : B_CONTROL_OFF);
				break;
			case GSM::PF_COMBO:
				{	BMenu *m = ((BMenuField*)c)->Menu();
					BMenuItem *it;
					int32 value;
					int k = m->CountItems();
					for (int l=0; l<k; l++) {
						it = m->ItemAt(l);
						it->Message()->FindInt32("_value",&value);
						if (value == v->v) {
							it->SetMarked(true);
							break;
						} else
							it->SetMarked(false);	// XXX doesn't quite work as deselector
					}
					((union pbVal*)attr->ItemAt(i))->v = v->v;
				}
				break;
		}
	}
}

void dialEditPB::validateData(void) {
	// go through all phone/p+e/text attributes, validate them and truncate
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
	// XXX check slot caps and limits
}

bool dialEditPB::saveData(void) {
	validateData();
	struct pbNum n;
	n.id = id;
	n.slot = slotWrite->sname;
	// go through all attributes and copy their values
	n.attr = new BList;	// XXX memleak - this is never freed
	union pbVal *nv;
	struct pbField *pf;
	BString id;
	BView *c;
	int j = slotWrite->fields->CountItems();
	for (int i=0; i<j; i++) {
		nv = new pbVal;
		pf = (struct pbField*)slotWrite->fields->ItemAt(i);
		// if 'combo' - read from own attr copy, otherwise from widget
		id = "pf"; id << i;
		c = view->FindView(id.String());
		switch (pf->type) {
			case GSM::PF_PHONEEMAIL:
			case GSM::PF_PHONE:
			case GSM::PF_TEXT:
				nv->text = new BString(((BTextControl*)c)->Text());
				break;
			case GSM::PF_BOOL:
				nv->b = (((BCheckBox*)c)->Value() == B_CONTROL_ON);
				break;
			case GSM::PF_COMBO:
				nv->v = ((union pbVal*)(attr->ItemAt(i)))->v;
				break;
		}
		n.attr->AddItem(nv);
	}
	// XXX validation - don't allow empty number/name??
	//
	if (gsm->storePBItem(&n) != 0) {
// XXX quite user friendly :/
		BAlert *a = new BAlert(APP_NAME, _("There was an error."),_("Ok"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		a->Go();
		return false;
	}
	return true;
}

void dialEditPB::MessageReceived(BMessage *Message) {
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
		case MENU_SLOT:
			{	void *ptr;
				if (Message->FindPointer("_slot",&ptr) == B_OK)
					switchToSlot((struct pbSlot*)ptr);
				break;
			}
		case MENU_FIELD:
			{	int32 id, value;
				Message->FindInt32("_id",&id);
				Message->FindInt32("_value",&value);
				((union pbVal*)attr->ItemAt(id))->v = value;
				break;
			}
		default:
			break;
	}
}
