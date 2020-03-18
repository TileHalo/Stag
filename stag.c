/* See LICENSE file for copyright and license details. */

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>

#include "util.h"

#include "tags.h"

typedef struct {
	char *suffix;
	char *name;
	int (*parse) (FILE *, Taglist *, const char *);
} FtCfg;

#include "parsers/parsers.h"
#include "config.h"


static int traverse_directory(char **, Taglist *,
		int(*)(char **, Taglist *));
static int traverse_rec(char **, Taglist *);
static int empty_traverse(char **, Taglist *);
static int write_tags(const char *, Taglist);
static char *path_append(const char *, const char *);


static int
traverse_directory(char **path, Taglist *tags,
		int(*recurse)(char **, Taglist *))
{
	DIR *d;
	FILE *fd;
	struct dirent *dir;
	int res;
	size_t i, len, size;
	char **dirs;

	if (!(*path = realloc(*path, sizeof(**path)*(strlen(*path) + 2)))) {
		die("realloc() failed");
	}
	strcat(*path, "/");
	len = 0;
	size = 10;
	dirs = calloc(size, sizeof(*dirs));

	if (!(d = opendir(*path))) {
		die("opendir() failed %s %d\n",
				*path, errno);
	}
	while ((dir = readdir(d))) {
		if (dir->d_type == DT_DIR && strcmp(dir->d_name, "..") != 0 &&
				strcmp(dir->d_name, ".") != 0) {
			dirs[len++] = path_append(*path, dir->d_name);
		} else if (dir->d_type == DT_REG) {
			char *newpath;

			newpath = path_append(*path, dir->d_name);
			for (i = 0; i < LEN(filetypes); i++) {
				int len, ftlen;

				len = strlen(dir->d_name);
				ftlen = strlen(filetypes[i].suffix);
				res = strcmp(dir->d_name+len-ftlen,
							filetypes[i].suffix);
				if (res == 0) {
					if (!(fd = fopen(newpath, "r"))) {
						die("fopen failed (%d)\n%s",
								errno,
								newpath);
					}

					filetypes[i].parse(fd, tags, newpath);
					fclose(fd);
					break;
				}
			}
			free(newpath);
		}
	}
	if (d) {
		closedir(d);
	}
	for (i = 0; i < len; i++) {
		if ((res = recurse(&dirs[i], tags)) == -1) {
			die("recurse went wrong\n");
		}
		free(dirs[i]);
	}
	free(dirs);
	return 0;
}

static int
traverse_rec(char **path, Taglist *tags)
{
	return traverse_directory(path, tags, &traverse_rec);
}

static int
empty_traverse(char **a, Taglist *b)
{
	(void) a;
	(void) b;

	return 1;
}

static int
write_tags(const char *file, Taglist tags)
{
	FILE *fd;
	char *tag;
	size_t i;

	if (!(fd = fopen(file, "w"))) {
		die("Error opening file (%d)\n", errno);
	}
	for (i = 0; i < tags.len; i++) {
		tag = print_tag(tags.tags[i]);
		fwrite(tag, sizeof(*tag), strlen(tag), fd);
		fputc('\n', fd);
		free(tag);
	}

	return fclose(fd);
}

static char *
path_append(const char *path, const char *suffix)
{
	char *newpath;
	newpath = malloc(sizeof(*newpath) *
			(strlen(path) +
			 strlen(suffix) + 1));
	if (!newpath) {
		die("malloc() failed\n");
	}
	strcpy(newpath, path);
	strcat(newpath, suffix);

	return newpath;
}

int
main(int argc, char **argv)
{
	char *tagfile, *dir;
	int res, recursive, i;
	/* size_t j; */

	Taglist tags;

	tagfile = "tags";
	if (!(dir = malloc(2*sizeof(char)))) {
		die("malloc() failed\n");
	}
	strcpy(dir, ".");
	recursive = 0;
	tags = new_taglist(10);
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-R") == 0) {
			recursive = 1;
		} else if ((strcmp(argv[i], "--file") == 0 ||
				strcmp(argv[i], "-f") == 0) &&
				strcmp(argv[i+1], tagfile) != 0) {
			i++;
			tagfile = malloc(sizeof(char)*(strlen(argv[i])+1));
			if (!tagfile) {
				die("malloc() returned null");
			}
			strcpy(tagfile, argv[i]);
		} else if ((strcmp(argv[i], "--dir") == 0 ||
				strcmp(argv[i], "-d") == 0) &&
				strcmp(argv[i+1], dir) != 0) {
			i++;
			dir = realloc(dir, sizeof(char)*(strlen(argv[i])+1));
			if (!dir) {
				die("malloc() returned null");
			}
			strcpy(dir, argv[i]);
		}
	}

	if (recursive) {
		res = traverse_directory(&dir, &tags, &traverse_rec);
	} else {
		res = traverse_directory(&dir, &tags, &empty_traverse);
	}
	if (res == -1) {
		die("Parsing went wrong");
	}

	if (tags.len > 1) {
		sort_tags(&tags);
	}

	write_tags(tagfile, tags);
	destroy_taglist(tags);
	if (strcmp(tagfile, "tags") != 0) {
		free(tagfile);
	}
	free(dir);
	return 0;
}
