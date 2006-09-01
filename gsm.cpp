//
// regexy! najwygodniej
// - sendcommand:
// 		- regexy w sendcommand do wykrycia statusu
//
// dodać usuwanie listy fields ze slotów i attr z pbnum przy refresh (?)
// przepisać pbField jako klasę i niech samo się troszczy o walidację (?)
// przepisać pbNum jako klasę (jakie korzyści?) (?)
// przepisać pbSlot jako klasę (jakie korzyści?) (?)
// zrobić prawdziwą detekcję własności telefonu

#include <Application.h>
#include <File.h>
#include <List.h>
#include <ScrollView.h>
#include <SerialPort.h>
#include <String.h>
#include <TextView.h>
#include <Window.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// regexp
#include "Pattern.h"
#include "Matcher.h"

#include "globals.h"
#include "gsm.h"

int toint(const char *input) {
	if (input != NULL)
		return strtol(input, NULL, 10);
	else
		return 0;
}

#define TERMLOG_WINDOWNAME "Terminal log"

GSM::GSM(void) {

	if ((sem = create_sem(1, "gsm_sem")) < B_NO_ERROR)
		return;
	listMemSlotSMS = new BList(5);
	listMemSlotPB = new BList(10);

	// init status
	active = false;
	isMotorola = false;
	// init phone status
	fCharge = fSignal = 0;
	fACPower = fBatPower = false;
	fManuf = fModel = fGSMVer = fIMEI = "";
	fStatusUpdateEn = false;

	port = new BSerialPort;
	logFile = new BFile;
	logWindow = new BWindow(BRect(500,170,880,550),_(TERMLOG_WINDOWNAME),B_FLOATING_WINDOW_LOOK,B_NORMAL_WINDOW_FEEL,B_NOT_ZOOMABLE);
	BView *v = new BView(logWindow->Bounds(),"terminalView",B_FOLLOW_ALL_SIDES,B_WILL_DRAW);
	logWindow->AddChild(v);
	BRect s = v->Bounds(); s.InsetBy(10,10); s.right -= B_V_SCROLL_BAR_WIDTH;
	BRect t; t.top = t.left = 0; t.right = s.Width(); t.bottom = s.Height();
	logView = new BTextView(s, "terminalTextView",t,B_FOLLOW_ALL_SIDES);
	logView->MakeEditable(false);
	logView->SetStylable(false);
	v->AddChild(new BScrollView("terminalPrvScroll",logView,B_FOLLOW_ALL_SIDES,0,false,true));

	// default config
	parity = B_NO_PARITY;
	databits = B_DATA_BITS_8;
	stopbits = B_STOP_BITS_1;
	flowcontrol = B_HARDWARE_CONTROL;
	baudrate = B_19200_BPS;
	dtr = true;
	rts = true;
	initString = "ATZ";
//	initString = "AT S7=45 S0=0 V1 X4 &c1";	// from Kmobiletools (except E0)
}

GSM::~GSM() {
	doneDevice();
	delete_sem(sem);
	// close log file
	logFile->Unset();
	delete logFile;
	// close window
	if (!logWindow)
		delete logWindow;
	delete port;
}

bool GSM::initDevice(BMessage *msg) {
	const char *dev, *ini;
	if (! msg)
		return false;
	msg->FindString("_dev", &dev);
	msg->FindBool("_log", &log);
	msg->FindBool("_term", &term);
	msg->FindInt32("_parity", &parity);
	msg->FindInt32("_databits", &databits);
	msg->FindInt32("_stopbits", &stopbits);
	msg->FindInt32("_flowcontrol", &flowcontrol);
	msg->FindInt32("_baudrate", &baudrate);
	msg->FindBool("_dtr", &dtr);
	msg->FindBool("_rts", &rts);
	msg->FindString("_initstring", &ini);
	initString = ini;
	return initDevice(dev, log, term);
}

bool GSM::initDevice(const char *device, bool l = false, bool t = false) {
	BString tmp;

	log = l;
	term = t;
	if (log) {
		logFile->SetTo("/boot/home/bemobile.log",B_ERASE_FILE|B_CREATE_FILE|B_WRITE_ONLY);
	}
	if (term) {
		logWindow->Lock();
		logView->SetText("");
		logWindow->Unlock();
		logWindow->Show();
	}
	if (log || term) {
		tmp = "opening device:["; tmp += device; tmp += "] ";
		tmp += "dr:"; tmp << baudrate; tmp += " db:"; tmp << databits; tmp += " sb:"; tmp << stopbits;
		tmp += " pr:"; tmp << parity; tmp += " fl:"; tmp << flowcontrol;
		tmp += " dtr:"; tmp << dtr; tmp += " rts:"; tmp << rts; tmp += "\n";
		logWrite(tmp.String());
	}
	doneDevice();
	if (strlen(device)==0)
		return false;
	port->SetFlowControl(flowcontrol);
	if (port->Open(device) <= 0) {
		if (log || term) {
			tmp = "can't open bserialport\n";
			logWrite(tmp.String());
		}
		return false;
	} else {
		port->SetDataRate((data_rate)baudrate);
		port->SetDataBits((data_bits)databits);
		port->SetStopBits((stop_bits)stopbits);
		port->SetParityMode((parity_mode)parity);
		port->SetTimeout(100000);
		port->SetDTR(dtr);
		port->SetRTS(rts);
		snooze(100000);
		// success?
		active = phoneReset();
	}
	return active;
}

void GSM::doneDevice(void) {
	if (active)
		port->Close();
	active = false;
}

void GSM::logWrite(const char *t) {
	if (log) {
		logFile->Write(t,strlen(t));
	}
	if (term) {
		static BString tmp;
		for (int i=0;i<be_app->CountWindows();i++) {
			tmp = be_app->WindowAt(i)->Name();
			if (tmp == TERMLOG_WINDOWNAME) {
				logWindow->Lock();
				logView->Insert(strlen(logView->Text()),t,strlen(t));
				logWindow->Unlock();
				break;
			}
		}
	}
}

// return 0 for OK, 1 for other, 2 for error, 3 for sem_error, 4 for tmout, 5 for unopened
int GSM::sendCommand(const char *cmd, BString *out = NULL, bool debug = false) {
// timeout threshold
#define THRSTMOUT 8
#define WLOG logWrite(lll.String());
	BString tmp;
	BString lll;
	static char buffer[10240];
	int r;
	int status = 0;
	int tmout = 0;

	if (log || term) {
		lll = "<--["; lll += cmd; lll += "]\n"; WLOG;
	}

	if (!active) {
if (debug) printf("port not open\n");
		if (log || term) {
			lll = "ERR: port not open\n"; WLOG;
		}
		buffer[0] = '\0';
		return COM_NOTOPEN;
	}

if (debug) printf("sending:[%s]\n",cmd);
	tmp = cmd; tmp += "\n\r";
	memset(buffer,0,sizeof(buffer));

	if (acquire_sem(sem) != B_NO_ERROR) {
		if (log || term) {
			lll = "ERR: can't acquire semaphore to serial port\n"; WLOG;
		}
		return COM_SEMERROR;
	}

	r = port->Write(tmp.String(), tmp.Length());

	// reset output
	tmp = "";
	// read answer
	r = port->WaitForInput();
	while (tmout < THRSTMOUT) {
if (debug) printf("wfi:%i\n",r);
		if (r>0) {
			r = port->Read(buffer,r);
			if (log || term || (out != NULL))		// preserve output only if needed
				tmp.Append(buffer,r);
if (debug) printf("got:[%s]\n",buffer);
			if (strstr(buffer,"RING")) {
printf("RRRIIIIIIIIIIIINGGGGGGG!\b\n");
				ringIncoming = true;
			}
			if (strstr(buffer,"\nOK\r\n")) {
				status = COM_OK;
if (debug) printf("ok!\n");
				break;
			}
			if (strstr(buffer,"ERROR")) {
				status = COM_ERROR;
if (debug) printf("error!\n");
				break;
			}
			if (strstr(buffer,"\nCONNECT") || strstr(buffer,"\nNO CARRIER\r\n") || strstr(buffer,"\nBUSY\r\n")) {
				status = COM_OTHER;
				break;
			}
		}
		// multiple runs will overwrite buffer, but we keep data in tmp
		snooze(100000);
		r = port->WaitForInput();
		if (r==0)
			tmout++;
	}
	// copy out data while locked
	if (log || term) {
		lll = "-->["; lll += tmp; lll += "]\n"; WLOG;
	}
	if (out!=NULL)
		out->SetTo(tmp);
	release_sem(sem);
	if (tmout>0)
		printf("tmout=%i, cmd=[%s]\n",tmout,cmd);
	if (tmout == THRSTMOUT) {
		if (log || term) {
			lll = "ERR: timeout\n"; WLOG;
		}
		status = COM_TIMEOUT;
	}
	return status;
}

bool GSM::phoneReset(void) {
	// reset
	active = true;	// lie for a moment
	// send some stuff to flush and drain
	sendCommand("\x1A");
	sendCommand("ATZ");
	// initialization string can't be ignored, it may ERR, but not timeout
	if (sendCommand(initString.String()) == COM_TIMEOUT) {
		active = false;
		return false;
	}
	// disable local echo
	sendCommand("ATE0");

	// fetch character sets, default to utf8 or fallback to whatever is there
//	sendCommand("AT+CSCS=?");
	ringIncoming = false;
	rawUTF8 = false;
	return true;
}

void GSM::getPhoneData(void) {
	static Pattern *pManuf = Pattern::compile("^\\+CGMI: \"(.*)\"$", Pattern::MULTILINE_MATCHING);
	static Matcher *mManuf = pManuf->createMatcher("");
	static Pattern *pIMEI = Pattern::compile("^\\+CGSN: IMEI(\\d+)", Pattern::MULTILINE_MATCHING);
	static Matcher *mIMEI = pIMEI->createMatcher("");
	static Pattern *pGSM = Pattern::compile("\"GSM(\\d+)\",?", Pattern::MULTILINE_MATCHING);
	static Matcher *mGSM = pGSM->createMatcher("");
	static Pattern *pModel = Pattern::compile("\"(MODEL\\=)?(\\w*)\"$", Pattern::MULTILINE_MATCHING);
	static Matcher *mModel = pModel->createMatcher("");
	static Pattern *pSoft = Pattern::compile("^\\+CGMR: \"(.*)\"$", Pattern::MULTILINE_MATCHING);
	static Matcher *mSoft = pSoft->createMatcher("");
	static Pattern *pIMSI = Pattern::compile("^\\+CIMI: (\\d+)", Pattern::MULTILINE_MATCHING);
	static Matcher *mIMSI = pIMSI->createMatcher("");
	static Pattern *pIMSI2 = Pattern::compile("^\(\\d+)", Pattern::MULTILINE_MATCHING);
	static Matcher *mIMSI2 = pIMSI2->createMatcher("");

	BString tmp;

	// manufacturer
	sendCommand("AT+CGMI",&tmp);
	mManuf->setString(tmp.String());
	if (mManuf->findFirstMatch())
		fManuf = mManuf->getGroup(1).c_str();
	// software revision
	sendCommand("AT+CGMR",&tmp);
	mSoft->setString(tmp.String());
	if (mSoft->findFirstMatch())
		fSoftwareVer = mSoft->getGroup(1).c_str();
	// supported GSM versions
	sendCommand("AT+CGMM",&tmp);
	mGSM->setString(tmp.String());
	if (mGSM->findFirstMatch()) {
		fGSMVer = mGSM->getGroup(1).c_str();
		while (mGSM->findNextMatch()) {
			fGSMVer += "/";
			fGSMVer += mGSM->getGroup(1).c_str();
		}
	}
	// model
	mModel->setString(tmp.String());
	if (mModel->findFirstMatch()) {
		fModel = mModel->getGroup(2).c_str();
	}
	// IMEI
	sendCommand("AT+CGSN",&tmp);
	mIMEI->setString(tmp.String());
	if (mIMEI->findFirstMatch())
		fIMEI = mIMEI->getGroup(1).c_str();
	// IMSI
	sendCommand("AT+CIMI",&tmp);
	mIMSI->setString(tmp.String());
	if (mIMSI->findFirstMatch())
		fIMSI = mIMSI->getGroup(1).c_str();
	else {
		mIMSI2->setString(tmp.String());
		if (mIMSI2->findFirstMatch())
			fIMSI = mIMSI2->getGroup(1).c_str();
	}
	// Motorola check for extended commands
	isMotorola = (sendCommand("AT+MMGL=?") == COM_OK);
	// enable error reporting, try most verbose mode
	sendCommand("AT+CMEE=1");
	sendCommand("AT+CMEE=2");

	// enable native UTF8 message format, fallback to Unicode and GSM
	fEncoding = ENC_UTF8;
	if (sendCommand("AT+CSCS=\"UTF8\"") != COM_OK) {
		fEncoding = ENC_UCS2;
		if (sendCommand("AT+CSCS=\"UCS2\"") != COM_OK) {
			fEncoding = ENC_GSM;
			sendCommand("AT+CSCS=\"GSM\"");
		}
	}
	// possibly do it later using configuration, and now only query for encodings
	// +CSCS=?
	// +CSCS: ("8859-1","ASCII","GSM","UCS2","UTF8")
	// kmt sends also: AT+MODE=2 if 'smsMode2' is enabled in config
	// if failed: AT+MODE=0 is sent

	// switch to TEXT mode, test/enable PDU by passing 0 here
	sendCommand("AT+CMGF=1");
	getSMSMemSlots();
	getPBMemSlots();

	// model-specific
	if (fModel == "L6")
		rawUTF8 = true;
}

void GSM::getPhoneStatus(void) {
	static Pattern *pPower = Pattern::compile("^\\+CBC: (\\d+),(\\d+)", Pattern::MULTILINE_MATCHING);
	static Matcher *mPower = pPower->createMatcher("");
	static Pattern *pSignal = Pattern::compile("^\\+CSQ: (\\d+),(\\d+)", Pattern::MULTILINE_MATCHING);
	static Matcher *mSignal = pSignal->createMatcher("");
	static Pattern *pDateTime = Pattern::compile("^\\+CCLK: \"(\\d+)\\/(\\d+)\\/(\\d+),(\\d+):(\\d+):(\\d+).(\\d+)", Pattern::MULTILINE_MATCHING);
	static Matcher *mDateTime = pDateTime->createMatcher("");
	BString tmp;

	// power, battery
	sendCommand("AT+CBC",&tmp);
	mPower->setString(tmp.String());
	if (mPower->findFirstMatch()) {
		int i = toint(mPower->getGroup(1).c_str());
		fACPower = ((i==1) || (i==2));
		fBatPower = ((i==0) || (i==1));
		fCharge = toint(mPower->getGroup(2).c_str());
	}

	// signal power
	sendCommand("AT+CSQ",&tmp);
	mSignal->setString(tmp.String());
	if (mSignal->findFirstMatch()) {
		fSignal = toint(mSignal->getGroup(1).c_str())*100/31;
	}

	// date and time
	sendCommand("AT+CCLK?",&tmp);
	mDateTime->setString(tmp.String());
	if (mDateTime->findFirstMatch()) {
		char tmbuf[200];
		fYear = toint(mDateTime->getGroup(1).c_str()); fYear += 2000;
		fMonth = toint(mDateTime->getGroup(2).c_str());
		fDay = toint(mDateTime->getGroup(3).c_str());
		fHour = toint(mDateTime->getGroup(4).c_str());
		fMinute = toint(mDateTime->getGroup(5).c_str());
		fSecond = toint(mDateTime->getGroup(6).c_str());
		fTimezone = toint(mDateTime->getGroup(7).c_str());
		snprintf(tmbuf, sizeof(tmbuf), "%i-%02i-%02i %i:%02i:%02i", fYear,fMonth,fDay,fHour,fMinute,fSecond);
		fDateTime = tmbuf;
printf("%s\n",tmbuf);
	}
}

int GSM::setDateTime(void) {
	struct tm *tm;
	time_t curtime;
	char tmbuf[200];

	curtime = time(NULL);
	tm = localtime(&curtime);
	tm->tm_year += 1900; tm->tm_year -= 2000;
	tm->tm_mon++;
	snprintf(tmbuf,sizeof(tmbuf),"AT+CCLK=\"%02i/%02i/%02i,%02i:%02i:%02i+00\"",tm->tm_year,tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
printf("%s\n",tmbuf);
	return sendCommand(tmbuf);
}

void GSM::updateSMSInfo(void) {
	struct SMS *sms = NULL;
	struct memSlotSMS *sl;
	BList *SMSList;

	fSMSRRead = fSMSRUnread = fSMSSSent = fSMSUSent = 0;
	int k = listMemSlotSMS->CountItems();
	for (int l=0;l<k;l++) {
		sl = (struct memSlotSMS*)listMemSlotSMS->ItemAt(l);
		SMSList = sl->msg;
		int j = SMSList->CountItems();
		for (int i=0;i<j;i++) {
			sms = (struct SMS*)SMSList->ItemAt(i);
			switch(sms->type) {
				case REC_READ:		fSMSRRead++; break;
				case REC_UNREAD:	fSMSRUnread++; break;
				case STO_SENT:		fSMSSSent++; break;
				case STO_UNSENT:	fSMSUSent++; break;
				default:	break;
			}
		}
	}
	fSMSInfo = _("Inbox: "); fSMSInfo << fSMSRRead+fSMSRUnread;
	fSMSInfo += _(" messages, ("); fSMSInfo << fSMSRUnread;
	fSMSInfo += _(" unread). ");
	fSMSInfo += _("Outbox: "); fSMSInfo << fSMSSSent+fSMSUSent;
	fSMSInfo += _(" messages, ("); fSMSInfo << fSMSUSent;
	fSMSInfo += _(" not sent yet).");
}

const char *GSM::getSMSMemSlotName(const char *slot) {
	static BString s;
	s = slot;
	if (s == "MT") return _("All messages");
	if (s == "IM") return _("Inbox");
	if (s == "OM") return _("Outbox");
	if (s == "BM") return _("Information service");
	if (s == "DM") return _("Drafts");
	if (s == "ME") return _("Phone SMS memory");
	if (s == "SM") return _("SIM SMS memory");
	s = _("Unknown SMS slot"); s += " ("; s += slot; s += ")";
	return s.String();
}

void GSM::getSMSMemSlots(void) {
	static Pattern *pSlots = Pattern::compile("^\\+CPMS: \\(([^)]+)\\),\\(([^)]+)\\)", Pattern::MULTILINE_MATCHING);
	static Matcher *mSlots = pSlots->createMatcher("");
	static Pattern *pSlot = Pattern::compile("(\\w\\w)");
	static Matcher *mSlot = pSlot->createMatcher("");

	BString out;

	// clear list
	if (listMemSlotSMS->CountItems()>0) {
		struct memSlotSMS *anItem;
		for (int i=0; (anItem=(struct memSlotSMS*)listMemSlotSMS->ItemAt(i)); i++)
			delete anItem;
		if (!listMemSlotSMS->IsEmpty())
			listMemSlotSMS->MakeEmpty();
	}

	struct memSlotSMS *slot;

	sendCommand("AT+CPMS=?",&out);
	mSlots->setString(out.String());
	if (mSlots->findFirstMatch()) {
		mSlot->setString(mSlots->getGroup(1).c_str());
		while (mSlot->findNextMatch()) {
			slot = new memSlotSMS;
			slot->sname = mSlot->getGroup(1).c_str();
			slot->name = getSMSMemSlotName(slot->sname.String());
			slot->items = changeSMSMemSlot(slot->sname.String());
			slot->unread = -1;
			slot->msg = new BList;
			slot->writable = false;
			listMemSlotSMS->AddItem(slot);
			printf("got:%s,%i\n",slot->name.String(),slot->items);
		}
		mSlot->setString(mSlots->getGroup(2).c_str());
		while (mSlot->findNextMatch()) {
			printf("writable:%s\n",mSlot->getGroup(1).c_str());
			slot = getSMSSlot(mSlot->getGroup(1).c_str());
			slot->writable = true;
		}
	}
	changeSMSMemSlot("MT");	// make it default
}

int GSM::changeSMSMemSlot(const char *slot) {
	static Pattern *pSlot = Pattern::compile("^\\+CPMS: (\\d+),(\\d+)", Pattern::MULTILINE_MATCHING);
	static Matcher *mSlot = pSlot->createMatcher("");
	int msgNum = 0;
	BString out, cmd = "AT+CPMS=\"";
	cmd += slot; cmd +="\"";

	sendCommand(cmd.String(),&out);
	mSlot->setString(out.String());
	if (mSlot->findFirstMatch()) {
		msgNum = toint(mSlot->getGroup(1).c_str());
		struct memSlotSMS *sl = getSMSSlot(slot);
		if (sl)
			sl->items = msgNum;
	}
	return msgNum;
}

struct memSlotSMS *GSM::getSMSSlot(const char *slot) {
	int i;
	int j = listMemSlotSMS->CountItems();

	for (i=0;i<j;i++) {
		if (strcmp(slot,((struct memSlotSMS*)listMemSlotSMS->ItemAt(i))->sname.String()) == 0)
			return ((struct memSlotSMS*)listMemSlotSMS->ItemAt(i));
	}
	return NULL;
}

bool GSM::hasSMSSlot(const char *slot) {
	int i;
	int j = listMemSlotSMS->CountItems();

	for (i=0;i<j;i++) {
		if (strcmp(slot,((struct memSlotSMS*)listMemSlotSMS->ItemAt(i))->sname.String()) == 0)
			return true;
	}
	return false;
}

int GSM::getSMSType(const char *type) {
	BString t = type;
	if (t == "REC READ") return REC_READ;
	if (t == "REC UNREAD") return REC_UNREAD;
	if (t == "STO SENT") return STO_SENT;
	if (t == "STO UNSENT") return STO_UNSENT;
	return MSG_UNK;
}

void GSM::getSMSContent(SMS *sms = NULL) {
	BString cmd, pat, pat2, pat3, out;

	if (sms == NULL)
		return;

	cmd = isMotorola ? "AT+MMGR=" : "AT+CMGR=";
	cmd << sms->id;
	pat = isMotorola ? "^\\+M" : "^\\+C";
	pat2 = pat;
	pat3 = pat;
	pat += "MGR: \"([ \\w]+)\", \"([^\"]+)\", \"([\\d\\/,:]+)\"\r\n(.*)OK$";
	pat2 += "MGR: \"([ \\w]+)\", \"([^\"]+)\".*\r\n(.*)$";
	pat3 += "MGR: \"([ \\w]+)\".*\r\n(.*)$";
	Pattern *pMsg1 = Pattern::compile(pat.String(), Pattern::DOT_MATCHES_ALL|Pattern::MULTILINE_MATCHING);
	Matcher *mMsg1 = pMsg1->createMatcher("");
	Pattern *pMsg2 = Pattern::compile(pat2.String(), Pattern::MULTILINE_MATCHING);
	Matcher *mMsg2 = pMsg2->createMatcher("");
	Pattern *pMsg3 = Pattern::compile(pat3.String(), Pattern::MULTILINE_MATCHING);
	Matcher *mMsg3 = pMsg3->createMatcher("");

	sendCommand(cmd.String(),&out);
	mMsg1->setString(out.String());
	if (mMsg1->findFirstMatch()) {
		sms->number = mMsg1->getGroup(2).c_str();
		sms->date = parseDate(mMsg1->getGroup(3).c_str());
		sms->msg = decodeSMSText(mMsg1->getGroup(4).c_str());	
		matchNumFromSMS(sms);
	} else {
		mMsg2->setString(out.String());
		if (mMsg2->findFirstMatch()) {
			sms->number = mMsg2->getGroup(2).c_str();
			sms->msg = decodeSMSText(mMsg2->getGroup(3).c_str());
			matchNumFromSMS(sms);
		} else {
			mMsg3->setString(out.String());
			if (mMsg3->findFirstMatch()) {
				sms->msg = decodeSMSText(mMsg3->getGroup(2).c_str());
			}
		}
	}
printf("%i,%i,%s,:%s\n[%s]\n",sms->id,sms->type,sms->number.String(),sms->date.String(),sms->msg.String());
}

void GSM::getSMSList(const char *slot) {
	struct memSlotSMS *sl = getSMSSlot(slot);
	int msgNum = changeSMSMemSlot(slot);
	BList *SMSList;

	sl->unread = 0;
	sl->items = 0;

	if (msgNum == 0)
		return;

	BString cmd, out, pat;

	cmd = isMotorola ? "AT+MMGL=\"HEADER ONLY\"" : "AT+CMGL";
	sendCommand(cmd.String(),&out);

	pat = isMotorola ? "^\\+M" : "^\\+C";
	pat += "MGL: (\\d+), \"([ \\w]+)\"";

	Pattern *pList = Pattern::compile(pat.String(), Pattern::MULTILINE_MATCHING);
	Matcher *mList = pList->createMatcher("");

	struct SMS *cursms = NULL;

	SMSList = sl->msg;
	// clear list
	if (SMSList->CountItems()>0) {
		struct SMS *anItem;
		for (int i=0; (anItem=(struct SMS*)SMSList->ItemAt(i)); i++) {
			SMSClearNumList(anItem);
			delete anItem;
		}
		if (!SMSList->IsEmpty())
			SMSList->MakeEmpty();
	}

	// XXX show progress?
	mList->setString(out.String());
	while (mList->findNextMatch()) {
		cursms = new SMS;
		cursms->slot = slot;
		cursms->id = toint(mList->getGroup(1).c_str());
		cursms->type = getSMSType(mList->getGroup(2).c_str());
		sl->items++;
		switch(cursms->type) {
			case REC_UNREAD:
			case STO_UNSENT:
				sl->unread++;
				break;
			default:
				break;
		}
		SMSList->AddItem(cursms);
	}
}

int GSM::removeSMS(SMS *sms = NULL) {
	if (!sms)
		return -1;
	//changeMemSlot(sms->slot.String());		// XXX possibly unneeded, IDs are unique?
	BString cmd = "AT+CMGD="; cmd << sms->id;
	int ret = sendCommand(cmd.String());
	if (ret == COM_OK) {
		struct memSlotSMS *sl = getSMSSlot(sms->slot.String());
		if ((sms->type == GSM::REC_UNREAD) || (sms->type == GSM::STO_UNSENT))
			sl->unread--;
		sl->items--;
		sl->msg->RemoveItem(sms);
	}
	//changeMemSlot("MT");						// XXX back to default
	return ret;
}

int GSM::storeSMS(const char *slot, const char *numbers, const char *msg) {
	if ( (!msg) || (!numbers) || (!slot) )
		return -1;
	if ( (strlen(msg)==0) || (strlen(numbers)==0) || (strlen(slot)==0) )
		return -1;
//	printf("sending to [%s],to[%s],msg=[%s]\n",slot,numbers,msg);
	BString sl = slot;
	sl += "\",\""; sl += slot;		// hackery!
	changeSMSMemSlot(sl.String());

	BString cmd = "AT+CMGW=\"";
	cmd += numbers; cmd +="\"\r";
	cmd += encodeText(msg); cmd += "\x1A";
	int ret = sendCommand(cmd.String());
	if ((ret == COM_OK) || (ret == COM_TIMEOUT))
		return 0;
	return -1;
}

int GSM::sendSMS(const char *numbers, const char *msg) {
	if ( (!msg) || (!numbers) )
		return -1;
	if ( (strlen(msg)==0) || (strlen(numbers)==0) )
		return -1;
	BString cmd = "AT+CMGS=\"";
	cmd += numbers; cmd +="\"\r";
	cmd += encodeText(msg); cmd += "\x1A";
	int ret = sendCommand(cmd.String());
	if ((ret == COM_OK) || (ret == COM_TIMEOUT))
		return 0;
	return -1;
}

int GSM::sendSMSFromStorage(const char *slot, int id) {
	if ( (!slot) || (id < 0) )
		return -1;
	if ( strlen(slot)==0 )
		return -1;

	BString sl = slot;
	sl += "\",\""; sl += slot;		// hackery!
	changeSMSMemSlot(sl.String());

	BString cmd = "AT+CMSS="; cmd << id;
	int ret = sendCommand(cmd.String());
	if ((ret == COM_OK) || (ret == COM_TIMEOUT))
		return 0;
	return -1;
}

const char *GSM::getPBMemSlotName(const char *slot) {
	static BString s;
	s = slot;
	// phonebooks
	if ((s == "MT") || (s == "AD")) return _("Composite phonebook");
	if (s == "EN") return _("Emergency numbers");
	if (s == "FD") return _("SIM fixed dialing numbers");
	if (s == "ME") return _("Phone number list");
	if ((s == "ON") || (s == "OW")) return _("Own numbers");
	if (s == "SM") return _("SIM number list");
	if (s == "SD") return _("SIM number list");	// XXX guessing and can't read properties
	if (s == "TA") return _("Data card number list");
	if (s == "MV") return _("Voice activated phonebook");
	if (s == "HP") return _("Hierarchical phonebook");
	if (s == "BC") return _("Own business card");
	if ((s == "QD") || (s == "DD")) return _("Quick dial number list");
	// call register
	if (s == "MD") return _("Last number redial");	// XXX does it belong here?
	if (s == "LD") return _("SIM last dialed numbers");
	if (s == "MC") return _("Missed calls");
	if (s == "DC") return _("Dialed calls");
	if (s == "RC") return _("Received calls");
	s = _("Unknown phonebook type");
	s += " ("; s += slot; s += ")";
	return s.String();
}

bool GSM::isPBSlotWritable(const char *slot) {
	BString s = slot;
	// add "SD" here (no params available)
	if ((s == "EN") || (s == "DC") || (s == "LD") || (s == "MC") || (s == "RC") || (s == "MD"))
		return false;
	else
		return true;
}

bool GSM::isPBSlotCallRegister(const char *slot) {
	BString s = slot;
	// XXX add "MD" here ???
	if ((s == "LD") || (s == "MC") || (s == "DC") || (s == "RC"))
		return true;
	else
		return false;
}

void GSM::getPBMemSlots(void) {
	static Pattern *pSlots = Pattern::compile("^\\+CPBS: \\(([^)]+)\\)", Pattern::MULTILINE_MATCHING);
	static Matcher *mSlots = pSlots->createMatcher("");
	static Pattern *pSlot = Pattern::compile("(\\w\\w)");
	static Matcher *mSlot = pSlot->createMatcher("");

	BString out;

	// clear list
	if (listMemSlotPB->CountItems()>0) {
		struct pbSlot *anItem;
		for (int i=0; (anItem=(struct pbSlot*)listMemSlotPB->ItemAt(i)); i++)
			delete anItem;
		if (!listMemSlotPB->IsEmpty())
			listMemSlotPB->MakeEmpty();
	}

	struct pbSlot *slot;

	sendCommand("AT+CPBS=?",&out);
	mSlots->setString(out.String());
	if (mSlots->findFirstMatch()) {
		mSlot->setString(mSlots->getGroup(1).c_str());
		while (mSlot->findNextMatch()) {
			slot = new pbSlot;
			slot->sname = mSlot->getGroup(1).c_str();
			slot->name = getPBMemSlotName(slot->sname.String());
			slot->writable = isPBSlotWritable(slot->sname.String());
			slot->callreg = isPBSlotCallRegister(slot->sname.String());
			slot->pb = new BList;
			slot->fields = new BList;
			listMemSlotPB->AddItem(slot);
			slot->min = slot->max = 0;
			if (checkPBMemSlot(slot) == false) {
				listMemSlotPB->RemoveItem(slot);
				delete slot;
			}
			else printf("got:%s\n",slot->name.String());
		}
	}
}

bool GSM::changePBMemSlot(const char *slot) {
	BString cmd = "AT+CPBS=\"";
	cmd += slot; cmd += "\"";

	return (sendCommand(cmd.String()) == COM_OK);
}

bool GSM::checkPBMemSlot(struct pbSlot *sl = NULL) {
	static Pattern *pSlot = Pattern::compile("^\\+CPBR: \\((\\d+)-(\\d+)\\),(\\d+),(\\d+)", Pattern::MULTILINE_MATCHING);
	static Matcher *mSlot = pSlot->createMatcher("");

	if (!sl)
		return false;

	if (changePBMemSlot(sl->sname.String()) == false)
		return false;

	BString out;
	// return false on error -> don't add such slot
	if (sendCommand("AT+CPBR=?",&out) == COM_ERROR)
		return false;

	mSlot->setString(out.String());
	if (mSlot->findFirstMatch()) {
		sl->min = toint(mSlot->getGroup(1).c_str());
		sl->max = toint(mSlot->getGroup(2).c_str());
		printf("got:%s - (%i-%i)\n",sl->sname.String(),sl->min,sl->max);
		// two fields: name & number are always present, always put them as first on the list
		pbField *pf;
		pf = new pbField;
		pf->type = isMotorola ? PF_PHONEEMAIL : PF_PHONE;	// be cautious
		pf->name = _("Number");
		pf->max = toint(mSlot->getGroup(3).c_str());
		pf->offset = 1;
		sl->fields->AddItem(pf);
		pf = new pbField;
		pf->type = PF_TEXT;
		pf->name = _("Name");
		pf->max = toint(mSlot->getGroup(4).c_str());
		pf->offset = 3;
		sl->fields->AddItem(pf);
	}
	sl->has_phtype = false;
	sl->has_address = false;
	if (isMotorola) {
		if (sendCommand("AT+MPBR=?",&out) == COM_OK) {
			// count commas... XXX this is stupid!
			int c = 0;
			for (int i=0; i<out.Length(); i++) {
				if (out[i] == ',')
					c++;
			}
//printf("%i commas\n",c);
			sl->has_phtype = (c>9);
			sl->has_address = (c>16);
		}
	}
	// device capabilities
	if (sl->has_phtype) {
		pbField *pf;
		pf = new pbField; pf->type = PF_COMBO; pf->name = _("Type");
		pf->offset = 4;
		pf->cb = new BList;
		pbCombo *pc;
		pc = new pbCombo; pc->text = _("Work"); pc->v = PK_WORK; pf->cb->AddItem(pc);
		pc = new pbCombo; pc->text = _("Home"); pc->v = PK_HOME; pf->cb->AddItem(pc);
		pc = new pbCombo; pc->text = _("Main"); pc->v = PK_MAIN; pf->cb->AddItem(pc);
		pc = new pbCombo; pc->text = _("Mobile"); pc->v = PK_MOBILE; pf->cb->AddItem(pc);
		pc = new pbCombo; pc->text = _("Fax"); pc->v = PK_FAX; pf->cb->AddItem(pc);
		pc = new pbCombo; pc->text = _("Pager"); pc->v = PK_PAGER; pf->cb->AddItem(pc);
		pc = new pbCombo; pc->text = _("E-mail"); pc->v = PK_EMAIL; pf->cb->AddItem(pc);
		pc = new pbCombo; pc->text = _("Mailing list"); pc->v = PK_MAILLIST; pf->cb->AddItem(pc);
		pf->max = PK_MAILLIST;
		sl->fields->AddItem(pf);
		pf = new pbField; pf->type = PF_BOOL; pf->name = _("Primary number");
		pf->offset = 8;
		sl->fields->AddItem(pf);
		// add address attributes if required
		if (sl->has_address) {
			pf = new pbField; pf->type = PF_TEXT; pf->name = _("Address (2)");
			pf->max = 30; pf->offset = 16;
			sl->fields->AddItem(pf);
			pf = new pbField; pf->type = PF_TEXT; pf->name = _("Address");
			pf->max = 30; pf->offset = 17;
			sl->fields->AddItem(pf);
			pf = new pbField; pf->type = PF_TEXT; pf->name = _("City");
			pf->max = 30; pf->offset = 18;
			sl->fields->AddItem(pf);
			pf = new pbField; pf->type = PF_TEXT; pf->name = _("State");
			pf->max = 30; pf->offset = 19;
			sl->fields->AddItem(pf);
			pf = new pbField; pf->type = PF_TEXT; pf->name = _("Zip");
			pf->max = 7; pf->offset = 20;
			sl->fields->AddItem(pf);
			pf = new pbField; pf->type = PF_TEXT; pf->name = _("Country");
			pf->max = 20; pf->offset = 21;
			sl->fields->AddItem(pf);
			pf = new pbField; pf->type = PF_TEXT; pf->name = _("Nick");
			pf->max = 24; pf->offset = 22;
			sl->fields->AddItem(pf);
			pf = new pbField; pf->type = PF_TEXT; pf->name = _("Birthday (MM-DD-YYYY)");
			pf->max = 10; pf->offset = 23;
			sl->fields->AddItem(pf);
		}
	}
	return true;
}

bool GSM::hasPBSlot(const char *slot) {
	int i;
	int j = listMemSlotPB->CountItems();

	for (i=0;i<j;i++) {
		if (strcmp(slot,((struct pbSlot*)listMemSlotPB->ItemAt(i))->sname.String()) == 0)
			return true;
	}
	return false;
}

struct pbSlot *GSM::getPBSlot(const char *slot) {
	int i;
	int j = listMemSlotPB->CountItems();

	for (i=0;i<j;i++) {
		if (strcmp(slot,((struct pbSlot*)listMemSlotPB->ItemAt(i))->sname.String()) == 0)
			return ((struct pbSlot*)listMemSlotPB->ItemAt(i));
	}
	return NULL;
}

void GSM::getPBList(const char *slot) {
	struct pbSlot *sl = getPBSlot(slot);
	changePBMemSlot(slot);

	BList *pbList = sl->pb;
	BString cmd, out, pat;

	cmd = isMotorola ? "AT+MPBR=" : "AT+CPBR=";
	cmd << sl->min; cmd += ","; cmd << sl->max;

	// clear list
	if (pbList->CountItems()>0) {
		struct pbNum *anItem;
		for (int i=0; (anItem=(struct pbNum*)pbList->ItemAt(i)); i++)
			delete anItem;
		if (!pbList->IsEmpty())
			pbList->MakeEmpty();
	}
	int rs = sendCommand(cmd.String(),&out);
	// accept timeout (SIM reading may be slow)
	if ((rs == COM_OK)||(rs == COM_TIMEOUT)) {
		pat = isMotorola ? "^\\+MP" : "^\\+CP";
		pat += "BR: (\\d+),\"([^\"]*)\",(\\d+),([^,\r\n]*)";
		if (isMotorola) {
			if (sl->has_phtype)
			// 5phtype,6voicetag,7alerttone,8backlight,9primary,10categorynum
				pat += ",(\\d+),(\\d+),(\\d+),(\\d+),(\\d+),(\\d+)";
			// 11,12,13-??,14iconpath,15-16??,17adres2,18adres1,19miasto,20stan,21kod,22kraj,23pseudo,24bday(mm-dd-yyyy),25??
			if (sl->has_address)
				pat += ",(\\d+),(\\d+),(\\d+),([^,\r\n]*),(\\d+),(\\d+),([^,\r\n]*),([^,\r\n]*),([^,\r\n]*),([^,\r\n]*),([^,\r\n]*),([^,\r\n]*),([^,\r\n]*),([^,\r\n]*)";
		}
		Pattern *pNum = Pattern::compile(pat.String(), Pattern::MULTILINE_MATCHING);
		Matcher *mNum = pNum->createMatcher("");
		struct pbNum *num;

		mNum->setString(out.String());
		while (mNum->findNextMatch()) {
			num = new pbNum;
			num->slot = slot;
			num->raw = mNum->getGroup(0).c_str();
			num->id = toint(mNum->getGroup(1).c_str());
			// XXX these should be derived from attr, but are needed for phonelistviews
			num->number = mNum->getGroup(2).c_str();
			switch (toint(mNum->getGroup(3).c_str())) {
				case 129:
					num->type = PB_PHONE;
					break;
				case 145:
					num->type = PB_INTLPHONE;
					break;
				case 128:
					num->type = PB_EMAIL;
					break;
				default:
					num->type = guessPBType(num->number.String());
					break;
			}
			if (mNum->getGroup(4).c_str()[0] != '"')
				num->name = decodeText(mNum->getGroup(4).c_str());
			else {
				BString tmp(mNum->getGroup(4).c_str());
				tmp.CopyInto(num->name,1,tmp.Length()-2);
			}
//			printf("%i:%s:%s:%i\n",num->id,num->number.String(),num->name.String(),num->type);
			//
			// XXX isMotorola && has_phtype...
			num->attr = new BList;
			union pbVal *v;
			struct pbField *pf;
			int j = sl->fields->CountItems();
			int offset;
			for (int i=0; i<j; i++) {
				pf = (struct pbField*)sl->fields->ItemAt(i);
				v = new pbVal;
				offset = pf->offset + 1;
				switch (pf->type) {
					case PF_PHONEEMAIL:
					case PF_PHONE:
						v->text = new BString(mNum->getGroup(offset).c_str());
						break;
					case PF_TEXT:
						{	if (mNum->getGroup(offset).c_str()[0] != '"')
								v->text = new BString(decodeText(mNum->getGroup(offset).c_str()));
							else {
								BString tmp2;
								BString tmp(mNum->getGroup(offset).c_str());
								tmp.CopyInto(tmp2,1,tmp.Length()-2);
								v->text = new BString(tmp2);
							}
						}
						break;
					case PF_BOOL:
						v->b = (toint(mNum->getGroup(offset).c_str()) != 0);
						break;
					case PF_COMBO:
						v->v = toint(mNum->getGroup(offset).c_str());
						if (v->v > pf->max)
							v->v = -1;
						break;
				}
				num->attr->AddItem(v);
			}
			//
			pbList->AddItem(num);
		}
	}

	//changePBMemSlot("MT");		// XXX change to default?
}

const char *GSM::stripLeadNum(const char *num) {
	static BString tmp;
	tmp = num;
	tmp.RemoveSet("+");
again:	// remove leading zeros, quite ugly
	if (tmp.Length()>0)
		if (tmp[0] == '0') {
			tmp.Remove(0,1);
			goto again;
		}
	// assume 9 digit numbers
	if (tmp.Length()>9) {
		BString tmp2 = tmp;
		tmp2.CopyInto(tmp,tmp.Length()-9,9);
	}
	return tmp.String();
}

struct pbNum *GSM::matchNumFromNum(const char *num) {
	BString crnum = stripLeadNum(num);

	// search in all Phonebook slots
	int k, l = listMemSlotPB->CountItems();
	struct pbSlot *pb;
	for (k=0;k<l;k++) {
		pb = (struct pbSlot*)listMemSlotPB->ItemAt(k);
		// don't search in callreg, just phonebooks
		if (!pb->callreg) {
		// search in all items
			int m, n = pb->pb->CountItems();
			struct pbNum *pn;
			for (m=0;m<n;m++) {
				pn = (struct pbNum*)pb->pb->ItemAt(m);
				if (crnum.Compare(stripLeadNum(pn->number.String())) == 0) {
					printf("%i:[%s]:[%s]\n",pn->id,pn->number.String(),crnum.String());
					return pn;
				}
			}
		}
	}
	// no match
	return NULL;
}

struct pbNum *GSM::matchNumFromPB(struct pbNum *num) {
	struct pbNum *newpb = new pbNum;
	newpb->slot = num->slot;
	newpb->id = num->id;		// slot and id from original
	struct pbNum *pb = matchNumFromNum(num->number.String());
	if (pb) {
		// phone book entry matched
		newpb->number = pb->number;	// all other data from phonebook
		newpb->name = pb->name;
		newpb->type = pb->type;
		newpb->attr = pb->attr;
	} else {
		newpb->number = num->number;
		newpb->name = num->name;
		newpb->type = num->type;
		newpb->attr = num->attr;
	}
	return newpb;
}

void GSM::matchNumFromSMS(struct SMS *sms) {
	Pattern *pNum = Pattern::compile("([^ ]+)", Pattern::MULTILINE_MATCHING);
	Matcher *mNum = pNum->createMatcher("");
	struct pbNum *pb, *newpb;
	// break down by spaces
	mNum->setString(sms->number.String());
	while (mNum->findNextMatch()) {
//		printf("got:[%s]\n",mNum->getGroup(0).c_str());
		pb = matchNumFromNum(mNum->getGroup(0).c_str());
		newpb = new pbNum;
		if (pb) {
			newpb->slot = pb->slot;
			newpb->id = pb->id;
			newpb->number = pb->number;
			newpb->name = pb->name;
			newpb->type = pb->type;
			newpb->attr = pb->attr;
		} else {
			newpb->slot = "??";
			newpb->id = -1;
			newpb->number = mNum->getGroup(0).c_str();
			newpb->name = "";
			newpb->type = guessPBType(newpb->number.String());
			// basic required attributes
			newpb->attr = new BList;
			union pbVal *v;
			v = new pbVal;
			v->text = new BString(mNum->getGroup(0).c_str());
			newpb->attr->AddItem(v);
			v = new pbVal;
			v->text = new BString("");
			newpb->attr->AddItem(v);
		}
		sms->pbnumbers.AddItem(newpb);
	}
}

// don't call this routine for callreg - it won't work: 1) no support, 2) stored pbNum* is different (new struct after match)
int GSM::removePBItem(struct pbNum *num = NULL) {
	if (!num)
		return -1;
	BString cmd = "AT+CPBW="; cmd << num->id;
	changePBMemSlot(num->slot.String());
	int ret = sendCommand(cmd.String());
	if (ret == COM_OK) {
		struct pbSlot *sl = getPBSlot(num->slot.String());
		sl->pb->RemoveItem(num);
	}
	return ret;
}

// XXX this stores only attr fields, number/name are ignored!
int GSM::storePBItem(struct pbNum *num = NULL) {
	if (!num)
		return -1;
	changePBMemSlot(num->slot.String());
	struct pbSlot *sl = getPBSlot(num->slot.String());

	BString cmd, out;

	cmd = isMotorola ? "AT+MPBW=" : "AT+CPBW=";
	if (num->id > 0)
		cmd << num->id;
	// go through fields (asc offset) and add attributes and commas
	// XXX assume that fields list is sorted by offset asc
	union pbVal *v;
	struct pbField *pf;
	int j = sl->fields->CountItems();
	int pos = 1;
	for (int i=0; i<j; i++) {
		pf = (struct pbField*)sl->fields->ItemAt(i);
		v = (union pbVal*)num->attr->ItemAt(i);
		// fix commas, track position
		while (pf->offset > pos) {
			pos++;
			cmd += ",";
		}
		switch (pf->type) {
			case PF_PHONEEMAIL:
			case PF_PHONE:
				cmd += ",\""; cmd += v->text->String(); cmd += "\"";
				break;
			case PF_TEXT:
				cmd += ",";
				if (rawUTF8) cmd += "\"";
				cmd += encodeText(v->text->String());
				if (rawUTF8) cmd += "\"";
				break;
			case PF_BOOL:
				cmd += ","; cmd += (v->b ? "1" : "0");
				break;
			case PF_COMBO:
				cmd += ",";
				if ((v->v >= 0) && (v->v <= pf->max))
					cmd << v->v;
				break;
		}
		pos++;
	}
	printf("cmd:[%s]\n",cmd.String());

	int ret = sendCommand(cmd.String(),&out,true);

	// sometimes this gives the delayed error
	if (ret == COM_OK) {
		ret = sendCommand("AT",NULL,true);
		// if there is error, try again and try delayed error
		if (ret != COM_OK) {
			ret = sendCommand(cmd.String(), &out, true);
			if (ret == COM_OK)
				ret = sendCommand("AT",NULL,true);
		}
	}

	return ret;
}

void GSM::SMSClearNumList(struct SMS *sms) {
	if (sms->pbnumbers.CountItems()>0) {
		struct pbNum *anItem;
		for (int i=0; (anItem=(struct pbNum*)sms->pbnumbers.ItemAt(i)); i++)
			delete anItem;
		if (!sms->pbnumbers.IsEmpty())
			sms->pbnumbers.MakeEmpty();
	}
}

const char *GSM::parseDate(const char *input) {
	static Pattern *pDate = Pattern::compile("(\\d+)\\/(\\d+)\\/(\\d+),(\\d+):(\\d+):(\\d+)", Pattern::MULTILINE_MATCHING);
	static Matcher *mDate = pDate->createMatcher("");
	static char datestring[21];
	int y,m,d,h,i,s;

	memset(datestring,sizeof(datestring),0);

	mDate->setString(input);
	if (mDate->findFirstMatch()) {
		y = toint(mDate->getGroup(1).c_str());
		m = toint(mDate->getGroup(2).c_str());
		d = toint(mDate->getGroup(3).c_str());
		h = toint(mDate->getGroup(4).c_str());
		i = toint(mDate->getGroup(5).c_str());
		s = toint(mDate->getGroup(6).c_str());
		snprintf(datestring,sizeof(datestring),"%04i/%02i/%02i %02i:%02i:%02i",y,m,d,h,i,s);
	} else {
		snprintf(datestring,sizeof(datestring),_("<none>"));
	}
	return datestring;
}

const char *GSM::encodeText(const char *input) {
	if (fEncoding != ENC_UTF8)
		return input;	// XXX implement!!!

	// below is for UTF8 only
	if (rawUTF8)
		return input;	// for L6

	static BString out;
	BString in = input;
	char tmp[3];

	out = "";
	int j = in.Length();
	for (int i=0; i<j; i++) {
		snprintf(tmp,3,"%X",in[i]);
		out.Append(tmp);
	}
	return out.String();
}

const char *GSM::decodeSMSText(const char *input) {
	if (fEncoding != ENC_UTF8)
		return decodeText(input);
	// guess if input is hex encoded or raw UTF8
	int nothex = 0;
	int i, j;
	j = strlen(input);
	for (i=0;i<j;i++)
		if (input[i]>'F')
			nothex++;
	if (nothex > 0) {
		rawUTF8 = true;	// remember that!
		return input;
	} else
		return decodeText(input);
}

const char *GSM::decodeText(const char *input) {
	static BString out;
	BString in = input;
	char tmp[3];
	char tmp2[2];
	int val;
	int i=0;

	memset(tmp,sizeof(tmp),0);
	memset(tmp2,sizeof(tmp2),0);
	out = "";

	if (fEncoding == ENC_UTF8) {
		while (i<in.Length()) {
			tmp[0] = in[i]; i++;
			tmp[1] = in[i]; i++;
			val = strtol(tmp,NULL,16);
			tmp2[0] = val;
			out.Append(tmp2);
		}
	} else
		// XXX implement!!!
		out = input;
	return out.String();
}

void GSM::dial(const char *num) {
	if (!num)
		return;
	if (strlen(num) == 0)
		return;

	BString cmd;
	cmd = "ATD"; cmd += num; cmd += ";";
	sendCommand(cmd.String());
}

void GSM::hangUp(void) {
	sendCommand("+++");
	sendCommand("ATH");
}

int GSM::guessPBType(const char *num) {
	BString tmp = num;

	// probably has +<intl> prefix
	if (tmp.FindFirst("+") != B_ERROR)
		return PB_INTLPHONE;
	// probably email
	if (tmp.FindFirst("@") != B_ERROR)
		return PB_EMAIL;
	// if all are digits, then it is probably a phone number
	int l = tmp.Length();
	for (int i=0;i<l;i++) {
		if ((tmp[i]<'0')||(tmp[i]>'9'))
			return PB_OTHER;
	}
	return PB_PHONE;
}
