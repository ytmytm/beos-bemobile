#ifndef _GSM_H
#define _GSM_H

#include <String.h>

struct SMS {
	BString slot;
	int id;
	int type;
	BString number;
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
//
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
	bool writable;
	BList *pb;
};

class BList;
class BSerialPort;

class GSM {
	public:
		GSM (const char *device);
		~GSM();
		//
		void getPhoneData(void);
		void getPhoneStatus(void);
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

		const char *getPBMemSlotName(const char *slot);
		void changePBMemSlot(const char *slot);
		bool hasPBSlot(const char *slot);
		struct pbSlot *getPBSlot(const char *slot);
		BList *listMemSlotPB;

		void getPBList(const char *slot);

		enum { ENC_UTF8 = 1, ENC_UCS2, ENC_GSM };
		enum { REC_READ=1, REC_UNREAD, STO_SENT, STO_UNSENT, MSG_UNK };
		// indirect pNum.type mapping
		enum { PB_PHONE = 1, PB_INTLPHONE, PB_EMAIL, PB_OTHER };
		// direct pNum.kind mapping (for motorola)
		enum { PK_WORK = 0, PK_HOME, PK_MAIN, PK_MOBILE, PK_FAX, PK_PAGER, PK_EMAIL, PK_MAILLIST };

	private:
int getSMSType(const char *type);
void getSMSMemSlots(void);
void getPBMemSlots(void);
const char *decodeText(const char *input);
const char *parseDate(const char *input);

		bool initDevice(const char *device);
		void doneDevice(void);
		//
		void phoneReset(void);
		//
		int sendCommand(const char *cmd, BString *out = NULL, bool debug = false);
		//
		BSerialPort *port;
		//
		sem_id sem;
		bool active;
		// data
		bool isMotorola;
		int	fEncoding;
		// status variables
		bool ringIncoming;
		bool fACPower, fBatPower;
		int fCharge;
		int fSignal;
		BString fManuf, fModel, fGSMVer, fIMEI, fSoftwareVer, fIMSI;
		int fYear, fMonth, fDay, fHour, fMinute, fSecond, fTimezone;
		BString fDateTime;
		BString fSMSInfo;
		//
		int fSMSRRead, fSMSRUnread, fSMSSSent, fSMSUSent;
		//
};

#endif
