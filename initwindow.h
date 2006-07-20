
#ifndef _INITWINDOW_H
#define _INITWINDOW_H

#include <String.h>
#include <Window.h>

	class BButton;
	class BCheckBox;
	class BSerialPort;
	class BStringView;
	class BTextControl;

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
			BMenuItem *parItems[3], *datItems[2], *stoItems[2], *floItems[4], *bauItems[7];
			BMenuItem *dtrItem, *rtsItem;
			BTextControl *initStr;
			// results
			int nports;
			bool log, term;
			BString device;
			// serial settings
			int parity;
			int databits;
			int stopbits;
			int flowcontrol;
			int baudrate;
			bool dtr;
			bool rts;
	};

#endif
