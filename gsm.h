#ifndef _GSM_H
#define _GSM_H

#include <String.h>
#include <List.h>

struct calEvent {
	int id;
	BString title;
	bool timed;
	bool alarm;
	BString start_time;
	BString start_date;
	int dur;
	BString alarm_time;
	BString alarm_date;
	int repeat;
};
//
struct calInfo {
	int maxnum;		// number of stored items (start with id=0)
	int used;		// # of used items
	int title_len;	// length of title field
};
//
struct pbNum {
	BString slot;
	int id;
	BString number;	// XXX removethis (or make link to attr[0])
	int type;		// number/email/other, not used neither shown
	BString raw;	// raw input to preserve unsupported extended attributes for editing
	BList *attr;	// extended attributes, as described in slot->fields
					// attr[0] = number, attr[1] = name required
};
//
struct pbSlot {
	BString sname;
	BString name;
	int min;
	int max;
	bool initialized;
	bool callreg;
	bool writable;
	bool has_phtype;	// XXX remove this
	bool has_address;	// XXX remove this
	BList *pb;			// list of contents
	BList *fields;		// description of extended attributes
};
//
struct pbCombo {		// single combo item
	BString text;
	int v;
};
//
union pbVal {			// single pbNum.attr value
	BString *text;
	bool b;
	int v;
};
//
struct pbField {
	int type;		// GSM::PF_*
	int max;		// maxlen for text, maxvalue for combo
	int offset;		// # of field in {C,M}PBW 0=id, 1=number, 2=type, 3=name,...
	BString name;	// field name (show it to user)
	BList *cb;		// combo items: <name>,<value>
};
//
struct SMS {
	BString slot;
	int id;
	int type;
	BString number;
	BList pbnumbers;
	BString msg;
	BString date;
};
//
struct memSlotSMS {
	BString sname;
	BString name;
	int items;
	int unread;
	bool writable;
	BList *msg;
};

class BFile;
class BList;
class BMessage;
class BSerialPort;
class BTextView;
class BWindow;

class GSM {
	public:
		GSM (void);
		~GSM();
		//
		bool initDevice(BMessage *msg);
		bool initDevice(const char *device, bool l = false, bool t = false);
		void doneDevice(void);
		bool phoneReset(void);
		//
		void getPhoneData(void);
		void getPhoneStatus(void);
		bool statusUpdateEnabled(void)	{ return fStatusUpdateEn; };
		void setStatusUpdate(bool s = false) { fStatusUpdateEn = s; };
		//
		int setDateTime(void);
		//
		bool getACPower(void)	{ return fACPower; };
		bool getBatPower(void)	{ return fBatPower; };
		int getCharge(void)		{ return fCharge; };
		int getSignal(void)		{ return fSignal; };
		const char *getManuf(void)	{ return fManuf.String(); };
		const char *getModel(void)	{ return fModel.String(); };
		const char *getGSMVer(void)	{ return fGSMVer.String(); };
		const char *getIMEI(void)	{ return fIMEI.String(); };
		const char *getDateTime(void) { return fDateTime.String(); };
		const char *getIMSI(void)	{ return fIMSI.String(); };
		const char *getSoftwareVer(void) { return fSoftwareVer.String(); };
		const char *getSMSInfo(void){ updateSMSInfo(); return fSMSInfo.String(); };
		bool hasCalendar(void)	{ return fHasCalendar; };

		const char *getSMSMemSlotName(const char *slot);
		int changeSMSMemSlot(const char *slot);
		bool hasSMSSlot(const char *slot);
		struct memSlotSMS *getSMSSlot(const char *slot);
		void updateSMSInfo(void);
		BList *listMemSlotSMS;

		void getSMSContent(SMS *sms = NULL);
		void getSMSList(const char *slot);
		int removeSMS(SMS *sms = NULL);

		int storeSMS(const char *slot, const char *numbers, const char *msg);
		int sendSMS(const char *numbers, const char *msg);
		int sendSMSFromStorage(const char *slot, int id);

		const char *getPBMemSlotName(const char *slot);
		bool changePBMemSlot(const char *slot);
		bool checkPBMemSlot(struct pbSlot *sl = NULL);
		bool hasPBSlot(const char *slot);
		struct pbSlot *getPBSlot(const char *slot);
		BList *listMemSlotPB;

		void getPBList(const char *slot);
		const char *stripLeadNum(const char *num);
		// these return(add to sms) new struct pbNum
		struct pbNum *matchNumFromPB(struct pbNum *num);
		struct pbNum *matchNumFromNum(const char *num);
		void matchNumFromSMS(struct SMS *sms);

		int removePBItem(struct pbNum *num = NULL);
		int storePBItem(struct pbNum *num = NULL);

		void dial(const char *num);
		void hangUp(void);

		void getCalendarInfo(void);
		int getCalendarEvents(void);
		int getCalendarFreeId(void);
		struct calInfo calSlot;
		BList *listCalEvent;

		// character encodings
		enum { ENC_UTF8 = 1, ENC_UCS2, ENC_GSM };
		// sms TEXT mode states
		enum { REC_READ=1, REC_UNREAD, STO_SENT, STO_UNSENT, MSG_UNK };
		// indirect pNum.type mapping
		enum { PB_PHONE = 1, PB_INTLPHONE, PB_EMAIL, PB_OTHER };
		// direct pNum.kind mapping (for motorola)
		enum { PK_WORK = 0, PK_HOME, PK_MAIN, PK_MOBILE, PK_FAX, PK_PAGER, PK_EMAIL, PK_MAILLIST };
		// phonebook fields' types
		enum { PF_PHONE = 0, PF_PHONEEMAIL, PF_TEXT, PF_BOOL, PF_COMBO };
		// direct alendar book repeat codes mapping
		enum { CAL_NONE = 0, CAL_DAILY, CAL_WEEKLY, CAL_MONTH_ON_DATE, CAL_MONTH_ON_DAY, CAL_YEARLY };

	private:
void logWrite(const char *log);
int getSMSType(const char *type);
void getSMSMemSlots(void);
void getPBMemSlots(void);
bool isPBSlotWritable(const char *slot);
bool isPBSlotCallRegister(const char *slot);
const char *decodeSMSText(const char *input);
const char *decodeText(const char *input);
const char *encodeText(const char *input);
const char *parseDate(const char *input);
void SMSClearNumList(SMS *sms);	// struct sms destructor
int guessPBType(const char *num);
		//
		int sendCommand(const char *cmd, BString *out = NULL, bool debug = false);
		// return codes from sendcommand
		enum { COM_OK = 0, COM_OTHER, COM_ERROR, COM_SEMERROR, COM_TIMEOUT, COM_NOTOPEN };
		//
		BSerialPort *port;
		//
		sem_id sem;
		bool active;
		bool log, term;
		// serial config
		int32 parity, databits, stopbits, flowcontrol, baudrate;
		bool dtr, rts;
		BString initString;
		// data
		bool isMotorola;
		bool fHasCalendar;
		int	fEncoding;
		bool rawUTF8;	// for L6 send/recv raw (non-HEX-encoded) utf8
		// status variables
		bool fStatusUpdateEn;
		bool ringIncoming;
		bool fACPower, fBatPower;
		int fCharge;
		int fSignal;
		BString fManuf, fModel, fGSMVer, fIMEI, fSoftwareVer, fIMSI;
		int fYear, fMonth, fDay, fHour, fMinute, fSecond, fTimezone;
		BString fDateTime;
		BString fSMSInfo;
		//
		BFile *logFile;
		BWindow *logWindow;
		BTextView *logView;
		//
		int fSMSRRead, fSMSRUnread, fSMSSSent, fSMSUSent;
		//
};

#endif
