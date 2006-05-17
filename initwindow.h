
#ifndef _INITWINDOW_H
#define _INITWINDOW_H

#include <String.h>
#include <Window.h>

	class BButton;
	class BCheckBox;
	class BSerialPort;
	class BStringView;

	class initWindow : public BWindow {
		public:
			initWindow(const char *name);
			~initWindow();
			void MessageReceived(BMessage *Message);
			bool QuitRequested();
		private:
			void DoFinish(void);
			// ui widgets
			BButton *but_quit, *but_ok;
			BCheckBox *cb_log, *cb_term;
			BMenuItem **devitems;
			// results
			int nports;
			bool log, term;
			BString device;
	};

#endif
