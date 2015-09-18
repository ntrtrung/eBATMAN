# This is a simple Makefile made in 5 minutes.
#   libftp 1.0 (stable) Makefile

# How to run:
# $ make
# # make install
# $ make clean     <- clean
# # make uninstall <- uninstall


CC= gcc
CFLAGS= -c -Wall
SOURCES_A=ftp_str.c ftpauth.c ftpconn.c ftperrors.c ftpquit.c ftpnoop.c
SOURCES_F=ftpcwd.c ftpdel.c ftprmd.c ftpsystem.c ftpwd.c ftpmkd.c ftprename.c
SOURCES_S=ftpasv.c ftpget.c ftplist.c ftpnlist.c ftptype.c ftput.c
AR= ar


all: $(SOURCES_A) $(SOURCES_F) $(SOURCES_S)
	$(CC) *.c $(CFLAGS)
	
install:
	$(AR) rc libOftp.a *.o
	
uninstall:
	rm /usr/lib/libOftp.a
	rm -R /usr/include/liboftp

.PHONY: clean

clean:
	rm -f *.o
	rm -f *~ 
	rm -f *.a
