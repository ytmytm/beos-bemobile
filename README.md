
BeMobile 0.9
============

(c) Maciej Witkowiak <ytm@elysium.pl>, <mwitkowiak@gmail.com>
http://members.elysium.pl/ytm/html/beos.html
http://ytm.bossstation.dnsalias.org/html/beos.html

BeMobile is a program to manage contacts and messages stored on your mobile phone.
It has been developed for Motorola C550 and L6 models but should work with any phone that
supports AT command set.

# BeOS

The last version for BeOS was released on 2017-11-03 and is tagged as v0.9

# SCREENSHOTS

![Screenshot](/images/bemobile01.png?raw=true) 
![Screenshot](/images/bemobile02.png?raw=true) 
![Screenshot](/images/bemobile03.png?raw=true) 
![Screenshot](/images/bemobile04.png?raw=true) 
![Screenshot](/images/bemobile05.png?raw=true) 
![Screenshot](/images/bemobile06.png?raw=true) 
![Screenshot](/images/bemobile07.png?raw=true) 
![Screenshot](/images/bemobile08.png?raw=true) 
![Screenshot](/images/bemobile09.png?raw=true) 
![Screenshot](/images/bemobile10.png?raw=true) 
![Screenshot](/images/bemobile11.png?raw=true) 


# COMPATIBILITY

BeMobile works only with phones that can be driven with AT commands. It has been developed
and tested ONLY on Motorola C550 and L6. It has been reported to work with Motorolas: SLVR L7,
RAZR V3, V220 and Nokia Vertu.
It should also work with phones from other vendors, like Siemens, Ericsson and some Nokia phones.

To make a long story short - if SerialConnect can talk with your phone via infra-red link,
RS232 or USB cable, then BeMobile should work too. In case of IR and RS232 you will have to find out
correct parameteres of connection (speed, data bits, etc.).

Phone numbers from SMS and Call Register lists are matched against Phone Books, but you have to
read (open the list) them first. Just 9 last digits are matched, without international prefixes madness
(I send SMS without prefixes and receive them both with and without +48 so it is best to strip them
all). Similarily, number picker in new SMS window is only filled with items from Phone Books that have
been already visited.



# CHANGES
### version 0.9
- ?

### version 0.8
- corrected calendar editing for some phones (like C550)
- date validation and selector for birthday field
- battery ok/critical icon on main status view (this should be redesigned...)

### version 0.7
- battery level should be displayed correctly for Motorola
- new option: setting SMS centre number on main SMS view
- better input validation in calendar events editor
- export selected SMS slots to BeMail-like files into $HOME/mail/SMS/ 
- minor corrections

### version 0.6
- new dialog to edit/enter calendar events
- common ISO date format everywhere (YYYY/MM/DD)
- export selected contacts from 'All contacts by name' to People files (best used with MrPeeps)
- export selected contacts from 'All contacts by name' to vCard (VCF) files, probably needs more testing
- new set of nice, clean BeOS-style icons (thank you, maui team & Ralf Schulke!)

### version 0.5
- fixed phonebook listing for v220 (and possibly some other models) broken in 0.4
- new phonebook page with numbers grouped by name
- phonebook summary displays number of unique entries (by name)
- lists expand upon double-click

### version 0.4
- fixed some bugs (like handling hidden/empty numbers on call register)
- dialog to edit/add new phonebook entries
- support for device-dependent additional attributes (phone number type, address, city, nick, etc.)
- phonebooks summary page
- mark unread/unsent messages
- calendar (datebook) viewer

### version 0.3
- initial window has menu with all possible serial configuration options, useful for real (COM) ports,
  meaningless for USB ACM devices; also with custom initalization string
- you can dial and delete items from phone books

### version 0.2
- support for L6
- some icons for device list
- match phone numbers in SMS preview and Call Register with Phone Book to show names+numbers
- store new SMS on phone, send SMS directly and from memory slots



# OPTIONS

On the initial screen there is a number of settings that you can change. The menu has all available
serial port options (important for COM cable, mostly meaningless for USB). You can also type custom
initialization string. It should be something like
	ATZ - modem reset
	AT&F - restore factory settings
or just AT (which does nothing).
You can also choose if you want to see communications between the phone and your computer.
It is very important for debugging.

On the phone status view there is summary of informations about the phone and a button:
- Set date and time - to set date and time on the phone according to computer clock
Menu items:
- Monitor status - disable this when troubleshooting, it won't fill the log with uninteresting data about phone battery status, network signal strength etc.
- Hangup - allows to break call initiated either from computer or phone

On phonebooks listings you can use buttons to manipulate items or double-click to edit.
If editing returns error check your input - extra spaces, etc. Check also type of item (email, number, etc.)
if extended attributes were detected. E.g. email must have '@' character and this is validated by the
phone.

On my phones there seems no way to disable 'Primary number' flag.

Note that categories are represented as numbers. There is no easy way to fetch category names from
the phone.



# EXPORTING CONTACTS

On the "All contacts by name" page you can export selected contacts to vCard format. The data will be
appended to your /boot/home/people/people.vcf file.

You can also select a contact and export it to People file. It will be saved under
/boot/home/people/BeMobile/ to prevent any damage to your address book. Currently only Motorola phones
extended attributes (nickname, birthday, address, etc.) are saved. Please use MrPeeps instead of People to
browse People files because it handles more attributes (separate home/work address, more phone types,
birthday).
Categories from phone are saved as groups. Currently they are only numerical because there is no easy way
to fetch category names from the phone.



# EXPORTING MESSAGES

On the main SMS slots view you can select a slot and export all its contents to BeMail files. The data will be
saved in your /boot/home/mail/SMS/<name of the slot> folder, each message in a separate file.



# NOKIA

I expect that this will come often - NO, MOST LIKELY IT WILL NOT RUN WITH YOUR NOKIA (YET).
As far as I know, most Nokia phones use something different than plain old modem AT commands.
If you're really interested, you should port gnokii (http://www.gnokii.org) to BeOS, or convince
someone to do it. Myself, I don't have a Nokia phone, so buy me one before asking about compatibility
with Nokias :)



# TROUBLESHOOTING

If the program doesn't work or interprets some data wrong or can't gather data at all, please
send me full information about your phone, link type and (most important) communication log.
The log is by default enabled and saved in your home directory as 'bemobile.log'. Please
disable 'Monitor status' menu option and try all options that don't work as they should.



# HELP/SUPPORT

Do you want to help? I'll be more than happy to receive any donations and reports (both success
and failure).
Do you want to donate? Please use http://www.moneybookers.com service and <mw4096@gmail.com>
(that is my not-often checked email address) as receiver. If you really care, I could cosider opening PayPal account too.


# LICENSE

This code is licensed under permissive MIT License.
