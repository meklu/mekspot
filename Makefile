mekspot : mekspot.c
	$(CC) $< -o $@ $(shell pkg-config --cflags-only-I --libs dbus-1)
