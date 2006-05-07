- na poczatek wizard:
	- enumeracja portow
	- wykrycie czy i co odpowiada po drugiej stronie
	- zapisanie konfiguracji
- lista rozwijana
	- zostawic tak, jak jest i wszystkie informacje po stronie view
	- usunac 3 level, przeniesc na strone view:
		- nowa lista:
			folder inbox: phone(new,count); sim(new,count)
			outbox: j/w
		- nowe listy, info o wszystkich mozliwych zrodlach
			- wazne dla kontaktow: phone/sim/lastout/lastin etc.
		- w/w - male listy po lewej u gory
		- duza lista u gory: podsumowanie wybranego
		- duze okno w dolnej polowie: duzy widok wybranego
	- na samej gorze duzy tekst co jest wybrane (wielkosc ~header listy)
- jesli mamy widget to na dole statusbar z:
	- telefon; podswietlony numer;bateria;sygnal;dataczas
- make sure that w/o device the app doesn't crash
- upon start make as many gsm objects, as many ports there are, try to probe all at once, add to list (???)

lista:
- dodac kolumne z wartosciami
lista SMS:
- superitem SMS - view z listą folderów:
	<icon_phone> <icon_folder> <nazwa> <unread> <total>
	---
	numery do centrum SMS?
	---
	opcja odswiezenia folderow?
- subitemy - <nazwa folderu> <total>
	- w kolejności: inbox, outbox (unsent), sent, draft, reszta
	Inbox:
	- to, co obecnie jest w glownym SMSview, ale uproszczona tabelka (tylko numer jako "Od")
	Outbox:
	- podobne do Inbox (numer jako "Do")  i jeszcze guzik <Send now> do zaznaczonego oraz "Nowy" (?)
	Sent:
	- podobne do Inbox (inny nagłówek listy, numer jako "do")
	Pozostałe:
	- tylko preview jak Inbox/Sent (numer jako 'numer')
- nowy SMS w nowym oknie, możliwość zapisania do "writable SMS folders" - odczytywać z CPMS

TODO:
- odczytywanie które sloty sms sa writable
- klasa bazowa dla smsviews (z inbox, zrobic outbox, bedzie wiadomo co jest inne)
	smsboxview
- przepisac listy sms jako zamkniete klasy?
- niepotrzebnie nie uaktualniac list w podklasach superitemu sms
