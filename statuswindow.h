
#ifndef _STATUSWINDOW_H
#define _STATUSWINDOW_H

	#include <Window.h>
	#include "globals.h"

	class BButton;
	class BMenu;
	class BMenuBar;
	class BMenuItem;
	class GSM;
	class statusView;

	class StatusWindow : public BWindow {
		public:
			StatusWindow(const char *windowTitle, GSM *g = NULL);
			~StatusWindow();
//			virtual void FrameResized(float width, float height);
			void MessageReceived(BMessage *Message);
//			virtual void DispatchMessage(BMessage *message, BHandler *handler);
			bool QuitRequested();
//			virtual void RefsReceived(BMessage *Message);
			void DoAbout(void);
		private:
			void SetDevice(GSM *g);
			// device
			GSM *gsm;
			//
			statusView *mainView;
			// main view widgets
			BMenuBar *menuBar;
			BMenu *menu;
	};

#endif
