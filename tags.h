/* See LICENSE file for copyright and license details. */

typedef struct {
	char *name;
	char *file;
	char *addr;
	char *field;
} Tag;

typedef struct {
	Tag *tags;
	size_t len;
	size_t size;
} Taglist;

Tag create_tag(const char *, const char *, const char *, const char *);
void destroy_tag(Tag);
Taglist new_taglist(size_t);
void destroy_taglist(Taglist);
void append_taglist(Taglist *, Tag);
Tag read_tag(const char *); /* Read tag from one line */
char *print_tag(Tag);
void sort_tags(Taglist *);
