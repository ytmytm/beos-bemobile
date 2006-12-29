
#ifndef _PBBYNAMEVIEW_H
#define _PBBYNAMEVIEW_H

#include "CLVEasyItem.h"
#include "gsm.h"
#include "mobileview.h"

class BButton;
class BList;
class BStatusBar;
class ColumnListView;

class pbByNameView : public mobileView {
	public:
		pbByNameView(BRect r);
		~pbByNameView() { };
		void Show(void);
		void MessageReceived(BMessage *Message);
	private:
		void clearList(void);
		void fillList(void);
		void fullListRefresh(void);

		void exportVCF(int i);
		void exportPeople(int i);

		ColumnListView *list;
		BStatusBar *progress;
		BButton *refresh, *exportvcf, *exportppl, *dial;

		BList *byNameList;
};

class pbByNameListItem : public CLVEasyItem {
	public:
		pbByNameListItem(int master, struct pbNum *num, GSM *g, int level = 0, bool superitem = false) : CLVEasyItem(
			level, superitem, false, 20.0) {
			fNum = num;
			gsm = g;
			RefreshData(superitem);
			fSuperItem = superitem;
			fMaster = master;
		}
		void RefreshData(bool superitem = false) {
			BString *tmp;
			if (superitem) {
				// insert name
				tmp = ((union pbVal*)fNum->attr->ItemAt(1))->text;
				SetColumnContent(1, tmp->String());
			} else {
				// insert number and other attrs
				tmp = ((union pbVal*)fNum->attr->ItemAt(0))->text;
				SetColumnContent(2, tmp->String());
				// set memory slot name on column 1
				SetColumnContent(1, gsm->getPBMemSlotName(fNum->slot.String()));
				// set number type on column 1 (if present! overwrite)
				struct pbSlot *sl = gsm->getPBSlot(fNum->slot.String());
				if (sl->has_phtype) {
					// XXX offset 2 -> motorola!
					struct pbField *pf = (struct pbField*)sl->fields->ItemAt(2);
					union pbVal* v = (union pbVal*)fNum->attr->ItemAt(2);
					if ((pf)&&(v)) {
						// XXX offset 4 -> motorola!
						if ((pf->offset == 4)&&(pf->type == GSM::PF_COMBO)) {
							// find combo value
							int l = pf->cb->CountItems();
							struct pbCombo *pc;
							for (int k=0; k<l; k++) {
								pc = (struct pbCombo*)pf->cb->ItemAt(k);
								if (pc->v == v->v) {
									SetColumnContent(1,pc->text.String());
									break;
								}
							}
						}
					}
				}
			}
		}
		struct pbNum *Num(void) { return fNum; };
		bool SuperItem(void) { return fSuperItem; };
		int Master(void) { return fMaster; } ;
	private:
		struct pbNum *fNum;
		GSM *gsm;
		bool fSuperItem;
		int fMaster;
};

class PeopleFile {
	public:
		PeopleFile(BMessage *p);
		~PeopleFile();
		int Save(const char *path, bool setHomeNumber = false);
	private:
		BMessage *person;
		const char *getMsgItem(const char *item);
// name, address, city, state, zip, country, hphone, wphone;
// fax, group, nickname, email, title, address2, cell, birthday;
// pager, waddress, waddress2, wcity, wcountry, wcphone, wfax, wstate, wzip;
};

#endif
