
#include <Alert.h>
#include <Box.h>
#include <Button.h>
#include <Font.h>
#include <ScrollView.h>
#include <StatusBar.h>
#include <StringView.h>
#include <TextView.h>
#include "ColumnListView.h"
#include "globals.h"
#include "smsboxview.h"

#include <stdio.h>

const uint32	SMSLIST_INV		= 'SB00';
const uint32	SMSLIST_SEL 	= 'SB01';
const uint32	SMSREFRESH		= 'SB02';
const uint32	SMSDELETE		= 'SB03';

smsBoxView::smsBoxView(BRect r, const char *slot) : mobileView(r, "smsBoxView") {

	memSlot = slot;

	BFont font(be_plain_font);
	float maxw, totalw = 0;
	BRect r = this->MyBounds();
	r.InsetBy(10,15);
	r.right -= B_V_SCROLL_BAR_WIDTH;
	r.bottom = r.top + r.Height()/2 - 50;

	// add column list
	int flags = CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE;
	CLVContainerView *containerView;
	list = new ColumnListView(r, &containerView, NULL, B_FOLLOW_TOP_BOTTOM|B_FOLLOW_LEFT_RIGHT,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST, false, true, true, true,
		B_FANCY_BORDER);
	maxw = font.StringWidth("XX"); totalw += maxw;
	list->AddColumn(new CLVColumn("#", maxw, flags));
	maxw = font.StringWidth("+999999999") + 20; totalw += maxw;
	BString tmp;
	int tflags = flags;
	if (memSlot == "IM") {
		tmp = _("Sender");
	} else
	if ((memSlot == "OM") || (memSlot == "DM")) {
		tmp = _("Receiver");
	} else
	if (memSlot == "BM") {
		tmp = "";
		tflags |= CLV_HIDDEN;
		totalw -= maxw;
	} else {
		tmp = _("Number");
	}
	list->AddColumn(new CLVColumn(tmp.String(), maxw, tflags));
	maxw = font.StringWidth("8888/88/88, 88:88") + 20; totalw += maxw;
	list->AddColumn(new CLVColumn(_("Date"), maxw, flags));
	list->AddColumn(new CLVColumn(_("Text"), r.right-B_V_SCROLL_BAR_WIDTH-totalw, flags));
	list->SetSortFunction(CLVEasyItem::CompareItems);
	this->AddChild(containerView);
	list->SetInvocationMessage(new BMessage(SMSLIST_INV));
	list->SetSelectionMessage(new BMessage(SMSLIST_SEL));

	r.right += 16;
	r.OffsetBy(0,r.Height()+25);
	BBox *box;
	this->AddChild(box = new BBox(r,"smsPrvBox"));
	box->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
	BRect s = box->Bounds(); s.InsetBy(10,10); s.right -= B_V_SCROLL_BAR_WIDTH;
	BRect t; t.top = t.left = 0; t.right = s.Width(); t.bottom = s.Height();
	prv = new BTextView(s, "smsPrv", t, B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
	prv->MakeEditable(false);
	prv->SetStylable(true);
	box->AddChild(new BScrollView("smsPrvScroll",prv,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM,0,false,true));

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

void smsBoxView::clearList(void) {
	if (list->CountItems()>0) {
		CLVEasyItem *anItem;
		for (int i=0; (anItem=(smsBoxListItem*)list->ItemAt(i)); i++)
			delete anItem;
		if (!list->IsEmpty())
			list->MakeEmpty();
	}
}

void smsBoxView::fillList(void) {
	struct memSlotSMS *sl = gsm->getSMSSlot(memSlot.String());
	struct SMS *sms;

	clearList();

	int j = sl->msg->CountItems();
	for (int i=0;i<j;i++) {
		sms = (struct SMS*)sl->msg->ItemAt(i);
		list->AddItem(new smsBoxListItem(sms));
	}
}

void smsBoxView::fullListRefresh(void) {
	BString right;
	struct memSlotSMS *sl;
	struct SMS *sms;
	int i, msgnum;
	float delta;

	progress->Reset();
	progress->Show();

	progress->Update(0, _("Checking number of messages..."));
	msgnum = gsm->changeSMSMemSlot(memSlot.String());
	sl = gsm->getSMSSlot(memSlot.String());

	if (msgnum > 0) {
		right = ""; right << msgnum;
		delta = 100/msgnum;
		gsm->getSMSList(memSlot.String());
		int j = sl->msg->CountItems();
		for (i=0;i<j;i++) {
			sms = (struct SMS*)sl->msg->ItemAt(i);
			gsm->getSMSContent(sms);
			progress->Update(delta, "0", right.String());
		}
	}
	progress->Hide();
	gsm->changeSMSMemSlot("MT");	// XXX configurable?
}

void smsBoxView::updatePreview(struct SMS *sms) {
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
			tmp2 = _("From:");
			break;
		case GSM::REC_UNREAD:
			tmp = _("Message unread");
			tmp2 = _("From:");
			break;
		default:
			tmp = _("Message");
			tmp2 = _("Number:");
			break;
	}
	tmp += "\n";
	tmp2 += " ";

	prv->SetText("");
	textlen = 0;
	bfont.SetSize(14.0);
	prv->SetFontAndColor(&bfont,B_FONT_ALL,blue);
	prv->Insert(textlen,tmp.String(),tmp.Length());
	textlen += tmp.Length();
	if (sms->number.Length() > 0) {
		prv->Insert(textlen,tmp2.String(),tmp2.Length());
		textlen += tmp2.Length();
		prv->SetFontAndColor(&bfont,B_FONT_ALL,red);
		prv->Insert(textlen,sms->number.String(),sms->number.Length());
		textlen += sms->number.Length();
	}
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
	BString msg = sms->msg.String();
	msg.RemoveAll("\r");
	prv->Insert(textlen,msg.String(),msg.Length());
	textlen += msg.Length();
	prv->Insert(textlen,"\n",1);
}

void smsBoxView::Show(void) {
	BView::Show();
	BView::Flush();
	BView::Sync();
	if (list->CountItems()==0) {
		if (gsm->getSMSSlot(memSlot.String())->msg->CountItems() == 0)
			fullListRefresh();
	}
	fillList();
}

void smsBoxView::SetDevice(GSM *g) {
	mobileView::SetDevice(g);
	BString cap = caption->Text();
	if (cap.Length() == 0) {
		cap = "SMS "; cap += (gsm->getSMSSlot(memSlot.String()))->name.String();
		caption->SetText(cap.String());
	}
}

void smsBoxView::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case SMSREFRESH:
			fullListRefresh();
			fillList();
			break;
		case SMSDELETE:
			{	int i = list->CurrentSelection(0);

				if (i>=0) {
					BAlert *ask = new BAlert(APP_NAME, _("Do you really want to delete this message?"), _("Yes"), _("No"), NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
					if (ask->Go() == 0) {
						struct SMS *sms = ((smsBoxListItem*)list->ItemAt(list->CurrentSelection(0)))->Msg();
						if (gsm->removeSMS(sms) == 0)
							list->RemoveItem(i);
					}
				}
				break;
			}
		case SMSLIST_INV:
		case SMSLIST_SEL:
			{	int i = list->CurrentSelection(0);
				if (i>=0) {
					struct SMS *sms = ((smsBoxListItem*)list->ItemAt(list->CurrentSelection(0)))->Msg();
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
