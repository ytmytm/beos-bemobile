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

		void getSMSContent(SMS *sms = NULL);
		void getSMSList(const char *slot);
		int removeSMS(SMS *sms = NULL);
		int changeSMSMemSlot(const char *slot);
		BList *listMemSlotSMS;
		BList *SMSList;

		enum { ENC_UTF8 = 1, ENC_UCS2, ENC_GSM };
		enum { REC_READ=1, REC_UNREAD, STO_SENT, STO_UNSENT, MSG_UNK };

	private:
const char *getSMSMemSlotName(const char *slot);
int getSMSType(const char *type);
void getSMSMemSlots(void);
void updateSMSInfo(void);
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
		BString fDateTime;	// XX unused in GUI
		BString fSMSInfo;
		//
		int fSMSRRead, fSMSRUnread, fSMSSSent, fSMSUSent;
		//
};

#endif
