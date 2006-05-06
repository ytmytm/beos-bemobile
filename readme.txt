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

