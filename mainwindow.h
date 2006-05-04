
#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

	#include <Window.h>
	#include "globals.h"

	class BButton;
	class BMenu;
	class BMenuBar;
	class BMenuItem;
	class GSM;
	class workView;

	class BeMobileMainWindow : public BWindow {
		public:
			BeMobileMainWindow(const char *windowTitle);
			~BeMobileMainWindow();
//			virtual void FrameResized(float width, float height);
			void MessageReceived(BMessage *Message);
//			virtual void DispatchMessage(BMessage *message, BHandler *handler);
			bool QuitRequested();
//			virtual void RefsReceived(BMessage *Message);
			void DoAbout(void);
		private:
			// device
			GSM *gsm;
			//
			workView *mainView;
			// main view widgets
			BMenuBar *menuBar;
			BMenu *menu;
	};

#endif
