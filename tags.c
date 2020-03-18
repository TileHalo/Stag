/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tags.h"
#include "util.h"

static void recursive_sort(Taglist *, int, int);
static void swap_tags(Tag *, Tag *);
static int partition_tags(Taglist *, int, int);
static int tagcmp(const Tag, const Tag);


Tag
create_tag(const char *name, const char *file,
		const char *addr, const char *field)
{
	Tag tag;

	tag.name = malloc(sizeof(char)*(strlen(name)+1));
	tag.file = malloc(sizeof(char)*(strlen(file)+1));
	tag.addr = malloc(sizeof(char)*(strlen(addr)+1));
	tag.field = malloc(sizeof(char)*(strlen(field)+1));

	tag.name = strcpy(tag.name, name);
	tag.file = strcpy(tag.file, file);
	tag.addr = strcpy(tag.addr, addr);
	tag.field = strcpy(tag.field, field);

	return tag;
}


void
destroy_tag(Tag tag)
{
	free(tag.name);
	free(tag.file);
	free(tag.addr);
	free(tag.field);
}

Taglist
new_taglist(size_t size)
{
	Taglist tags;

	tags.size = size;
	tags.tags = malloc(sizeof(Tag)*size);
	tags.len = 0;

	return tags;
}
void
destroy_taglist(Taglist tags)
{
	size_t i;

	for (i = 0; i < tags.len; i++) {
		destroy_tag(tags.tags[i]);
	}
	free(tags.tags);
	tags.size = 0;
	tags.len = 0;
}

void
append_taglist(Taglist *tags, Tag tag)
{
	if (tags->len+1 == tags->size) {
		tags->size *= 2;
		tags->tags = realloc(tags->tags,
				tags->size*sizeof(*(tags->tags)));
		if (!tags->tags) {
			die("realloc() failed\n");
		}
	}
	tags->tags[tags->len++] = tag;
}

Tag
read_tag(const char *line)
{
	Tag tag;
	char *info[4];
	const char *l;
	size_t max_len;
	int i, j;

	max_len = strlen(line);
	l = line;

	for (i = 0; i < 4; i++) {
		info[i] = calloc(max_len, sizeof(char));
		for (j = 0; *l; j++) {
			if (*l == '\t') {
				l++;
				break;
			} else {
				info[i][j] = *l;
			}
			l++;
		}
	}

	tag = create_tag(info[0], info[1], info[2], info[3]);

	for (i = 0; i < 4; i++) {
		free(info[i]);
	}

	return tag;
}

char *
print_tag(Tag tag)
{
	char *res;
	size_t nlen, flen, alen, dlen;

	nlen = strlen(tag.name);
	flen = strlen(tag.file);
	alen = strlen(tag.addr);
	dlen = strlen(tag.field);

	res = malloc(sizeof(char)*(nlen+flen+alen+dlen+6));
	sprintf(res, "%s\t%s\t%s;\"\t%s",
			tag.name, tag.file, tag.addr, tag.field);

	return res;
}


void
sort_tags(Taglist *tags)
{
	recursive_sort(tags, 0, tags->len - 1);
}

static void
swap_tags(Tag *a, Tag *b)
{
	Tag t;

	t = *a;
	*a = *b;
	*b = t;
}

static int
partition_tags(Taglist *tags, int low, int high)
{
	Tag pivot;
	int i;
	int j;

	pivot = tags->tags[high];
	i = low - 1;

	for (j = low; j <= high - 1; j++) {
		if (tagcmp(tags->tags[j], pivot) < 0) {
			i++;
			swap_tags(&tags->tags[i], &tags->tags[j]);
		}
	}
	swap_tags(&tags->tags[i+1], &tags->tags[high]);
	return (i + 1);
}


static void
recursive_sort(Taglist *tags, int low, int high)
{
	int pi;
	if (low < high) {
		pi = partition_tags(tags, low, high);

		recursive_sort(tags, low, pi - 1);
		recursive_sort(tags, pi + 1, high);
	}
}


static int
tagcmp(Tag a, Tag b)
{
	return strcmp(a.name, b.name);
}
