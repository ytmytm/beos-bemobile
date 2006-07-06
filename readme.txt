OPCJE:
	SMS INBOX
		- guzik [NEW], [DIAL]
	SMS OUTBOX
		- guzik [NEW], [DIAL], [SEND CURRENT]
	SMS DRAFT i pozostałe
		- guzik [NEW], [DIAL]

	OKNO [NOWY SMS]
	- combo z listą slotów writable
	- lista numerów z wybranego slotu do wyboru i wpisania w textedit
	- treść SMSa
	- [OK], [CANCEL], [SEND NOW], [SAVE TO SLOT]

	CALLREG
	- guzik [DIAL], [DELETE]-o ile działa

	PB
	- guzik [DIAL], [DELETE], [EDIT]-na zaznaczone/dwuklik, [NEW]

	OKNO [NOWY NUMER]
	- combo z listą slotów (domyślnie ustawiony ten, gdzie kliknięcie)
	- pola tekstowe: numer, nazwa, combo:typ, cbox: primary (może dodatkowe moto z adresem)
	- [CANCEL], [SAVE] (sprawdzać replace)

LISTA:
- dodac do nazw kolumne z wartosciami (ile numerow, ile smsow, warto???)

LISTA SMS:
	- superitem SMS - view z listą folderów:
		numery do centrum SMS?
	- lista sms - wyświetlać nazwy? (smslistitem nie zna smsnumbertext(...)content z smsboxview)
- PB:
	- (superitem) lista slotów z informacją o wykorzystaniu
	- dodatkowe pola z motoroli (adres, urodziny itd.)
	- przy dzialaniu pomijac sloty MD i AT
	- lista numerów: ikonki z typem (home/mobile/pager etc.)
	- nowy slot wirtualny [All numbers]
		- połączone dane z wszystkich
		- w widoku rozwijana lista według nazw, do każdej nazwy kolumna z liczbą numerów
		- do każdego rozwiniętego: jak w poszczególnych
		- guziki jak w poszczególnych

TODO:
- statystyki użycia slotów (total?)
- sms PDU mode (odczytywanie wszystkich informacji)
- coś pochodnego CLVEasyItem, ale żeby na żądanie pisało się bold (mark new)
- przepisac listy pb/sms jako zamkniete klasy?
- opcje eksportu smsów, eksportu/importu phonebooka

IKONY
- ikony typu numeru (telefon/email/dom/praca itd.) - 8-9
- ikony typu smsa (nowy,przeczytany,nowyniewysłany,wysłany) - 4 koperty
- ikona baterii/baterii krytycznej (użyć z resources, gdzie?)
- ikona z wieżą nadawczą (do siły sygnału)
