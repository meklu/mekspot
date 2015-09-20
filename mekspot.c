#include <stdio.h>
#include <stdint.h>
#include <dbus/dbus.h>

void prntquotstr(const char *str) {
	int i;
	char c, pc, ps;
	if (str == NULL) {
		fputs("null", stdout);
		return;
	}
	fputs("\"", stdout);
	for (i = 0; str[i] != '\0'; i += 1) {
		c = str[i];
		ps = 1;
		switch (c) {
			case '"':
			case '\\':
				pc = c;
				break;
			case '\n':
				pc = 'n';
				break;
			case '\t':
				pc = 't';
				break;
			case '\f':
				pc = 'f';
				break;
			case '\r':
				pc = 'r';
				break;
			case '\b':
				pc = 'b';
				break;
			default:
				pc = c;
				ps = 0;
		}
		if (ps) {
			fputs("\\", stdout);
		}
		printf("%c", pc);
	}
	fputs("\"", stdout);
}

DBusConnection *cnct(DBusError *err) {
	DBusConnection *conn = NULL;
	int ret;
	conn = dbus_bus_get(DBUS_BUS_SESSION, err);
	if (dbus_error_is_set(err)) {
		fprintf(stderr, "err: Connection Error (%s)\n", err->message);
		dbus_error_free(err);
		return NULL;
	}
	if (!conn) {
		return NULL;
	}
	ret = dbus_bus_request_name(
		conn,
		"org.meklu.mekspot",
		DBUS_NAME_FLAG_REPLACE_EXISTING,
		err
	);
	if (dbus_error_is_set(err)) {
		fprintf(stderr, "err: Name Error (%s)\n", err->message);
		dbus_error_free(err);
		dbus_connection_close(conn);
		return NULL;
	}
	if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
		dbus_connection_close(conn);
		return NULL;
	}
	return conn;
}

/*
 * dbus-send --print-reply --session --dest=org.mpris.MediaPlayer2.spotify /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Get string:'org.mpris.MediaPlayer2.Player' string:'Metadata'
 */

void prntprim(DBusMessageIter *pos) {
	union {
		uint64_t msui64;
		uint32_t msui32;
		int64_t msi64;
		int32_t msi32;
		double msdouble;
	} msn;
	const char *str;
	int argtype;

	argtype = dbus_message_iter_get_arg_type(pos);

	if (argtype == DBUS_TYPE_STRING) {
		dbus_message_iter_get_basic(pos, &str);
		prntquotstr(str);
		str = NULL;
	} else if (argtype == DBUS_TYPE_UINT64) {
		dbus_message_iter_get_basic(pos, &msn.msui64);
		printf("%llu", (long long unsigned int) msn.msui64);
	} else if (argtype == DBUS_TYPE_INT64) {
		dbus_message_iter_get_basic(pos, &msn.msi64);
		printf("%lld", (long long int) msn.msi64);
	} else if (argtype == DBUS_TYPE_UINT32) {
		dbus_message_iter_get_basic(pos, &msn.msui32);
		printf("%d", (unsigned int) msn.msui32);
	} else if (argtype == DBUS_TYPE_INT32) {
		dbus_message_iter_get_basic(pos, &msn.msi32);
		printf("%d", (int) msn.msi32);
	} else if (argtype == DBUS_TYPE_DOUBLE) {
		dbus_message_iter_get_basic(pos, &msn.msdouble);
		printf("%f", msn.msdouble);
	} else {
		fprintf(stderr, "Unknown type '%c'!\n", argtype);
	}
}

int grbsng(DBusConnection *conn) {
	int i;
	DBusMessage *msg;
	DBusMessageIter args;
	DBusPendingCall *pending;
	const char *params[] = {
		"org.mpris.MediaPlayer2.Player",
		"Metadata"
	};

	msg = dbus_message_new_method_call(
		"org.mpris.MediaPlayer2.spotify",
		"/org/mpris/MediaPlayer2",
		"org.freedesktop.DBus.Properties",
		"Get"
	);
	if (msg == NULL) {
		fputs("err: msg == NULL\n", stderr);
		return 0;
	}
	dbus_message_iter_init_append(msg, &args);
	for (i = 0; i < (sizeof(params) / sizeof(params[0])); i += 1) {
		if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &params[i])) {
			fputs("err: Out Of Memory!\n", stderr);
			return 0;
		}
	}
	if (!dbus_connection_send_with_reply(conn, msg, &pending, -1)) {
		fputs("err: Out Of Memory!\n", stderr);
		return 0;
	}
	if (pending == NULL) {
		fputs("err: Out Of Memory!\n", stderr);
		return 0;
	}
	dbus_connection_flush(conn);
	dbus_message_unref(msg);

	{
		const char *idx;
		DBusMessageIter sub, dictsub, artsub;
		int argtype = DBUS_TYPE_INVALID, subhasnext, artsubhasnext;
		dbus_pending_call_block(pending);

		msg = dbus_pending_call_steal_reply(pending);
		if (NULL == msg) {
			fputs("err: Reply Null\n", stderr);
			return 0;
		}
		// free the pending message handle
		dbus_pending_call_unref(pending);

		// read the parameters
		if (!dbus_message_iter_init(msg, &args)) {
			fputs("err: Message has no arguments!\n", stderr);
		}
		argtype = dbus_message_iter_get_arg_type(&args);

		if (argtype != DBUS_TYPE_VARIANT) {
			fputs("err: Argument is not variant!\n", stderr);
			return 0;
		}

		dbus_message_iter_recurse(&args, &sub);
		argtype = dbus_message_iter_get_arg_type(&sub);

		if (argtype != DBUS_TYPE_ARRAY) {
			fputs("err: Argument is not array!\n", stderr);
			return 0;
		}

		/* outer array */
		fputs("{", stdout);

		dbus_message_iter_recurse(&sub, &sub);

		do {
			argtype = dbus_message_iter_get_arg_type(&sub);

			if (argtype != DBUS_TYPE_DICT_ENTRY) {
				fputs("err: Argument is not dict entry!\n", stderr);
				return 0;
			}

			dbus_message_iter_recurse(&sub, &dictsub);
			argtype = dbus_message_iter_get_arg_type(&dictsub);

			if (argtype != DBUS_TYPE_STRING) {
				fputs("err: Argument is not string!\n", stderr);
				return 0;
			}
			/* index */
			dbus_message_iter_get_basic(&dictsub, &idx);
			prntquotstr(idx);
			fputs(":", stdout);
			/* value */
			if (!dbus_message_iter_next(&dictsub)) {
				fputs("err: No value!\n", stderr);
				return 0;
			}
			argtype = dbus_message_iter_get_arg_type(&dictsub);

			if (argtype != DBUS_TYPE_VARIANT) {
				fputs("err: Argument is not variant!\n", stderr);
				return 0;
			}

			dbus_message_iter_recurse(&dictsub, &dictsub);
			argtype = dbus_message_iter_get_arg_type(&dictsub);

			if (argtype == DBUS_TYPE_ARRAY) {
				fputs("[", stdout);
				dbus_message_iter_recurse(&dictsub, &artsub);
				do {
					argtype = dbus_message_iter_get_arg_type(&artsub);

					prntprim(&artsub);
					artsubhasnext = dbus_message_iter_next(&artsub);
					if (artsubhasnext) {
						fputs(",", stdout);
					}
				} while (artsubhasnext);
				fputs("]", stdout);
			} else if (argtype != DBUS_TYPE_INVALID) {
				prntprim(&dictsub);
			} else {
				fputs("err: Argument is neither array nor string!\n", stderr);
				return 0;
			}
			/* separators */
			subhasnext = dbus_message_iter_next(&sub);
			if (subhasnext) {
				fputs(",", stdout);
			}
		} while (subhasnext);

		/* outer array */
		fputs("}\n", stdout);

		if (dbus_message_iter_next(&args)) {
			fputs("err: Message has too many arguments!\n", stderr);
		}
		// free reply and close connection
		dbus_message_unref(msg);
	}
	return 1;
}

int main(void) {
	DBusError err;
	DBusConnection *conn = NULL;
	dbus_error_init(&err);
	conn = cnct(&err);
	if (conn == NULL) {
		fputs("err: Connection Failed\n", stderr);
		return 1;
	}
	if (grbsng(conn) == 0) {
		fputs("err: Song Grabbing Failed\n", stderr);
		dbus_error_free(&err);
		dbus_connection_close(conn);
		return 1;
	}
	dbus_connection_close(conn);
	return 0;
}
