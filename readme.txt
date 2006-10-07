OPCJE:
	BUTTONS:
	[REFRESH] [NEW] [SEND/EDIT] [DELETE] [DIAL]
	STATUS WINDOW:
		- guzik [READ EVERYTHING] (czasochłonne) ?

	OKNO [NOWY SMS]
	- licznik znaków (jaki limit? skad brac? (reimpl. bview i chwytać keydown? blee)

	CALLREG
	- jakie guziki? co mozna oprocz [DIAL]? co na superitem?

	PB SUPERITEM
	- guziki [NEW],[EXPORT/IMPORT/SYNC] dotyczące zaznaczonego

	OKNO [EDIT/NEW]
	- po zapisie rekord na liście _musi_ być odświeżony/dodany
	- zmiana slotu może być fatalna w skutkach!!! (wyłączone do czasu poprawienia)
	- a co jeśli niczego nie wpiszę w oba pola???
	- usunac pbNum->{number} i zobaczyc co jeszcze do poprawki
	- wykrywanie pozostałych atrybutów i ich długości, usunięcie has_phtype, has_address (naiwna detekcja!)
	- włączenie [primary] działa, wyłączenie - nie bardzo, może usunąć z edycji?
	- przy mailing list coś do wyboru numerów (warto...?) np. lista z wielokrotnym wyborem
	- dlaczego trzeba zapisywać dwa razy na L6?

LISTA SMS:
	- superitem SMS - skrócić widok folderów o połowę, a niżej co?
		- edycja/zmiana numeru do centrum SMS? wybór z predef. listy?
- PB:
	- przy dzialaniu pomijac sloty MD i AT

TODO:
- PB: zrobić dopasowywanie jako fallbacks (nawet jeśli zgadujemy że features są obsługiwane)
- PB: poprawić wykrywanie features (liczenie przecinków może być za głupie)
- informacje o SMSC, własnym numerze, sieci (at+cops?); wyszukiwanie sieci (b. dlugo! at+cops=?)
- powiadamianie o przychodzących: at+cnmi (doczytać co i jak)
- zapisywanie/odczyt parametrów z initWindow
- sms PDU mode (odczytywanie wszystkich informacji)
- eksport smsów, eksport/import phonebooków
- ikony uwaga: CVS nie przechowa atrybutów, tylko ZIP
IDEA:
- backup/restore slotów PB
- eksport slotów PB do plików people (peeps)
	- grupowanie zrobione dla pbbynameview zrobić gdzieś ogólniej do korzystania
	- wykrywanie atrybutów (typ numeru) (towrite) zrobić ogólniej do korzystania - dopasowanie do people
		(nie da sie ogólnie, jest zależne od motoroli)
- j/w import/sync (jak dopasowywać? własny atrybut <slot>-<id>?)
- eksport smsów do plików poczty (bemail, treść w plik, od/do/data w atrybuty, coś dziwnego z indeksami robi)

IKONY
- użyć nadesłanych
- ikony typu numeru (telefon/email/dom/praca itd.) - 8-9
- ikony typu smsa (nowy,przeczytany,nowyniewysłany,wysłany) - 4 koperty
- ikona baterii/baterii krytycznej (użyć z resources, gdzie?)
- ikona z wieżą nadawczą (do siły sygnału)

MODULARYZACJA
- wydzielic z klasy GSM klase Engine (wirtualna)
	- Engine: wysylanie komend, odczytywanie statusu itp
	- GSM: manipulacja listami, wyszukiwanie
	- pochodna Engine: ATMotoEngine
	- pod spodem: device jako DeviceSerial
- przepisac listy pb/sms jako zamkniete klasy? elementy list też?
- sortowanie, grupowanie zawartości PB, wykrywanie atrybutów i typu numeru
