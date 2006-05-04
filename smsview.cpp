//
// - btextview prv inside scrollview
// - parse date into nice format (dd/mm/yyyy hh:mm)
// - implement delete
// - make workview derivative from mobileview (has setgsm)
// - visual difference between read and unread messages (icon in col0? char?)

#include <Box.h>
#include <Button.h>
#include <Font.h>
#include <StatusBar.h>
#include <StringView.h>
#include <TextView.h>
#include "CLVEasyItem.h"
#include "ColumnListView.h"
#include "globals.h"
#include "smsview.h"

#include <stdio.h>

const uint32	SMSLIST_INV	= 'SL00';
const uint32	SMSLIST_SEL = 'SL01';
const uint32	SMSREFRESH	= 'SB00';
const uint32	SMSDELETE	= 'SB01';
const uint32	SMSNEW		= 'SB02';

smsView::smsView(BRect r) : mobileView(r, "smsView") {
	caption->SetText(_("SMS summary"));

	// if selected SMS - show both inbox/outbox on the list
	// if inbox -> inbox, outbox->outbox
	// show status bar only when needed

	BFont font(be_plain_font);
	float maxw, totalw = 0;
	BRect r = this->MyBounds();
	r.InsetBy(10,15);
	r.right -= 16;
	r.bottom = r.top + r.Height()/2 - 50;

	// add column list
	CLVContainerView *containerView;
	list = new ColumnListView(r, &containerView, NULL, B_FOLLOW_TOP_BOTTOM|B_FOLLOW_LEFT_RIGHT,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST, false, true, true, true,
		B_FANCY_BORDER);
	maxw = font.StringWidth("+999999999") + 20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Sender"), maxw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	totalw += maxw;
	list->AddColumn(new CLVColumn(_("Receiver"), maxw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	maxw = font.StringWidth("8888/88/88, 88:88") + 20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Date"), maxw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->AddColumn(new CLVColumn(_("Text"), r.right-16-totalw, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->SetSortFunction(CLVEasyItem::CompareItems);
	this->AddChild(containerView);
	list->SetInvocationMessage(new BMessage(SMSLIST_INV));
	list->SetSelectionMessage(new BMessage(SMSLIST_SEL));

	r.right += 16;
	r.OffsetBy(0,r.Height()+25);
	BBox *box;
	this->AddChild(box = new BBox(r,"smsPrvBox"));
	box->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
	BRect s = box->Bounds(); s.InsetBy(10,10);
	BRect t; t.top = t.left = 0; t.right = s.Width(); t.bottom = s.Height();
	prv = new BTextView(s, "smsPrv", t, B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
	box->AddChild(prv);
	prv->MakeEditable(false);
	prv->SetStylable(true);

	r.OffsetBy(0,r.Height()+5);
	r.bottom = r.top + font.Size()*2;
	this->AddChild(progress = new BStatusBar(r, "smsStatusBar"));
	progress->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
	progress->SetMaxValue(100);
	progress->Hide();

	r = this->MyBounds();
	r.InsetBy(20,20);
	s = r; s.top = s.bottom - font.Size()*2; s.right = s.left + font.StringWidth("MMMMMMMMMM")+40;
	this->AddChild(refresh = new BButton(s, "smsRefresh", _("Refresh"), new BMessage(SMSREFRESH), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM));
	t = r; t.top = s.top; t.bottom = s.bottom; t.right = r.right; t.left = t.right - (font.StringWidth("MMMMMMMMMM")+40);
	this->AddChild(del = new BButton(t, "smsDelete", _("Delete"), new BMessage(SMSDELETE), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM));
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

void smsView::fillList(int mode = SMS_ALLBOXEN) {
	BString slot;
	BString right;
	struct SMS *sms;
	int i, msgnum;
	float delta;

	clearList();
	progress->Reset();
	progress->Show();

	switch (mode) {
		case SMS_INBOX:
			slot = "IM";
			break;
		case SMS_OUTBOX:
			slot = "OM";
			break;
		case SMS_ALLBOXEN:
		default:
			slot = "MT";
			break;
	}
	progress->Update(0, _("Checking number of messagess..."));
	msgnum = gsm->changeSMSMemSlot(slot.String());

	if (msgnum > 0) {
		right = ""; right << msgnum;
		delta = 100/msgnum;
		gsm->getSMSList(slot.String());
		for (i=0;i<gsm->SMSList->CountItems();i++) {
			sms = (struct SMS*)gsm->SMSList->ItemAt(i);
			gsm->getSMSContent(sms);
			list->AddItem(new smsListItem(sms));
			progress->Update(delta, "0", right.String());
		}
	}
	progress->Hide();
}

void smsView::updatePreview(struct SMS *sms) {
	BString tmp, tmp2;
	int textlen;
	static BFont font(be_plain_font);
	static BFont bfont(be_bold_font);
	static rgb_color *red = NULL, *blue = NULL, *green = NULL, *black = NULL;

	if (!red) {
		red = new rgb_color; red->red = 255; red->blue = red->green = 0;
		blue = new rgb_color; blue->blue = 255; blue->red = blue->green = 0;	
		green = new rgb_color; green->green = 255; green->red = green->blue = 0;
		black = new rgb_color; black->red = black->green = black->blue = 0;
	}

	switch (sms->type) {
		case GSM::STO_SENT:
			tmp = _("Message sent");
			tmp2 = _("To:");
			break;
		case GSM::STO_UNSENT:
			tmp = _("Message not yet sent");
			tmp2 = _("To:");
			break;
		case GSM::REC_READ:
			tmp = _("Message received");
			tmp2 = _("From");
			break;
		case GSM::REC_UNREAD:
			tmp = _("Message unread");
			tmp2 = _("From:");
			break;
		default:
			tmp = _("Message");
			break;
	}
	tmp += "\n"; tmp += tmp2; tmp += " ";

	prv->SetText("");
	textlen = 0;
	bfont.SetSize(14.0);
	prv->SetFontAndColor(&bfont,B_FONT_ALL,blue);
	prv->Insert(textlen,tmp.String(),tmp.Length());
	textlen += tmp.Length();
	prv->SetFontAndColor(&bfont,B_FONT_ALL,red);
	prv->Insert(textlen,sms->number.String(),sms->number.Length());
	textlen += sms->number.Length();
	prv->Insert(textlen,"\n",1);
	textlen++;
	if (sms->date.Length() > 0) {
		prv->SetFontAndColor(&bfont,B_FONT_ALL,blue);
		tmp = _("Date:"); tmp += " ";
		prv->Insert(textlen,tmp.String(),tmp.Length());
		textlen += tmp.Length();
		prv->SetFontAndColor(&bfont,B_FONT_ALL,red);
		prv->Insert(textlen,sms->date.String(),sms->date.Length());
		textlen += sms->date.Length();
	}
	prv->Insert(textlen,"\n\n",2);
	textlen += 2;
	font.SetSize(12.0);
	prv->SetFontAndColor(&font,B_FONT_ALL,black);
	prv->Insert(textlen,sms->msg.String(),sms->msg.Length());
	textlen += sms->msg.Length();
	prv->Insert(textlen,"\n",1);
}

void smsView::Show(void) {
printf("show!\n");
	BView::Show();
	if (list->CountItems()==0)
		fillList();	// XXX what's the mode here?
}

void smsView::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case SMSREFRESH:
			fillList();
			break;
		case SMSDELETE:
			// BAlert ask:
			// XXX implement!
			break;
		case SMSLIST_INV:
		case SMSLIST_SEL:
			{	int i = list->CurrentSelection(0);
				if (i>=0) {
					struct SMS *sms = ((smsListItem*)list->ItemAt(list->CurrentSelection(0)))->Msg();
					if (sms)
						updatePreview(sms);

				} else {
					prv->SetText("");
				}
				break;
			}
		default:
			break;
	}
}
