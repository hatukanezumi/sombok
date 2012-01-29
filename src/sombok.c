#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "sombok.h"

#define BUFLEN (8192)
static char buf[BUFLEN];
static char *encbuf = NULL;
static unistr_t unistr = { NULL, 0 };
static unichar_t newline_str[] = { 0x000A };

static
unichar_t hextou(unichar_t * str, int len)
{
    size_t i;
    unichar_t c, u = 0;

    for (i = 0; i < len; i++) {
	c = str[i];
	if ((unichar_t) '0' <= c && c <= (unichar_t) '9')
	    u = u * 16 + c - (unichar_t) '0';
	else if ((unichar_t) 'a' <= c && c <= (unichar_t) 'f')
	    u = u * 16 + c - (unichar_t) 'a' + 10;
	else if ((unichar_t) 'A' <= c && c <= (unichar_t) 'F')
	    u = u * 16 + c - (unichar_t) 'A' + 10;
	else
	    return (unichar_t)-1;
    }
    return u;
}

static
unistr_t *parse_string(char *utf8str, size_t len)
{
    unichar_t *buf;
    size_t i, j;

    if (sombok_decode_utf8(&unistr, 0, utf8str, len, 3) == NULL)
	return NULL;
    buf = unistr.str;
    for (i = 0, j = 0; i < unistr.len; i++) {
	if (buf[i] == (unichar_t) '\\') {
	    if (i + 1 < unistr.len) {
		i++;
		switch (buf[i]) {
		case (unichar_t) '0':
		    buf[j] = 0x0000;	/* null */
		    break;
		case (unichar_t) 'a':
		    buf[j] = 0x0007;	/* bell */
		    break;
		case (unichar_t) 'b':
		    buf[j] = 0x0008;	/* back space */
		    break;
		case (unichar_t) 't':
		    buf[j] = 0x0009;	/* horizontal tab */
		    break;
		case (unichar_t) 'n':
		    buf[j] = 0x000A;	/* line feed */
		    break;
		case (unichar_t) 'v':
		    buf[j] = 0x000B;	/* vertical tab */
		    break;
		case (unichar_t) 'f':
		    buf[j] = 0x000C;	/* form feed */
		    break;
		case (unichar_t) 'r':
		    buf[j] = 0x000D;	/* carriage return */
		    break;
		case (unichar_t) 'e':
		    buf[j] = 0x001B;	/* escape */
		    break;
		case (unichar_t) 'x':	/* \xhh */
		    if ((buf[j] = hextou(buf + i + 1, 2)) == -1)
			buf[j] = buf[i];
		    else
			i += 2;
		    break;
		case (unichar_t) 'u':	/* \uhhhh */
		    if ((buf[j] = hextou(buf + i + 1, 4)) == -1)
			buf[j] = buf[i];
		    else
			i += 4;
		    break;
		case (unichar_t) 'U':	/* \Uhhhhhhhh */
		    if ((buf[j] = hextou(buf + i + 1, 8)) == -1)
			buf[j] = buf[i];
		    else
			i += 8;
		    break;
		default:
		    buf[j] = buf[i];
		}
	    } else if (j < i)
		buf[j] = buf[i];
	} else if (j < i)
	    buf[j] = buf[i];
	j++;
    }

    unistr.len = j;
    return &unistr;
}

static
pid_t popen2(const char *cmd, const char *arg, int *ifd, int *ofd)
{
    int ipipe[2], opipe[2], errnum;
    pid_t pid;

    if (pipe(ipipe) != 0 || pipe(opipe) != 0)
	return -1;

    if ((pid = fork()) < 0)
	return -1;
    if (pid == 0) {
	close(ipipe[1]);
	dup2(ipipe[0], 0);
	close(opipe[0]);
	dup2(opipe[1], 1);

	execl(SHELL_PROGRAM, SHELL_NAME, "-c", cmd, SHELL_NAME, arg, NULL);
	errnum = errno;
	perror("execl");
	exit(errnum);
    }

    *ifd = ipipe[1];
    *ofd = opipe[0];
    return pid;
}

static
gcstring_t *format_SHELL(linebreak_t * lbobj, linebreak_state_t state,
			 gcstring_t * gcstr)
{
    size_t len;
    int ifd = 0, ofd = 1;
    char *statestr;
    gcstring_t *ret;

    switch (state) {
    case LINEBREAK_STATE_SOT:
	statestr = "sot";
	break;
    case LINEBREAK_STATE_SOP:
	statestr = "sop";
	break;
    case LINEBREAK_STATE_SOL:
	statestr = "sol";
	break;
    case LINEBREAK_STATE_LINE:
	statestr = "";
	break;
    case LINEBREAK_STATE_EOL:
	statestr = "eol";
	break;
    case LINEBREAK_STATE_EOP:
	statestr = "eop";
	break;
    case LINEBREAK_STATE_EOT:
	statestr = "eot";
	break;
    default:
	lbobj->errnum = EINVAL;
	return NULL;
    }
    if ((encbuf = sombok_encode_utf8(encbuf, &len, 0, (unistr_t *)gcstr))
	== NULL) {
	lbobj->errnum = errno;
	return NULL;
    }

    popen2(lbobj->format_data, statestr, &ifd, &ofd);
    write(ifd, encbuf, len);
    close(ifd);
    if ((len = read(ofd, buf, BUFSIZ)) == -1) {
	lbobj->errnum = errno;
	close(ofd);
	return NULL;
    }
    if (close(ofd) == -1) {
	lbobj->errnum = errno;
	return NULL;
    }

    if (len == 0)
	unistr.len = 0;
    else if (sombok_decode_utf8(&unistr, 0, buf, len, 3) == NULL) {
	lbobj->errnum = errno;
	return NULL;
    }

    if ((ret = gcstring_newcopy(&unistr, lbobj)) == NULL) {
	lbobj->errnum = errno ? errno : ENOMEM;
	return NULL;
    }
    return ret;
}

int main(int argc, char **argv)
{
    linebreak_t *lbobj;
    size_t i, j, len;
    gcstring_t **lines;
    char *outfile = NULL;
    FILE *ifp, *ofp;
    int errnum;
    unistr_t newline;

    lbobj = linebreak_new(NULL);

    lbobj->colmax = 76.0;
    lbobj->charmax = 998;
    newline.str = newline_str;
    newline.len = 1;
    linebreak_set_newline(lbobj, &newline);
    linebreak_set_format(lbobj, linebreak_format_SIMPLE, NULL);
    linebreak_set_sizing(lbobj, linebreak_sizing_UAX11, NULL);

    for (i = 1; i < argc; i++) {
	if (argv[i][0] == '-' && argv[i][1] == '-') {
	    if (argv[i][2] == '\0') {
		i++;
		break;
	    } else if (strcmp(argv[i] + 2, "colmax") == 0)
		lbobj->colmax = atof(argv[++i]);
	    else if (strcmp(argv[i] + 2, "colmin") == 0)
		lbobj->colmin = atof(argv[++i]);
	    else if (strcmp(argv[i] + 2, "charmax") == 0)
		lbobj->charmax = atol(argv[++i]);
	    else if (strcmp(argv[i] + 2, "newline") == 0 && i + 1 < argc) {
		i++;
		if (parse_string(argv[i], strlen(argv[i])) == NULL) {
		    errnum = errno;
		    perror("parse_string");
		    linebreak_destroy(lbobj);
		    exit(errnum);
		}
		linebreak_set_newline(lbobj, &unistr);
	    } else if (strcmp(argv[i] + 2, "complex-breaking") == 0)
		lbobj->options |= LINEBREAK_OPTION_COMPLEX_BREAKING;
	    else if (strcmp(argv[i] + 2, "no-complex-breaking") == 0)
		lbobj->options &= ~LINEBREAK_OPTION_COMPLEX_BREAKING;
	    else if (strcmp(argv[i] + 2, "context") == 0 && i + 1 < argc) {
		i++;
		if (strcasecmp(argv[i], "EASTASIAN") == 0)
		    lbobj->options |= LINEBREAK_OPTION_EASTASIAN_CONTEXT;
		else
		    lbobj->options &= ~LINEBREAK_OPTION_EASTASIAN_CONTEXT;
	    } else if (strcmp(argv[i] + 2, "hangul-as-al") == 0)
		lbobj->options |= LINEBREAK_OPTION_HANGUL_AS_AL;
	    else if (strcmp(argv[i] + 2, "no-hangul-as-al") == 0)
		lbobj->options &= ~LINEBREAK_OPTION_HANGUL_AS_AL;
	    else if (strcmp(argv[i] + 2, "legacy-cm") == 0)
		lbobj->options |= LINEBREAK_OPTION_LEGACY_CM;
	    else if (strcmp(argv[i] + 2, "no-legacy-cm") == 0)
		lbobj->options &= ~LINEBREAK_OPTION_LEGACY_CM;
	    else if (strcmp(argv[i] + 2, "nonstarter") == 0 && i + 1 < argc) {
		i++;
		if (strcasecmp(argv[i], "LOOSE") == 0)
		    lbobj->options |= LINEBREAK_OPTION_NONSTARTER_LOOSE;
		else
		    lbobj->options &= ~LINEBREAK_OPTION_NONSTARTER_LOOSE;
	    } else if (strcmp(argv[i] + 2, "virama-as-joiner") == 0)
		lbobj->options |= LINEBREAK_OPTION_VIRAMA_AS_JOINER;
	    else if (strcmp(argv[i] + 2, "no-virama-as-joiner") == 0)
		lbobj->options &= ~LINEBREAK_OPTION_VIRAMA_AS_JOINER;
	    else if (strcmp(argv[i] + 2, "format-func") == 0 &&
		       i + 1 < argc) {
		i++;
		if (strcasecmp(argv[i], "NONE") == 0)
		    linebreak_set_format(lbobj, NULL, NULL);
		else if (strcasecmp(argv[i], "SIMPLE") == 0)
		    linebreak_set_format(lbobj, linebreak_format_SIMPLE,
					 NULL);
		else if (strcasecmp(argv[i], "NEWLINE") == 0)
		    linebreak_set_format(lbobj, linebreak_format_NEWLINE,
					 NULL);
		else if (strcasecmp(argv[i], "TRIM") == 0)
		    linebreak_set_format(lbobj, linebreak_format_TRIM,
					 NULL);
		else
		    linebreak_set_format(lbobj, format_SHELL, argv[i]);
	    } else if (strcmp(argv[i] + 2, "prep-func") == 0 &&
		       i + 1 < argc) {
		i++;
		if (strcasecmp(argv[i], "NONE") == 0)
		    linebreak_add_prep(lbobj, NULL, NULL);
		else if (strcasecmp(argv[i], "BREAKURI") == 0)
		    linebreak_add_prep(lbobj, linebreak_prep_URIBREAK, "");
		else if (strcasecmp(argv[i], "NONBREAKURI") == 0)
		    linebreak_add_prep(lbobj, linebreak_prep_URIBREAK,
				       NULL);
		else {
		    fprintf(stderr, "unknown option value: %s\n", argv[i]);
		    linebreak_destroy(lbobj);
		    exit(1);
		}
	    } else if (strcmp(argv[i] + 2, "sizing-func") == 0 &&
		       i + 1 < argc) {
		i++;
		if (strcasecmp(argv[i], "NONE") == 0)
		    linebreak_set_sizing(lbobj, NULL, NULL);
		else if (strcasecmp(argv[i], "UAX11") == 0)
		    linebreak_set_sizing(lbobj, linebreak_sizing_UAX11,
					 NULL);
		else {
		    fprintf(stderr, "unknown option value: %s\n", argv[i]);
		    linebreak_destroy(lbobj);
		    exit(1);
		}
	    } else if (strcmp(argv[i] + 2, "urgent-func") == 0 &&
		       i + 1 < argc) {
		i++;
		if (strcasecmp(argv[i], "NONE") == 0)
		    linebreak_set_urgent(lbobj, NULL, NULL);
		else if (strcasecmp(argv[i], "ABORT") == 0)
		    linebreak_set_urgent(lbobj, linebreak_urgent_ABORT,
					 NULL);
		else if (strcasecmp(argv[i], "FORCE") == 0)
		    linebreak_set_urgent(lbobj, linebreak_urgent_FORCE,
					 NULL);
		else {
		    fprintf(stderr, "unknown option value: %s\n", argv[i]);
		    linebreak_destroy(lbobj);
		    exit(1);
		}
	    } else if (strcmp(argv[i] + 2, "eawidth") == 0 && i + 1 < argc) {
		char *p, *q, *codes, *propname = "";
		size_t j;
		propval_t propval = PROP_UNKNOWN;
		unichar_t beg, end, c;

		i++;
		p = argv[i];
		while ((q = strchr(p, '=')) != NULL) {
		    *q = '\0';
		    codes = p;
		    propname = q + 1;
		    if ((q = strchr(propname, ',')) != NULL) {
			*q = '\0';
			p = q + 1;
			while (*p == ' ' || *p == '\t') p++;
		    } else while (*p) p++;

		    for (j = 0; linebreak_propvals_EA[j] != NULL; j++)
			if (strcasecmp(linebreak_propvals_EA[j], propname)
			    == 0) {
			    propval = j;
			    break;
			}
		    if ((q = strchr(codes, '-')) != NULL) {
			*q = '\0';
			beg = (unichar_t)strtoul(codes, NULL, 16);
			end = (unichar_t)strtoul(q + 1, NULL, 16);
			if (end < beg) {
			    c = beg;
			    beg = end;
			    end = c;
			}
		    } else
			beg = end = (unichar_t)strtoul(codes, NULL, 16);

		    for (c = beg; c <= end; c++)
			linebreak_update_eawidth(lbobj, c, propval);
		}
	    } else if (strcmp(argv[i] + 2, "lbclass") == 0 && i + 1 < argc) {
		char *p, *q, *codes, *propname = "";
		size_t j;
		propval_t propval = PROP_UNKNOWN;
		unichar_t beg, end, c;

		i++;
		p = argv[i];
		while ((q = strchr(p, '=')) != NULL) {
		    *q = '\0';
		    codes = p;
		    propname = q + 1;
		    if ((q = strchr(propname, ',')) != NULL) {
			*q = '\0';
			p = q + 1;
			while (*p == ' ' || *p == '\t') p++;
		    } else while (*p) p++;

		    for (j = 0; linebreak_propvals_LB[j] != NULL; j++)
			if (strcasecmp(linebreak_propvals_LB[j], propname)
			    == 0) {
			    propval = j;
			    break;
			}
		    if ((q = strchr(codes, '-')) != NULL) {
			*q = '\0';
			beg = (unichar_t)strtoul(codes, NULL, 16);
			end = (unichar_t)strtoul(q + 1, NULL, 16);
			if (end < beg) {
			    c = beg;
			    beg = end;
			    end = c;
			}
		    } else
			beg = end = (unichar_t)strtoul(codes, NULL, 16);

		    for (c = beg; c <= end; c++)
			linebreak_update_lbclass(lbobj, c, propval);
		}
	    } else if (strcmp(argv[i] + 2, "version") == 0) {
		printf(PACKAGE_NAME " " PACKAGE_VERSION "\n");
		linebreak_destroy(lbobj);
		exit(0);
	    } else if (strcmp(argv[i] + 2, "sea-support") == 0) {
		printf("%s\n", linebreak_southeastasian_supported ?
		       linebreak_southeastasian_supported : "none");
		linebreak_destroy(lbobj);
		exit(0);
	    } else {
		fprintf(stderr, "unknown option: %s\n", argv[i]);
		linebreak_destroy(lbobj);
		exit(1);
	    }
	} else if (argv[i][0] == '-' && argv[i][1] != '\0' &&
		   argv[i][2] == '\0') {
	    switch (argv[i][1]) {
	    case 'o':
		if (i + 1 < argc) {
		    i++;
		    outfile = argv[i];
		    break;
		}
	    default:
		fprintf(stderr, "Unknown optoion %s\n", argv[i]);
		linebreak_destroy(lbobj);
		exit(1);
	    }
	} else
	    break;
    }

    if (outfile == NULL)
	ofp = stdout;
    else if ((ofp = fopen(outfile, "wb")) == NULL) {
	errnum = errno;
	perror(outfile);
	linebreak_destroy(lbobj);
	exit(errnum);
    }

    if (argc <= i) {
	len = fread(buf, sizeof(char), BUFLEN - 1, stdin);
	if (len <= 0 && errno) {
	    errnum = errno;
	    perror("(stdin)");
	    linebreak_destroy(lbobj);
	    exit(errnum);
	}
	if (len == 0)
	    unistr.len = 0;
	else if (sombok_decode_utf8(&unistr, 0, buf, len, 3) == NULL) {
	    errnum = errno;
	    perror("decode_utf8");
	    linebreak_destroy(lbobj);
	    exit(errnum);
	}

	lines = linebreak_break(lbobj, &unistr);
	if (lbobj->errnum == LINEBREAK_ELONG) {
	    fprintf(stderr, "Excessive line was found\n");
	    free(unistr.str);
	    linebreak_destroy(lbobj);
	    exit(LINEBREAK_ELONG);
	} else if (lbobj->errnum) {
	    errno = errnum = lbobj->errnum;
	    perror("linebreak_break");
	    linebreak_destroy(lbobj);
	    exit(errnum);
	}

	for (j = 0; lines[j] != NULL; j++) {
	    if (lines[j]->str != NULL) {
		if ((encbuf = sombok_encode_utf8(encbuf, &len, 0,
						 (unistr_t *)(lines[j])))
		    == NULL) {
		    errnum = errno;
		    perror("encode_utf8");
		    linebreak_destroy(lbobj);
		    exit(errnum);
		}
		fwrite(encbuf, sizeof(char), len, ofp);
	    }
	    gcstring_destroy(lines[j]);
	}
	free(lines);
    } else {
	for (; i < argc; i++) {
	    if (argv[i][0] == '-' && argv[i][1] == '\0')
		ifp = stdin;
	    else if ((ifp = fopen(argv[i], "rb")) == NULL) {
		errnum = errno;
		perror(argv[i]);
		linebreak_destroy(lbobj);
		exit(errnum);
	    }

	    len = fread(buf, sizeof(char), BUFLEN - 1, ifp);
	    if (len <= 0 && errno) {
		errnum = errno;
		perror("fread");
		linebreak_destroy(lbobj);
		exit(errnum);
	    }
	    if (len == 0)
		unistr.len = 0;
	    else if (sombok_decode_utf8(&unistr, 0, buf, len, 3) == NULL) {
		errnum = errno;
		perror("decode_utf8");
		linebreak_destroy(lbobj);
		exit(errnum);
	    }
	    fclose(ifp);

	    lines = linebreak_break_partial(lbobj, &unistr);
	    if (lbobj->errnum == LINEBREAK_ELONG) {
		fprintf(stderr, "Excessive line was found\n");
		free(unistr.str);
		linebreak_destroy(lbobj);
		exit(LINEBREAK_ELONG);
	    } else if (lbobj->errnum) {
		errno = errnum = lbobj->errnum;
		perror("linebreak_break_partial");
		linebreak_destroy(lbobj);
		exit(errnum);
	    }

	    for (j = 0; lines[j] != NULL; j++) {
		if (lines[j]->str != NULL) {
		    if ((encbuf = sombok_encode_utf8(encbuf, &len, 0,
						     (unistr_t *)(lines[j])))
			== NULL) {
			errnum = errno;
			perror("encode_utf8");
			linebreak_destroy(lbobj);
			exit(errnum);
		    }
		    fwrite(encbuf, sizeof(char), len, ofp);
		}
		gcstring_destroy(lines[j]);
	    }
	    free(lines);
	}
	lines = linebreak_break_partial(lbobj, NULL);
	if (lbobj->errnum == LINEBREAK_ELONG) {
	    fprintf(stderr, "Excessive line was found\n");
	    free(unistr.str);
	    linebreak_destroy(lbobj);
	    exit(LINEBREAK_ELONG);
	} else if (lbobj->errnum) {
	    errno = errnum = lbobj->errnum;
	    perror("linebreak_break_partial");
	    linebreak_destroy(lbobj);
	    exit(errnum);
	}

	for (j = 0; lines[j] != NULL; j++) {
	    if (lines[j]->str != NULL) {
		if ((encbuf = sombok_encode_utf8(encbuf, &len, 0,
						 (unistr_t *)(lines[j])))
		    == NULL) {
		    errnum = errno;
		    perror("encode_utf8");
		    linebreak_destroy(lbobj);
		    exit(errnum);
		}
		fwrite(encbuf, sizeof(char), len, ofp);
	    }
	    gcstring_destroy(lines[j]);
	}
	free(lines);
    }

    fclose(ofp);
    free(encbuf);
    free(unistr.str);
    linebreak_destroy(lbobj);
    exit(0);
}
