OPCJE:
	BUTTONS:
	[REFRESH] [NEW] [SEND/EDIT] [DELETE] [DIAL]
	STATUS WINDOW:
		- guzik [READ EVERYTHING] (czasochłonne) ?

	OKNO [NOWY SMS]
	- licznik znaków (jaki limit? skad brac? (reimpl. bview? blee)

	CALLREG
	- jakie guziki? co mozna oprocz [DIAL]?
	- usunac dodatkowe kolumny z listy (tylko #, number, name)?
	(to samo przy edycji SM i niektórych innych slotów)

	OKNO [EDIT/NEW]
	- przetestowac na C550
	- zmiana slotu może być fatalna w skutkach!!! (wyłączone do czasu poprawienia)
	- a co jeśli niczego nie wpiszę w oba pola???
	- usunac pbNum->{number,name} i zobaczyc co jeszcze do poprawki
	- wykrywanie pozostałych atrybutów i ich długości, usunięcie has_phtype, has_address (naiwna detekcja!)
	- włączenie [primary] działa, wyłączenie - nie bardzo, może usunąć z edycji?
	- przy mailing list coś do wyboru numerów (warto...?) np. lista z wielokrotnym wyborem
	- dlaczego trzeba zapisywać dwa razy?

LISTA:
- dodac do nazw kolumne z wartosciami (ile numerow, ile smsow, warto???)

LISTA SMS:
	- superitem SMS - view z listą folderów:
		numery do centrum SMS?
	- lista sms - wyświetlać nazwy? (smslistitem nie zna smsnumbertext(...)content z smsboxview)
- PB:
	- (superitem) lista slotów z informacją o wykorzystaniu; guziki [NEW],[EXPORT/IMPORT/SYNC]
	- przy dzialaniu pomijac sloty MD i AT
	- lista numerów: ikonki z typem (home/mobile/pager etc.) (wziąć z mobilePhoneTools)
	- nowy slot wirtualny [All numbers]
		- połączone dane z wszystkich
		- w widoku rozwijana lista według nazw, do każdej nazwy kolumna z liczbą numerów
		- do każdego rozwiniętego: jak w poszczególnych
		- guziki jak w poszczególnych PB

TODO:
- informacja o sieci at+cops?, wyszukiwanie sieci (dlugo!) at+cops=?
- informacja o przychodzących: at+cnmi (doczytać co i jak)
- zapisywanie/odczyt parametrów z initWindow
- statystyki użycia slotów (total?) na oknach głównych PB i CALLREG i w caption?
- sms PDU mode (odczytywanie wszystkich informacji)
- coś pochodnego CLVEasyItem, ale żeby na żądanie pisało się bold (mark new)
- przepisac listy pb/sms jako zamkniete klasy?
- opcje eksportu smsów, eksportu/importu phonebooka
- ikony uwaga: CVS nie przechowa atrybutów, tylko ZIP
IDEA:
- eksport slotów PB do plików people
- j/w import/sync (jak dopasowywać? własny atrybut <slot>-<id>?)
- eksport smsów do plików poczty

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
