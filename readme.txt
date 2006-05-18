- napisac normalne readme po angielsku
	- ze tylko serialport (whatever is there in /dev/ports - serial/usb/ircomm maybe)
	- ze testowane na C550, powinno dzialac z innymi moto, siemens, etc (sprawdzic liste z kmobiletools)
	- ze nokie tylko niektore, a jak komu zalezy niech sportuje gnokii, bo ja musialbym nokie kupic
	- ze na razie tylko browser
	- ze w razie problemow prosze o kontakt z wyslanym logiem

- zaraz po uruchomieniu okno tymczasowe z informacją który port jest otwierany
  (wlasciwie na razie widac to ladnie w logu)

lista:
- dodać kolumnę z ikonami
- dodac kolumne z wartosciami
lista SMS:
- superitem SMS - view z listą folderów:
	numery do centrum SMS?
- subitemy - <nazwa folderu> <total>
	- w kolejności: inbox, outbox (unsent), sent, draft, reszta
	Inbox:
	- to, co obecnie jest w glownym SMSview, ale uproszczona tabelka (tylko numer jako "Od")
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
- coś pochodnego CLVEasyItem, ale żeby na żądanie pisało się bold (mark new)
- przepisac listy pb/sms jako zamkniete klasy?
- opcje zapisywania smsów, eksportu/importu phonebooka
- phonebook:
	- na superitems (phonebook/call register)
		- lista wszystkich numerow z oznaczeniem slotu? moze tylko informacje o slotach? (raczej 1sze)
	- osobno phonebooks (phone, sim, fixed, etc.)
		- lista nazw pionowa:
			- po wyborze: po prawej wypisanie wszystkich dostępnych numerów i ich rodzajów
			  (gdy dostępne)
		- guziki [REFRESH], [DIAL], [NEW], [DELETE], [EDIT]
	- osobno call register (dialed, missed, received)
		- tylko lista podglądu <ikonarodzaj> <numertel> <nazwa>
		- guzik [DIAL], [DELETE] (o ile to możliwe)

IKONY
- ikona programu 1
- ikony elementów listy wybieralnej ??
- ikony typu numeru (telefon/email) - 8-9
- ikony typu smsa (nowy,przeczytany,nowyniewysłany,wysłany) - 4 koperty
