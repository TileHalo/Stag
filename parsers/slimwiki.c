/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../tags.h"
#include "../util.h"
#include "parsers.h"

static Tag parse_generic_card(const char *, const char *);
static Tag parse_todo(const char *, const char *);
static Tag parse_done(const char *, const char *);
static Tag parse_event(const char *, const char *);
static Tag parse_appointment(const char *, const char *);
static Tag parse_class(const char *, const char *);
static Tag parse_location(const char *, const char *);
static Tag parse_heading(const char *, const char *);
static Tag parse_uid(const char *, const char *, const char *);
static char *addr_from_line(const char *);


int
slimwiki_parse(FILE *fd, Taglist *tags, const char *file)
{
	char *line, *prev;
	Tag tag;

	line = NULL;
	prev = NULL;
	while (getline(&line, 0, fd) != -1) {
		switch (*line) {
		case '#': /* Line starts with heading */
			if (strncmp(line, "## TODO:", 8) == 0) {
				tag = parse_todo(line, file);
			} else if (strncmp(line, "## DONE:", 8) == 0) {
				tag = parse_done(line, file);
			} else if (strncmp(line, "## EVENT:", 9) == 0) {
				tag = parse_event(line, file);
			} else if (strncmp(line, "## APPOINTMENT:", 15) == 0) {
				tag = parse_appointment(line, file);
			} else if (strncmp(line, "## CLASS:", 9) == 0) {
				tag = parse_class(line, file);
			} else if (strncmp(line, "## LOCATION:", 12) == 0) {
				tag = parse_location(line, file);
			} else {
				tag = parse_heading(line, file);
			}
			append_taglist(tags, tag);
			break;
		case '-': /* Parse estimates and uids */
			if (strncmp(line, "- UID:", 6) == 0) {
				append_taglist(tags,
						parse_uid(line, file, prev));
			}
			break;
		default:
			break;
		}
		free(prev);
		prev = line;
		line = NULL;
	}
	if (*line == '#') {
		tag = parse_heading(line, file);
	}
	free(line);
	free(prev);
	return 0;
}

static Tag
parse_generic_card(const char *line, const char *file)
{
	Tag tag;
	const char *l;
	char *addr, *t;
	size_t i;

	addr = addr_from_line(line);
	l = line;
	i = 0;

	while (*l++ != ':' && *l) {
		continue;
	}
	l++;
	while(*l++ != ' ' && *l) {
		i++;
	}
	if (!*l) {
		i++;
		l -= i;
	} else {
		l -= i + 1;
	}

	if (!(t = malloc(sizeof(char)*(i + 2)))) {
		die("malloc() failed");
	}
	strncpy(t, l, i);
	t[i] = 0;

	tag = create_tag(t, file, addr, "EMPTY");
	free(addr);
	free(t);

	return tag;
}

static Tag
parse_todo(const char *line, const char *file)
{
	Tag tag;

	tag = parse_generic_card(line, file);
	strcpy(tag.field, "t");

	return tag;
}

static Tag
parse_done(const char *line, const char *file)
{
	Tag tag;

	tag = parse_generic_card(line, file);
	strcpy(tag.field, "d");

	return tag;
}

static Tag
parse_event(const char *line, const char *file)
{
	Tag tag;

	tag = parse_generic_card(line, file);
	strcpy(tag.field, "e");

	return tag;
}

static Tag
parse_appointment(const char *line, const char *file)
{
	Tag tag;

	tag = parse_generic_card(line, file);
	strcpy(tag.field, "a");

	return tag;
}

static Tag
parse_class(const char *line, const char *file)
{
	Tag tag;

	tag = parse_generic_card(line, file);
	strcpy(tag.field, "c");

	return tag;
}

static Tag
parse_location(const char *line, const char *file)
{
	Tag tag;

	tag = parse_generic_card(line, file);
	strcpy(tag.field, "l");

	return tag;
}

static Tag
parse_heading(const char *line, const char *file)
{
	Tag tag;
	char heading[3];
	const char *l;
	int i;
	char *addr;

	addr = addr_from_line(line);
	l = line;
	i = 0;

	while (*l++ == '#') {
		i++;
	}

	sprintf(heading, "h%d", i);
	tag = create_tag(l, file, addr, heading);
	free(addr);

	return tag;
}
static Tag
parse_uid(const char *line, const char *file, const char *prev)
{
	Tag tag;
	char uid[17];
	const char *l;
	char *addr;

	addr = addr_from_line(prev);
	l = line;

	while (*l++ != ':'){
		continue;
	}
	l++;
	strncpy(uid, l, 16);

	tag = create_tag(uid, file, addr, "uid");
	free(addr);

	return tag;
}

static char *
addr_from_line(const char *line)
{
	char *addr;

	if (!(addr = malloc(sizeof(*addr)*(strlen(line) + 5)))) {
		die("malloc() failed\n");
	}
	strcpy(addr, "/^");
	strcat(addr, line);
	strcat(addr, "$/");
	return addr;
}
