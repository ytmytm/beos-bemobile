lista:
- dodac kolumne z wartosciami (ile numerow, ile smsow, warto???)
lista SMS:
- superitem SMS - view z listą folderów:
	numery do centrum SMS?
- subitemy - <nazwa folderu> <total>
	- w kolejności: inbox, outbox (unsent), sent, draft, reszta
	Outbox:
	- informacja wysłany/niewysłany w osobnej kolumnie?
	- [NEW] - nowe okno do pisania
	- [SEND NOW] - wyślij zaznaczony sms
	Pozostałe:
	- tylko preview jak Inbox/Sent (numer jako 'numer')
- nowy SMS w nowym oknie
	- możliwość zapisania do writable SMS slots, wybór popupmenu
	- wybór adresata z popup (1 - slot pb, 2 - adresat)
- PB:
	- przy dzialaniu pomijac sloty MD i AT

TODO:
- statystyki użycia slotów (total?)
- PB - dodatkowe pola z motoroli (adres, urodziny itd.)
- sms PDU mode (odczytywanie wszystkich informacji)
- coś pochodnego CLVEasyItem, ale żeby na żądanie pisało się bold (mark new)
- preview SMS - uzyc wyszukiwarki numerów (jak w callreg)
- callreg: użyć wyszukiwarki numerów w oknie podglądu smsa - dodać informację od kogo to
	(przenieść wyszukiwarkę we wspólne miejsce, gdzie???)
- przepisac listy pb/sms jako zamkniete klasy?
- opcje eksportu smsów, eksportu/importu phonebooka
- phonebook:
	- na superitems (phonebook/call register)
		- lista wszystkich numerow z oznaczeniem slotu? moze tylko informacje o slotach? (raczej 1sze)
	- osobno phonebooks (phone, sim, fixed, etc.)
		- lista nazw pionowa:
			- po wyborze: po prawej wypisanie wszystkich dostępnych numerów i ich rodzajów
			  (gdy dostępne)
		- guziki [REFRESH], [DIAL], [NEW], [DELETE], [EDIT]
			(NEW/EDIT - brać pod uwagę ograniczenia długości!)
	- osobno call register (dialed, missed, received)
		- tylko lista podglądu <ikonarodzaj> <numertel> <nazwa>
		- guzik [DIAL], [DELETE] (o ile to możliwe)
- zaraz po uruchomieniu okno tymczasowe z informacją który port jest otwierany
  (wlasciwie na razie widac to ladnie w logu)


IKONY
- ikony typu numeru (telefon/email/dom/praca itd.) - 8-9
- ikony typu smsa (nowy,przeczytany,nowyniewysłany,wysłany) - 4 koperty
- ikona baterii/baterii krytycznej
