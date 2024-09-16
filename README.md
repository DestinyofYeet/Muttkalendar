# prpr-MuttKalender

This is a .ics parser for the email client Mutt

# Usage

The final executable can either be used as a standalone to print out a .ics file or be used in Mutt (or any other client that uses the stdout)

## Standalone
Just feed your ics file to the final executable with the -i flag
```
./ics_muttkalendar -i <yourFileHere>
```

## Mailcap
Add this to your mutt config dir: (Most likely it's`$XDG_CONFIG_HOME/mutt/mailcap` and `$XDG_CONFIG_HOME` is `/home/<user>/.config` in most cases)

```
text/calendar; /usr/local/bin/ics_muttkalendar -c %{charset} -i %s; copiousoutput
```

Change `/usr/local/bin/ics_muttkalendar` to your install path or your compiled binary if you didn't run `sudo make install`

# Build

```
git pull https://gitlab.othr.de/beo45216/prpr-muttkalender
cd prpr-muttkalender
autoreconf -i -f 
mkdir build && cd build
../configure
make
```
One can also specify 
`--enable-debug=yes` or `--enable-asan=yes` at `../configure` to enable debug flags or address sanitization

# Project Status

Values that can be accessed:
- [X] PRODID
- [X] VERSION
- [X] SUMMARY
- [X] DESCRIPTION
- [X] LOCATION
- [ ] UID
- [ ] X-SMT-CATEGORY-COLOR
- [ ] CATEGORIES
- [ ] LAST-MODIFED
- [ ] Transp
- [X] DTSTART
- [X] DTEND
- [ ] X-SMT-MISSING-YEAR
- [ ] DTSTAMP
- [ ] STATUS
- [X] RRULE 
- [X] VALARMS
    - [X] DESCRIPTION
    - [X] ACTION
    - [X] TRIGGER (not parsed, only accessable through char*)
- [ ] EXDATES


