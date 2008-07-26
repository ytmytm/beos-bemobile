SE K800i (i pewnie inne)
- "UTF-8" zamiast "UTF8" w CSCS
- nie wszystkie komendy wracają jako "+CXX: (odp)"
- po przecinkach może być spacja
- problem z przecinkami w nazwach kontaktów
- typ kontaktu zachowany jako "/M"=mobile, "/H"=home, itd.

plan na wersję 0.9:
	- akceptacja vcf/people przeciąganych DnD (równoważne [NEW] na pbview)
	- refaktoryzacja kodu: podział GSM, przepisanie struktur pb* na BMessage, przepisanie list na klasy
	- edycja nazwanych kategorii
		- odczyt/zapis przez konfigurację (razem z ustawieniami serialportu?)
	- BMessage zwrotny zakończonej sukcesem edycji SMS/pbItem/calItem dla odświeżenia listy w oknie
potem:
	- unsolicited messages (incoming message, call), ikonka do deskbara (JAK współdzielić port szeregowy?)
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

- PB:
	- przy dzialaniu pomijac sloty MD i AT

TODO:
- jakoś przetworzyć stronę ze statusem, żeby ikona baterii trafiła gdzie trzeba, co jeszcze tam wstawić?
- PB: zrobić dopasowywanie jako fallbacks (nawet jeśli zgadujemy że features są obsługiwane)
- PB: poprawić wykrywanie features (liczenie przecinków może być za głupie)
- informacje o własnym numerze, sieci (at+cops?); wyszukiwanie sieci (b. dlugo! at+cops=?)
- powiadamianie o przychodzących: at+cnmi (doczytać co i jak)
- replikant(?) w deskbarze do reagowania na incoming message, call, tooltip ze statusem
- zapisywanie/odczyt parametrów z initWindow jako konfiguracji programu
- sms PDU mode (odczytywanie wszystkich informacji)
- eksport/import phonebooków
- ikony uwaga: CVS nie przechowa atrybutów, tylko ZIP
IDEA:
- przyjmowanie DROPów z plików people -> wywołanie dodania do aktualnego slotu
- backup/restore slotów PB
- eksport do vcf
	- encoding qp,utf8!
	- zmiana na vcard v2.1? (jak eksport z motoroli i obsługiwane przez mail-it/vcard2people etc.?)
- j/w import/sync - dopasowywanie wg vcf::uid?
- zamiana pbVal na Message, wszystkie atrybuty kodowane z/na standardowe nazwy -> łatwo znaleźć lub
  stwierdzić nieistnienie, można zapisywać faktyczne wartości

IKONY
- ikony typu numeru (telefon/email/dom/praca itd.) - 8-9
- ikony typu smsa (nowy,przeczytany,nowyniewysłany,wysłany) - 4 koperty
- ikona z wieżą nadawczą (do siły sygnału)

MODULARYZACJA / REFAKTORYZACJA
- wydzielic z klasy GSM klase Engine (wirtualna)
	- Engine: wysylanie komend, odczytywanie statusu itp
	- GSM: manipulacja listami, wyszukiwanie
	- pochodna Engine: ATMotoEngine
	- pod spodem: device jako DeviceSerial
- przepisac listy pb/sms jako zamkniete klasy? elementy list też?
- sortowanie, grupowanie zawartości PB, wykrywanie atrybutów i typu numeru
