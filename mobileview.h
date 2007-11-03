#ifndef _MOBILEVIEW_H
#define _MOBILEVIEW_H

#include <Rect.h>
#include <View.h>

class BStringView;
class GSM;

BBitmap *getIconFromResources(const char *icon_resname);

class mobileView : public BView {
	public:
		mobileView(BRect r, const char *name, GSM *g = NULL);
		~mobileView() { };
		virtual void MessageReceived(BMessage *Message);
		virtual void SetDevice(GSM *g);
	protected:
		BRect MyBounds(void);
		GSM *gsm;
		BStringView *caption;
	private:
		float top;
};

#endif
