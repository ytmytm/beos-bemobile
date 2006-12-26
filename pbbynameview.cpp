
//
// XXX:it is ugly to recognize attribute type by its label
//

#include <Alert.h>
#include <Button.h>
#include <File.h>
#include <Font.h>
#include <StatusBar.h>
#include <StringView.h>
#include "ColumnListView.h"
#include "globals.h"
#include "pbbynameview.h"

#include <stdio.h>
#include <time.h>

#define VCFPATH "/boot/home/people/"
#define VCFFILE "people.vcf"

const uint32	PBNLIST_INV	= 'PBN0';
const uint32	PBNLIST_SEL	= 'PBN1';
const uint32	PBNREFRESH	= 'PBN2';
const uint32	PBNDIAL		= 'PBN3';
const uint32	PBNEXPORTVCF = 'PBN4';

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
	maxw = font.StringWidth(_("M"))*17+20; totalw += maxw;
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
	this->AddChild(exportvcf = new BButton(s, "pbnExportVCF", _("Export vCard"), new BMessage(PBNEXPORTVCF), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));
	s.OffsetBy(len*3,0);
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
			{	int i = list->CurrentSelection(0);
				if (i>=0) {
					exportVCF(((pbByNameListItem*)list->ItemAt(i))->Master());
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
			if (f->name.Compare(_("Birthday (MM-DD-YYYY)")) == 0) {
				tmp = v->text->String();
				if (tmp.Length()>0) {
					bday = "BDAY:";
					bday += tmp[6]; bday += tmp[7]; bday += tmp[8]; bday += tmp[9]; bday += "-";
					bday += tmp[0]; bday += tmp[1]; bday += "-";
					bday += tmp[3]; bday += tmp[4];
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
		// XXX alert w/ error
		printf("error!\n");
		return;
	}
	f.Write(vcard.String(), vcard.Length());
	f.Unset();
}
