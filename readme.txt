- zaraz po uruchomieniu okno tymczasowe z informacją który port jest otwierany
	- jesli nieznany, to w oknie combo z wyborem portu i OK, QUIT - koniec programu
	- jeśli znany to nazwa portu i info 'please wait', może z informacją o komórce
	- jeśli się nie uda to -- to samo, co nieznany
- na poczatek wizard:
	- enumeracja portow
	- wykrycie czy i co odpowiada po drugiej stronie
	- zapisanie konfiguracji
- jesli mamy widget to na dole statusbar z:
	- telefon; podswietlony numer;bateria;sygnal;dataczas

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
	- getPBSlots
		- oznaczyć zapisywalne sloty
		- oznaczyc sloty PB i call register
	- przy dzialaniu pomijac sloty MD i AT

TODO:
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
