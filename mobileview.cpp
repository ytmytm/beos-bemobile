
#include <Application.h>
#include <Bitmap.h>
#include <Font.h>
#include <StringView.h>
#include <Resources.h>

#include "mobileview.h"

#define CAPTION_SIZE 20.0

mobileView::mobileView(BRect r, const char *name, GSM *g = NULL) : BView(r, name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW|B_PULSE_NEEDED) {
	// initialize data
	SetDevice(g);
	//
	this->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	BRect r = this->Bounds();
	r.InsetBy(5,5);
	r.bottom = r.top + CAPTION_SIZE;
	// my top
	top = r.bottom + 5;
	this->AddChild(caption = new BStringView(r,"viewCaption",NULL,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP));
	BFont myfont(be_plain_font); myfont.SetSize(CAPTION_SIZE);
	caption->SetFont(&myfont,B_FONT_SIZE);
}

void mobileView::SetDevice(GSM *g) {
	gsm = g;
}

void mobileView::MessageReceived(BMessage *Message) {
// may cause crash (e.g. using mouse roller on the view)
//	BView::MessageReceived(Message);
}

BRect mobileView::MyBounds(void) {
	static BRect r;
	r = this->Bounds();
	r.top = top;
	return r;
}

////////////

#include <stdio.h>

BBitmap *getIconFromResources(const char *icon_resname) {
	BBitmap *bmp = NULL;
	BResources *res = be_app->AppResources();
	if (res->HasResource('BBMP',icon_resname)) {
		printf("has resource bitmap [%s]\n",icon_resname);
		BMessage msg;
		size_t len;
		char *buf;
		buf = (char *)res->LoadResource('BBMP', icon_resname, &len);
//					printf("loaded,len=%i\n",len);
		msg.Unflatten(buf);
		bmp = new BBitmap(&msg);
	}
	return bmp;
}
