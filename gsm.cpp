//
// regexy! najwygodniej
// - sendcommand:
// 		- regexy w sendcommand do wykrycia statusu
//

#include <File.h>
#include <List.h>
#include <SerialPort.h>
#include <String.h>
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

	port = new BSerialPort;
	logFile = new BFile;
}

GSM::~GSM() {
	doneDevice();
	delete_sem(sem);
	// close log file
	logFile->Unset();
}

bool GSM::initDevice(const char *device, bool l = false, bool t = false) {
	BString tmp;

	log = l;
	if (log) {
		logFile->SetTo("/boot/home/bemobile.log",B_ERASE_FILE|B_CREATE_FILE|B_WRITE_ONLY);
		tmp = "opening device:["; tmp += device; tmp += "]\n";
		logFile->Write(tmp.String(),tmp.Length());
	}
	doneDevice();
	if (strlen(device)==0)
		return false;
	port->SetFlowControl(B_HARDWARE_CONTROL);
	if (port->Open(device) <= 0) {
		if (log) {
			tmp = "can't open bserialport\n";
			logFile->Write(tmp.String(),tmp.Length());
		}
		return false;
	} else {
		port->SetDataRate(B_19200_BPS);
		port->SetDataBits(B_DATA_BITS_8);
		port->SetStopBits(B_STOP_BITS_1);
		port->SetParityMode(B_NO_PARITY);
		port->SetTimeout(100000);
		snooze(100000);
		// success?
		active = phoneReset();
	}
	term = t;
	if (term) {
// XXX create a window with terminal log
	}
	return active;
}

void GSM::doneDevice(void) {
	if (active)
		port->Close();
	active = false;
}

// timeout threshold
#define THRSTMOUT 8
#define WLOG { logFile->Write(lll.String(),lll.Length()); }
// return 0 for OK, 1 for other, 2 for error, 3 for sem_error, 4 for tmout, 5 for unopened
int GSM::sendCommand(const char *cmd, BString *out = NULL, bool debug = false) {
	BString tmp;
	BString lll;
	static char buffer[10240];
	int r;
	int status = 0;
	int tmout = 0;

	if (log) {
		lll = "<--["; lll += cmd; lll += "]\n"; WLOG;
	}

	if (!active) {
if (debug) printf("port not open\n");
		if (log) {
			lll = "ERR: port not open\n"; WLOG;
		}
		buffer[0] = '\0';
		return 5;
	}

if (debug) printf("sending:[%s]\n",cmd);
	tmp = cmd; tmp += "\n\r";
	memset(buffer,0,sizeof(buffer));

	if (acquire_sem(sem) != B_NO_ERROR) {
		if (log) {
			lll = "ERR: can't acquire semaphore to serial port\n"; WLOG;
		}
		return 3;
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
			if (out != NULL)		// preserve output only if needed
				tmp.Append(buffer,r);
if (debug) printf("got:[%s]\n",buffer);
			if (strstr(buffer,"RING")) {
printf("RRRIIIIIIIIIIIINGGGGGGG!\b\n");
				ringIncoming = true;
			}
			if (strstr(buffer,"\nOK\r\n")) {
				status = 0;
if (debug) printf("ok!\n");
				break;
			}
			if (strstr(buffer,"ERROR")) {
				status = 2;
if (debug) printf("error!\n");
				break;
			}
			if (strstr(buffer,"\nCONNECT") || strstr(buffer,"\nNO CARRIER\r\n") || strstr(buffer,"\nBUSY\r\n")) {
				status = 1;
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
	if (log) {
		lll = "-->["; lll += tmp; lll += "]\n"; WLOG;
	}
	if (out!=NULL)
		out->SetTo(tmp);
	release_sem(sem);
	if (tmout>0)
		printf("tmout=%i, cmd=[%s]\n",tmout,cmd);
	if (tmout == THRSTMOUT) {
		if (log) {
			lll = "ERR: timeout\n"; WLOG;
		}
		status = 4;
	}
	return status;
}

bool GSM::phoneReset(void) {
	// reset
	active = true;	// lie for a moment
	if (sendCommand("ATZ") != 0) {
		active = false;
		return false;
	}
	sendCommand("ATE0");
	// fetch character sets, default to utf8 or fallback to whatever is there
//	sendCommand("AT+CSCS=?");
	ringIncoming = false;
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
	// Motorola check for extended commands
	isMotorola = (sendCommand("AT+MMGL=?") == 0);
	// enable error reporting, try most verbose mode
	sendCommand("AT+CMEE=1");
	sendCommand("AT+CMEE=2");

	// enable native UTF8 message format, fallback to Unicode and GSM
	fEncoding = ENC_UTF8;
	if (sendCommand("AT+CSCS=\"UTF8\"") > 0) {
		fEncoding = ENC_UCS2;
		if (sendCommand("AT+CSCS=\"UCS2\"") > 0) {
			fEncoding = ENC_GSM;
			sendCommand("AT+CSCS=\"GSM\"");
		}
	}
	// possibly do it later using configuration, and now only query for encodings
	// +CSCS=?
	// +CSCS: ("8859-1","ASCII","GSM","UCS2","UTF8")
	// kmt sends also: AT+MODE=2 if 'smsMode2' is enabled in config
	// if failed: AT+MODE=0 is sent
	// PDU mode test goes from: AT+CMGF=0
	getSMSMemSlots();
	getPBMemSlots();
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
	BString s = slot;
	if (s == "MT") return _("All messages");
	if (s == "IM") return _("Inbox");
	if (s == "OM") return _("Outbox");
	if (s == "BM") return _("Information service");
	if (s == "DM") return _("Drafts");
	return _("Unknown");
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
	pat += "MGR: \"([ \\w]+)\", \"([^\"]+)\", \"([\\d\\/,:]+)\"\r\n(.*)$";
	pat2 += "MGR: \"([ \\w]+)\", \"([^\"]+)\".*\r\n(.*)$";
	pat3 += "MGR: \"([ \\w]+)\".*\r\n(.*)$";
	Pattern *pMsg1 = Pattern::compile(pat.String(), Pattern::MULTILINE_MATCHING);
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
		sms->msg = decodeText(mMsg1->getGroup(4).c_str());	
	} else {
		mMsg2->setString(out.String());
		if (mMsg2->findFirstMatch()) {
			sms->number = mMsg2->getGroup(2).c_str();
			sms->msg = decodeText(mMsg2->getGroup(3).c_str());
		} else {
			mMsg3->setString(out.String());
			if (mMsg3->findFirstMatch()) {
				sms->msg = decodeText(mMsg3->getGroup(2).c_str());
			}
		}
	}
printf("%i,%i,%s,%s\n[%s]\n",sms->id,sms->type,sms->number.String(),sms->date.String(),sms->msg.String());
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
		for (int i=0; (anItem=(struct SMS*)SMSList->ItemAt(i)); i++)
			delete anItem;
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
	if (ret == 0) {
		struct memSlotSMS *sl = getSMSSlot(sms->slot.String());
		if ((sms->type == GSM::REC_UNREAD) || (sms->type == GSM::STO_UNSENT))
			sl->unread--;
		sl->items--;
		sl->msg->RemoveItem(sms);
	}
	//changeMemSlot("MT");						// XXX back to default
	return ret;
}

const char *GSM::getPBMemSlotName(const char *slot) {
	BString s = slot;
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
	return _("Unknown phonebook type");
}

bool GSM::isPBSlotWritable(const char *slot) {
	BString s = slot;
	// add "SD" here (no params available)
	if ((s == "EN") || (s == "DC") || (s == "LD") || (s == "MC") || (s == "RC") || (s == "MD"))
		return false;
	else
		return true;
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
			slot->items = -1;
			slot->pb = new BList;
			listMemSlotPB->AddItem(slot);
			slot->min = slot->max = slot->numlen = slot->namelen = 0;
			if (changePBMemSlot(slot->sname.String()) == false) {
				listMemSlotPB->RemoveItem(slot);
				delete slot;
			}
			else printf("got:%s\n",slot->name.String());
		}
	}
}

bool GSM::changePBMemSlot(const char *slot) {
	static Pattern *pSlot = Pattern::compile("^\\+CPBR: \\((\\d+)-(\\d+)\\),(\\d+),(\\d+)", Pattern::MULTILINE_MATCHING);
	static Matcher *mSlot = pSlot->createMatcher("");

	BString out, cmd = "AT+CPBS=\"";
	cmd += slot; cmd +="\"";

	sendCommand(cmd.String());
	cmd = "AT+CPBR=?";
	// return false on error -> don't add such slot
	if (sendCommand(cmd.String(),&out) == 2)
		return false;

	mSlot->setString(out.String());
	if (mSlot->findFirstMatch()) {
		struct pbSlot *sl = getPBSlot(slot);
		if (sl) {
			sl->min = toint(mSlot->getGroup(1).c_str());
			sl->max = toint(mSlot->getGroup(2).c_str());
			sl->numlen = toint(mSlot->getGroup(3).c_str());
			sl->namelen = toint(mSlot->getGroup(4).c_str());
			printf("got:%s - (%i-%i),%i,%i\n",slot,sl->min,sl->max,sl->numlen,sl->namelen);
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
	if ((rs == 0)||(rs == 4)) {
		pat = isMotorola ? "^\\+MP" : "^\\+CP";
		pat += "BR: (\\d+),\"([^\"]+)\",(\\d+),([^,\r\n]+)";
		if (isMotorola) {
			// 5phtype,6voicetag,7alerttone,8backlight,9primary,10categorynum
			pat += ",(\\d+),(\\d+),(\\d+),(\\d+),(\\d+),(\\d+)";
		}
		Pattern *pNum = Pattern::compile(pat.String(), Pattern::MULTILINE_MATCHING);
		Matcher *mNum = pNum->createMatcher("");
		struct pbNum *num;

		mNum->setString(out.String());
		while (mNum->findNextMatch()) {
			num = new pbNum;
			num->slot = slot;
			num->id = toint(mNum->getGroup(1).c_str());
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
					num->type = PB_OTHER;
			}
			num->name = decodeText(mNum->getGroup(4).c_str());
			num->primary = true;
			num->kind = PK_MAIN;
			if (isMotorola) {
				// extended params
				num->kind = toint(mNum->getGroup(5).c_str());
				if (toint(mNum->getGroup(9).c_str()) == 0)
					num->primary = false;
			}
//			printf("%i:%s:%s:%i:%i,%i\n",num->id,num->number.String(),num->name.String(),num->kind,num->type,num->primary?0:1);
			pbList->AddItem(num);
		}

	}
	sl->items = pbList->CountItems();

	//changePBMemSlot("MT");		// XXX change to default?
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
	}
	return datestring;
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
