OPCJE:
	BUTTONS:
	[REFRESH] [NEW] [SEND/EDIT] [DELETE] [DIAL]
	STATUS WINDOW:
		- guzik [READ EVERYTHING] (czasochłonne) ?

	OKNO [NOWY SMS]
	- licznik znaków (jaki limit? skad brac? (reimpl. bview? blee)

	CALLREG
	- [BUG!] - dwuklik na liście otwiera edycje!
	- jakie guziki? co mozna oprocz [DIAL]?

	OKNO [EDIT/NEW]
	- [EDIT] - wypełnić domyślnymi danymi
	- combo z listą slotów (domyślnie ustawiony ten, gdzie kliknięcie)
	- [CANCEL], [SAVE] (sprawdzać replace)
	- pola (default):
		- slot (tylko dla nowych?)
		- numer, nazwa
		- dla !moto - numer musi być \+?\d+, zapisywać z typ=129 (numer) lub 145 (+numer)
	- pola (moto):
		- combo: typ, cbox: primary
			- jeśli typ=mailinglist, to zamiast numeru podaje się "a b c d" - jakiś sposób do wyboru id
			(widget listy z wielokrotnym wyborem zamiast pola tekstowego?)
		- jeśli obsługiwane, to dodatkowe pola (adres, bday, miasto, kraj itd.)
		- pomijać nieznane atrybuty: voiceid,ringer itp. (przy edycji lepiej zachowywać aktualne wartości)
		(pamietane w num->raw, moze niekoniecznie, wystarczy wstawiac puste przecinki dla edycji
		CPBW=x,,,"newname" zmieni tylko nazwe, zeby nadpisac numer trzeba go usunac i dopiero
		zapisac")
	- zapis: rozroznic update od overwrite! kontrolowac dlugosc pol

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
	- lista numerów: ikonki z typem (home/mobile/pager etc.) (wziąć z mobilePhoneTools)
	- nowy slot wirtualny [All numbers]
		- połączone dane z wszystkich
		- w widoku rozwijana lista według nazw, do każdej nazwy kolumna z liczbą numerów
		- do każdego rozwiniętego: jak w poszczególnych
		- guziki jak w poszczególnych

TODO:
- statystyki użycia slotów (total?) na oknach głównych PB i CALLREG i w caption?
- sms PDU mode (odczytywanie wszystkich informacji)
- coś pochodnego CLVEasyItem, ale żeby na żądanie pisało się bold (mark new)
- przepisac listy pb/sms jako zamkniete klasy?
- opcje eksportu smsów, eksportu/importu phonebooka
- ikony uwaga: CVS nie przechowa atrybutów, tylko ZIP

IKONY
- ikony typu numeru (telefon/email/dom/praca itd.) - 8-9
- ikony typu smsa (nowy,przeczytany,nowyniewysłany,wysłany) - 4 koperty
- ikona baterii/baterii krytycznej (użyć z resources, gdzie?)
- ikona z wieżą nadawczą (do siły sygnału)

MODULARYZACJA
- wydzielic z klasy GSM klase Engine (wirtualna)
	- Engine: wysylanie komend, odczytywanie statusu itp
	- GSM: manipulacja listami, wyszukiwanie
	- pochodna Engine: ATMotoEngine
