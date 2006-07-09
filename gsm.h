#ifndef _GSM_H
#define _GSM_H

#include <String.h>
#include <List.h>

struct pbNum {
	BString slot;
	int id;
	BString number;
	BString name;
	int type;	// number/email/other
	int kind;	// home/work/etc.
	bool primary;
};
//
struct pbSlot {
	BString sname;
	BString name;
	int min;
	int max;
	int items;
	int numlen;
	int namelen;
	bool callreg;
	bool writable;
	BList *pb;
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
class BSerialPort;
class BTextView;
class BWindow;

class GSM {
	public:
		GSM (void);
		~GSM();
		//
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

		const char *getPBMemSlotName(const char *slot);
		bool changePBMemSlot(const char *slot);
		bool hasPBSlot(const char *slot);
		struct pbSlot *getPBSlot(const char *slot);
		BList *listMemSlotPB;

		void getPBList(const char *slot);
		const char *stripLeadNum(const char *num);
		// may return value from pb or input
		struct pbNum *matchNumFromPB(struct pbNum *num);
		// may return value from pb or NULL
		struct pbNum *matchNumFromNum(const char *num);
		void matchNumFromSMS(struct SMS *sms);

		void dial(const char *num);
		void hangUp(void);

		// encodings
		enum { ENC_UTF8 = 1, ENC_UCS2, ENC_GSM };
		// sms TEXT mode states
		enum { REC_READ=1, REC_UNREAD, STO_SENT, STO_UNSENT, MSG_UNK };
		// indirect pNum.type mapping
		enum { PB_PHONE = 1, PB_INTLPHONE, PB_EMAIL, PB_OTHER };
		// direct pNum.kind mapping (for motorola)
		enum { PK_WORK = 0, PK_HOME, PK_MAIN, PK_MOBILE, PK_FAX, PK_PAGER, PK_EMAIL, PK_MAILLIST };
	
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
		// data
		bool isMotorola;
		int	fEncoding;
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
