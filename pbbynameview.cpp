
//
// XXX:it is ugly to recognize attribute type by its label
//		(here and hardcoded offsets in refresh for listitem)
//		- add 'type' constant to slot-fields (temporary)
//		- replace pbNum w/ bmessage and attribute list
// XXX:VCF export probably needs QP-encoding!

#include <Alert.h>
#include <Button.h>
#include <Directory.h>
#include <File.h>
#include <Font.h>
#include <Node.h>
#include <NodeInfo.h>
#include <StatusBar.h>
#include <StringView.h>
#include "ColumnListView.h"
#include "globals.h"
#include "pbbynameview.h"

#include <stdio.h>
#include <time.h>

char *people_path = "/boot/home/people/BeMobile/";

#define VCFPATH "/boot/home/people/"
#define VCFFILE "people.vcf"

const uint32	PBNLIST_INV	= 'PBN0';
const uint32	PBNLIST_SEL	= 'PBN1';
const uint32	PBNREFRESH	= 'PBN2';
const uint32	PBNDIAL		= 'PBN3';
const uint32	PBNEXPORTVCF = 'PBN4';
const uint32	PBNEXPORTPPL = 'PBN5';


pbByNameView::pbByNameView(BRect r) : mobileView(r, "pbByNameView") {
	caption->SetText(_("Phonebook by name"));

	BFont font(be_plain_font);
	float maxw, totalw = 0;
	BRect r = this->MyBounds();
	r.InsetBy(10,15);
	r.right -= B_V_SCROLL_BAR_WIDTH;
	r.bottom -= 100;

	// add column list
	CLVContainerView *containerView;
	list = new ColumnListView(r, &containerView, NULL, B_FOLLOW_TOP_BOTTOM|B_FOLLOW_LEFT,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST, true, true, true, true,
		B_FANCY_BORDER);
	totalw = 20;
	list->AddColumn(new CLVColumn(NULL,20,CLV_EXPANDER|CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE));
	maxw = font.StringWidth("M")*17+20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Name"), maxw, CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE|CLV_PUSH_PASS|CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE));
//	maxw = font.StringWidth(_("9"))*12+20; totalw += maxw;
//	list->AddColumn(new CLVColumn(_("Number"), maxw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->AddColumn(new CLVColumn(_("Number"), r.right-B_V_SCROLL_BAR_WIDTH-4-totalw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->SetSortFunction(CLVEasyItem::CompareItems);
	this->AddChild(containerView);
	list->SetInvocationMessage(new BMessage(PBNLIST_INV));
	list->SetSelectionMessage(new BMessage(PBNLIST_SEL));

	r = this->MyBounds();
	r.InsetBy(10,15);
	r.top = r.bottom - font.Size()*4 - 30;
	r.bottom = r.top + font.Size()*2;
	this->AddChild(progress = new BStatusBar(r, "pbnStatusBar"));
	progress->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
	progress->SetMaxValue(100);
	progress->Hide();

	r = this->MyBounds();
	r.InsetBy(20,20);
	BRect s;
	s = r; s.top = s.bottom - font.Size()*2;
	float len = s.Width()/5;	
	s.right = s.left + len - 10;
	this->AddChild(refresh = new BButton(s, "pbnRefresh", _("Refresh"), new BMessage(PBNREFRESH), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));
	s.OffsetBy(len, 0);
	this->AddChild(exportvcf = new BButton(s, "pbnExportVCF", _("Export to vCard"), new BMessage(PBNEXPORTVCF), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));
	s.OffsetBy(len, 0);
	this->AddChild(exportppl = new BButton(s, "pbnExportPpl", _("Export to People"), new BMessage(PBNEXPORTPPL), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));
	s.OffsetBy(len*2,0);
	this->AddChild(dial = new BButton(s, "pbnDial", _("Dial"), new BMessage(PBNDIAL), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM));

	byNameList = new BList;
}

void pbByNameView::clearList(void) {
	// clear visible list
	if (list->CountItems()>0) {
		CLVEasyItem *anItem;
		for (int i=0; (anItem=(pbByNameListItem*)list->ItemAt(i)); i++)
			delete anItem;
		if (!list->IsEmpty())
			list->MakeEmpty();
	}
}

void pbByNameView::fillList(void) {
	pbByNameListItem *item;

	clearList();
	// insert master items (with names) and secondaries (with numbers and other info)
	// go through list and insert new master item for every name change
	int j = byNameList->CountItems();
	if (j==0)
		return;
	struct pbNum *c = (struct pbNum*)byNameList->ItemAt(0);
	// add 1st item (master and secondary)
	int master = 0;
	list->AddItem(new pbByNameListItem(master, c, gsm, 0, true));
	list->AddItem(new pbByNameListItem(master, c, gsm, 1, false));
	BString *last = ((union pbVal*)c->attr->ItemAt(1))->text;
	BString *cur;
	// go through the list and insert new master on each change
	for (int i=1;i<j;i++) {
		c = (struct pbNum*)byNameList->ItemAt(i);
		cur = ((union pbVal*)c->attr->ItemAt(1))->text;
		if (cur->Compare(last->String()) != 0) {
			last = cur;
			master = i;
			// there is name change! insert new master item with new name
			item = new pbByNameListItem(master, c, gsm, 0, true);
			list->AddItem(item);
		}
		// insert new secondary item
		item = new pbByNameListItem(master, c, gsm, 1, false);
		list->AddItem(item);
	}
}

void pbByNameView::fullListRefresh(void) {
	/// this is exactly as in pbView
	{
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
	///

	// clear sorted list, don't delete its contents
	if (byNameList->CountItems()>0) {
		if (!byNameList->IsEmpty())
			byNameList->MakeEmpty();
	}
	// go through phonebooks in phone (not callreg) and add pointers to one list
	BString n;
	struct pbSlot *sl;
	int j = gsm->listMemSlotPB->CountItems();
	for (int i=0;i<j;i++) {
		sl = (struct pbSlot*)gsm->listMemSlotPB->ItemAt(i);
		if (!sl->callreg) {
			// real stuff
			struct pbNum *pb;
			int k = sl->pb->CountItems();
			for (int l=0;l<k;l++) {
				pb = (struct pbNum*)sl->pb->ItemAt(l);
				byNameList->AddItem(pb);
			}
		}
	}
	// now sort list by name (just like in pbview to get uniques)
	byNameList->SortItems(&pbNumCompareByName);
}

void pbByNameView::Show(void) {
	BView::Show();
	BView::Flush();
	BView::Sync();
	if (list->CountItems()==0) {
		fullListRefresh();
	}
	fillList();
}

void pbByNameView::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case PBNDIAL:
			{	int i = list->CurrentSelection(0);

				if (i>=0) {
					struct pbNum *p = ((pbByNameListItem*)list->ItemAt(i))->Num();
					if ((p->type != GSM::PB_PHONE) && (p->type != GSM::PB_INTLPHONE)) {
						BAlert *err = new BAlert(APP_NAME, _("Selected item is not a phone number"), _("Ok"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
						err->Go();
					} else {
						gsm->dial(p->number.String());
					}
				}
				break;
			}
		case PBNREFRESH:
			fullListRefresh();
			fillList();
			break;
		case PBNLIST_INV:
			{	int i = list->CurrentSelection(0);
				if (i>=0)
					list->Expand((pbByNameListItem*)list->ItemAt(i));
				//
			}
			break;
		case PBNLIST_SEL:
			break;
		case PBNEXPORTVCF:
		case PBNEXPORTPPL:
			{	int i = list->CurrentSelection(0);
				if (i>=0) {
					if (Message->what == PBNEXPORTVCF)
						exportVCF(((pbByNameListItem*)list->ItemAt(i))->Master());
					else
						exportPeople(((pbByNameListItem*)list->ItemAt(i))->Master());
				}
			}
			break;
		default:
			mobileView::MessageReceived(Message);
			break;
	}
}

const char *types[] = { "WORK", "HOME", "VOICE", "CELL", "FAX", "PAGER", "EMAIL", "" };

void pbByNameView::exportVCF(int i) {
	bool cont;
	bool hadbday = false;
	bool haveadr = false;
	bool pref = false;
	int startidx = i;

	BString type;
	BString categories;
	BString nick;
	BString bday;
	BString adr2, adr, city, state, zip, country;

	union pbVal *v;
	pbNum *num = (pbNum*)byNameList->ItemAt(i);
	pbSlot *sl;
	BString tmp;

	v = (pbVal*)num->attr->ItemAt(1);
	BString name = v->text->String();

	BString vcard = "BEGIN:VCARD\nCLASS:PUBLIC\nVERSION:3.0\n";

	name.ReplaceAll(",","\\,");
	name.ReplaceAll(";","\\;");
	vcard += "FN:"; vcard += name; vcard += "\n";
	vcard += "N:"; vcard += name; vcard += ";;;;\n";

	// for each number
	cont = true;
	while (cont) {
		// for each attr
		type = "";
		categories = "";
		bday = "";
		pref = false;
		haveadr = false;
		adr = ""; adr2 = ""; city = ""; state = ""; zip = ""; country = "";

		num = (pbNum*)byNameList->ItemAt(i);
		sl = gsm->getPBSlot(num->slot.String());

		int k, l = sl->fields->CountItems();
		pbField *f;
		for (k=0;k<l;k++) {
			f = (pbField*)sl->fields->ItemAt(k);
			v = (pbVal*)num->attr->ItemAt(k);
			if (f->name.Compare(_("Primary number")) == 0) {
				pref = v->b;
			} else
			if (f->name.Compare(_("Type")) == 0) {
				if ((v->v<0) || (v->v > GSM::PK_MAILLIST))
					type = "";
				else
					type = types[v->v];
			} else
			if (f->name.Compare(_("Category")) == 0) {
				if (categories.Length()>0)
					categories += ",";
				categories << v->v;	// XXX translate into category name?
			} else
			if (f->name.Compare(_("Nick")) == 0) {
				if (v->text->Length()>0) {
					if (nick.Length()>0)
						nick += ",";
					tmp = v->text->String();
					tmp.ReplaceAll(",","\\,");
					tmp.ReplaceAll(";","\\;");
					nick += tmp;
				}
			} else
			if (f->name.Compare(_("Birthday (YYYY/MM/DD)")) == 0) {
				tmp = v->text->String();
				if (tmp.Length()>0) {
					bday = "BDAY:";
					tmp.ReplaceAll("/","-");
					bday += tmp;
					bday += "T00:00:00Z\n";
				}
			} else
			if (f->name.Compare(_("Address (2)")) == 0) {
				haveadr = haveadr || (v->text->Length() > 0);
				adr2 = v->text->String();
				adr2.ReplaceAll(",","\\,");
				adr2.ReplaceAll(";","\\;");
			} else
			if (f->name.Compare(_("Address")) == 0) {
				haveadr = haveadr || (v->text->Length() > 0);
				adr = v->text->String();
				adr.ReplaceAll(",","\\,");
				adr.ReplaceAll(";","\\;");
			} else
			if (f->name.Compare(_("City")) == 0) {
				haveadr = haveadr || (v->text->Length() > 0);
				city = v->text->String();
				city.ReplaceAll(",","\\,");
				city.ReplaceAll(";","\\;");
			} else
			if (f->name.Compare(_("State")) == 0) {
				haveadr = haveadr || (v->text->Length() > 0);
				state = v->text->String();
				state.ReplaceAll(",","\\,");
				state.ReplaceAll(";","\\;");
			} else
			if (f->name.Compare(_("Zip")) == 0) {
				haveadr = haveadr || (v->text->Length() > 0);
				zip = v->text->String();
				zip.ReplaceAll(",","\\,");
				zip.ReplaceAll(";","\\;");
			} else
			if (f->name.Compare(_("Country")) == 0) {
				haveadr = haveadr || (v->text->Length() > 0);
				country = v->text->String();
				country.ReplaceAll(",","\\,");
				country.ReplaceAll(";","\\;");			
			};
		}
		tmp = "";
		if (type.Compare("EMAIL") == 0) {
			// it's email
			tmp = "EMAIL";
			if (pref)
				tmp += ";TYPE=PREF";
		} else {
			// it's phone
			tmp = "TEL";
			if (type.Length()>0) {
				tmp += ";TYPE="; tmp += type;
				if (pref)
					tmp += ",PREF";
			} else if (pref)
					tmp += ";TYPE=PREF";
		}
		tmp += ":";
		tmp += ((pbVal*)num->attr->ItemAt(0))->text->String();
		vcard += tmp; vcard += "\n";
		if (!hadbday && (bday.Length() > 0)) {
			hadbday = true;
			vcard += bday;
		}
		if (haveadr) {
			BString tmp2;
			tmp = "";
			if (type.Compare("HOME") == 0)
				tmp += ";TYPE=HOME";
			else if (type.Compare("WORK") == 0)
				tmp += ";TYPE=WORK";
			if (pref) {
				if (tmp.Length()>0)
					tmp += ",PREF";
				else
					tmp += ";TYPE=PREF";
			}
			tmp2 = "ADR"; tmp2 += tmp;
			tmp = tmp2;
			tmp += ";";							// p.o. box
			tmp += adr2; tmp += ";";			// extended address
			tmp += adr;	tmp += ";";				// street address
			tmp += city; tmp += ";";			// locality
			tmp += state; tmp += ";";			// region
			tmp += zip; tmp += ";";				// postal code
			tmp += country;						// country
			tmp += "\n";
			vcard += tmp;
		}
		//
		i++;
		if (i == byNameList->CountItems())
			cont = false;
		else
			cont = name.Compare(((pbVal*)((pbNum*)byNameList->ItemAt(i))->attr->ItemAt(1))->text->String()) == 0;
	}
	if (nick.Length() > 0) {
		vcard += "NICKNAME:";
		vcard += nick;
		vcard += "\n";
	}
	if (categories.Length()>0) {
		vcard += "CATEGORIES:"; vcard += categories; vcard += "\n";
	}
	{
		struct tm *tm;
		time_t curtime;

		curtime = time(NULL);
		tm = localtime(&curtime);
		tm->tm_year += 1900;
		tm->tm_mon++;
		tmp = "REV:"; tmp << tm->tm_year; tmp += "-"; tmp << tm->tm_mon; tmp += "-"; tmp << tm->tm_mday;
		tmp += "\n";
		vcard += tmp;
	}
	{
		tmp = "UID:";
		tmp += gsm->getIMEI();
		num = (pbNum*)byNameList->ItemAt(startidx);
		tmp += "-"; tmp += num->slot.String();
		tmp += "-"; tmp << num->id;
		tmp += "\n";
		vcard += tmp;
	}
	vcard += "END:VCARD\n\n";
	printf("%s",vcard.String());

	// append to file
	BFile f;
	if (f.SetTo(VCFPATH VCFFILE, B_WRITE_ONLY|B_CREATE_FILE|B_OPEN_AT_END) != B_OK) {
		tmp = _("There was error and vCard file was not saved.");
		tmp += "\n";
		tmp += VCFPATH;
		tmp += VCFFILE;
		BAlert *err = new BAlert(APP_NAME, tmp.String(), _("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_INFO_ALERT);
		err->Go();
		printf("error!\n");
		return;
	}
	f.Write(vcard.String(), vcard.Length());
	f.Unset();
	tmp = _("Contact data has been added to vCard file");
	tmp += "\n";
	tmp += VCFPATH; tmp += VCFFILE;
	BAlert *a = new BAlert(APP_NAME, tmp.String(), _("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_INFO_ALERT);
	a->Go();
}

/// this is from mrpeeps
//file attributes
#define PERSON_FILE_TYPE	"application/x-person"

#define PERSON_NAME			"META:name"
#define PERSON_FIRSTNAME	"META:firstname"
#define PERSON_LASTNAME		"META:lastname"
#define PERSON_TITLE		"META:title"
#define PERSON_NICKNAME		"META:nickname"
#define PERSON_EMAIL		"META:email"
#define PERSON_URL			"META:url"
#define PERSON_HOME_PHONE	"META:hphone"
#define PERSON_WORK_PHONE	"META:wphone"
#define PERSON_CELL_PHONE	"META:cell"
#define PERSON_FAX			"META:fax"
#define PERSON_ADDRESS		"META:address"
#define PERSON_ADDRESS2		"META:address2"
#define PERSON_CITY			"META:city"
#define PERSON_STATE		"META:state"
#define PERSON_ZIP			"META:zip"
#define PERSON_COUNTRY		"META:country"
#define PERSON_BIRTHDAY		"META:birthday"
#define PERSON_ANNIVERSARY	"META:anniversary"
#define PERSON_GROUP		"META:group"
#define PERSON_SPOUSE		"META:spouse"
#define PERSON_CHILDREN		"META:children"
#define PERSON_ASSISTANT	"META:assist"
#define PERSON_ASSISTANT_PHONE	"META:assistphone"
#define PERSON_MSN			"META:msn"
#define PERSON_JABBER		"META:jabber"
#define PERSON_ICQ			"META:icq"
#define PERSON_ICQ_UIN		"META:icquin"
#define PERSON_YAHOO		"META:yahoo"
#define PERSON_AIM			"META:aim"
#define PERSON_EMAIL3		"META:email3"
#define PERSON_EMAIL4		"META:email4"
#define PERSON_EMAIL5		"META:email5"

#define PERSON_OTHER_URL	"META:url3"
#define PERSON_NOTES		"META:notes"

#define PERSON_WORK_EMAIL	"META:email2"
#define PERSON_WORK_CELL	"META:wcphone"
#define PERSON_WORK_COUNTRY	"META:wcountry"
#define PERSON_WORK_CITY	"META:wcity"
#define PERSON_WORK_ADDRESS	"META:waddress"
#define PERSON_WORK_URL		"META:url2"
#define PERSON_PAGER		"META:pager"
#define PERSON_WORK_FAX		"META:wfax"
#define PERSON_WORK_ZIP		"META:wzip"
#define PERSON_WORK_STATE	"META:wstate"
#define PERSON_WORK_ADDRESS2	"META:waddress2"
#define PERSON_POSITION		"META:position"
#define PERSON_COMPANY		"META:company"
#define PERSON_PHOTO		"META:photo"

PeopleFile::PeopleFile(BMessage *p) {
	if (p->what == MSG_PERSON)
		person = p;
	else
		person = NULL;
}

PeopleFile::~PeopleFile() {

}

int PeopleFile::Save(const char *path) {

	if (!person)
		return -1;	// invalid

	BString tmp;
	BString fname(path);
	fname += "/";

	if (create_directory(path, 0777) != B_OK) {
		tmp = _("Can't create directory for People files export");
		tmp += "\n"; tmp += path;
		BAlert *a = new BAlert(APP_NAME, tmp.String(), _("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_INFO_ALERT);
		a->Go();
		return -1;
	}

	tmp = getMsgItem("name");
	if (tmp.Length()<1) {
		tmp = _("You can't export record without a name");
		BAlert *a = new BAlert(APP_NAME, tmp.String(), _("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_INFO_ALERT);
		a->Go();
		return -1;
	}
	fname += tmp;

	// create file
	BFile f(fname.String(),B_WRITE_ONLY|B_CREATE_FILE);
	BNodeInfo ninfo(&f);
	ninfo.SetType(PERSON_FILE_TYPE);

	// save attributes
	tmp = getMsgItem("name"); if (tmp.Length() > 0)
	f.WriteAttr(PERSON_NAME, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
	tmp = getMsgItem("nickname"); if (tmp.Length() > 0)
	f.WriteAttr(PERSON_NICKNAME, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
	tmp = getMsgItem("birthday"); if (tmp.Length() > 0)
	f.WriteAttr(PERSON_BIRTHDAY, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
	tmp = getMsgItem("category"); if (tmp.Length() > 0)
	f.WriteAttr(PERSON_GROUP, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);

	int32 t = -1;
	// save address - either work or any other
	if ((person->FindInt32("num_type",&t) == B_OK) && (t == GSM::PK_WORK)) {
		tmp = getMsgItem("address"); if (tmp.Length() > 0)
		f.WriteAttr(PERSON_WORK_ADDRESS, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
		tmp = getMsgItem("address2"); if (tmp.Length() > 0)
		f.WriteAttr(PERSON_WORK_ADDRESS2, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
		tmp = getMsgItem("city"); if (tmp.Length() > 0)
		f.WriteAttr(PERSON_WORK_CITY, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
		tmp = getMsgItem("state"); if (tmp.Length() > 0)
		f.WriteAttr(PERSON_WORK_STATE, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
		tmp = getMsgItem("zip"); if (tmp.Length() > 0)
		f.WriteAttr(PERSON_WORK_ZIP, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
		tmp = getMsgItem("country"); if (tmp.Length() > 0)
		f.WriteAttr(PERSON_WORK_COUNTRY, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
	} else {
		tmp = getMsgItem("address"); if (tmp.Length() > 0)
		f.WriteAttr(PERSON_ADDRESS, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
		tmp = getMsgItem("address2"); if (tmp.Length() > 0)	// peeps attr
		f.WriteAttr(PERSON_ADDRESS2, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
		tmp = getMsgItem("city"); if (tmp.Length() > 0)
		f.WriteAttr(PERSON_CITY, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
		tmp = getMsgItem("state"); if (tmp.Length() > 0)
		f.WriteAttr(PERSON_STATE, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
		tmp = getMsgItem("zip"); if (tmp.Length() > 0)
		f.WriteAttr(PERSON_ZIP, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
		tmp = getMsgItem("country"); if (tmp.Length() > 0)
		f.WriteAttr(PERSON_COUNTRY, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
	}

	tmp = getMsgItem("number");
	if (tmp.Length() > 0) {
		int32 t = -1;
		char buf[100];
		int state = person->FindInt32("num_type", &t);
		// only save the first found phone if it was not present, and current type is not email or there is no type
		if ((state != B_OK) || ((state == B_OK) && (t != GSM::PK_EMAIL)) &&
			(f.ReadAttr(PERSON_HOME_PHONE, B_STRING_TYPE, 0, buf, 10) == B_ENTRY_NOT_FOUND)) {
			printf("no home set, not email or no type, writing default");
			f.WriteAttr(PERSON_HOME_PHONE, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);	
		}
		if (state == B_OK) {
			switch (t) {
				case GSM::PK_WORK:
					f.WriteAttr(PERSON_WORK_PHONE, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
					break;
				case GSM::PK_MOBILE:
					f.WriteAttr(PERSON_CELL_PHONE, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
					break;
				case GSM::PK_MAIN:
				case GSM::PK_HOME:
					f.WriteAttr(PERSON_HOME_PHONE, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
					break;
				case GSM::PK_FAX:
					f.WriteAttr(PERSON_FAX, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
					break;
				case GSM::PK_PAGER:
					f.WriteAttr(PERSON_PAGER, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
					break;
				case GSM::PK_EMAIL:
					f.WriteAttr(PERSON_EMAIL, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);
					break;
				default:
//					printf("Unknown type:%i for %s\n", t, tmp.String());
					break;
			}
		} else {
			// no type information -- must be phone (home)
			f.WriteAttr(PERSON_HOME_PHONE, B_STRING_TYPE, 0, tmp.String(), tmp.Length()+1);	
		}
	}
	f.Unset();

	return B_OK;
}

const char *PeopleFile::getMsgItem(const char *item) {
	static const char *s;
	if (person->FindString(item, &s) != B_OK)
		return NULL;
	else
		return s;
}

void pbByNameView::exportPeople(int i) {
	bool cont = false;

	// pass msg for each one so it will overwrite/update itself
	PeopleFile *p;
	BMessage *msg;

	pbSlot *sl;
	pbNum *num = (pbNum*)byNameList->ItemAt(i);
	union pbVal *v = (pbVal*)num->attr->ItemAt(1);
	BString name = v->text->String();
	BString tmp;

	// for each number
	cont = true;
	while (cont) {
		num = (pbNum*)byNameList->ItemAt(i);
		sl = gsm->getPBSlot(num->slot.String());

		// actual stuff
		msg = new BMessage(MSG_PERSON);
		msg->AddString("name", name.String());
		msg->AddString("number", ((pbVal*)num->attr->ItemAt(0))->text->String());

		int k, l = sl->fields->CountItems();
		pbField *f;
		for (k=0;k<l;k++) {
			f = (pbField*)sl->fields->ItemAt(k);
			v = (pbVal*)num->attr->ItemAt(k);
			if (f->name.Compare(_("Type")) == 0) {
				msg->AddInt32("num_type", v->v);
			} else if (f->name.Compare(_("Category")) == 0) {
				tmp = ""; tmp << v->v;
				msg->AddString("category", tmp.String());
			} else if (f->name.Compare(_("Nick")) == 0) {
				if (v->text->Length() > 0)
					msg->AddString("nickname", v->text->String());
			} else if (f->name.Compare(_("Birthday (YYYY/MM/DD)")) == 0) {
				tmp = v->text->String();
				if (tmp.Length() == 10) {
					tmp.ReplaceAll("/","-");
					msg->AddString("birthday", tmp.String());
				}
			} else if (f->name.Compare(_("Country")) == 0) {
				if (v->text->Length() > 0)
					msg->AddString("country", v->text->String());
			} else if (f->name.Compare(_("Zip")) == 0) {
				if (v->text->Length() > 0)
					msg->AddString("zip", v->text->String());
			} else if (f->name.Compare(_("State")) == 0) {
				if (v->text->Length() > 0)
					msg->AddString("state", v->text->String());
			} else if (f->name.Compare(_("City")) == 0) {
				if (v->text->Length() > 0)
					msg->AddString("city", v->text->String());
			} else if (f->name.Compare(_("Address")) == 0) {
				if (v->text->Length() > 0)
					msg->AddString("address", v->text->String());
			} else if (f->name.Compare(_("Address (2)")) == 0) {
				if (v->text->Length() > 0)
					msg->AddString("address2", v->text->String());
			}
		}
		msg->PrintToStream();
		p = new PeopleFile(msg);
		if (p->Save(people_path) != B_OK) {
			delete p;
			delete msg;
			return;
		}
		delete p;
		delete msg;
		i++;
		if (i == byNameList->CountItems())
			cont = false;
		else
			cont = name.Compare(((pbVal*)((pbNum*)byNameList->ItemAt(i))->attr->ItemAt(1))->text->String()) == 0;
	}
}
