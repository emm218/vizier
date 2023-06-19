#include <clang-c/Index.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CAP 1 /* fine since I only ever need 1 markdown file */

struct vec {
	size_t capacity;
	size_t len;
	FILE *buf[];
};

enum sym_kind {
	FUNCTION,
	TYPE,
};

struct sym_info {
	char *name;
	enum sym_kind kind;
	CXSourceLocation location;
	int has_impl;
	struct sym_info *next;
};

enum CXChildVisitResult visitor(CXCursor, CXCursor, CXClientData);
struct vec *insert(struct vec *, FILE *);
int is_md(const char *const);

void usage(void);

struct sym_info *head = NULL;

int
main(int argc, char **argv)
{
	char **cur, **files;
	unsigned int n_opts;
	struct vec *md_files;
	FILE *tmp;

	CXIndex idx;
	CXTranslationUnit tu;

	if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
		usage();
		return 0;
	}

	// make files point to first non-option argument
	files = argv + (argc - 1);
	while ((files - 1) > argv && *(files - 1)[0] != '-') {
		files--;
	}

	n_opts = files - argv - 1;

	md_files = malloc(sizeof(struct vec) + sizeof(char *) * DEFAULT_CAP);
	if (md_files == NULL) {
		perror(argv[0]);
		return 1;
	}
	md_files->capacity = 1;
	md_files->len = 0;

	idx = clang_createIndex(0, 0);

	for (cur = files; *cur; cur++) {
		if (is_md(*cur)) {
			if ((tmp = fopen(*cur, "r")) == NULL) {
				fprintf(stderr, "%s: failed to open '%s' %s\n",
				    argv[0], *cur, strerror(errno));
				return 1;
			}
			md_files = insert(md_files, tmp);
			if (md_files == NULL) {
				perror(argv[0]);
				return 1;
			}
			continue;
		}
		tu = clang_parseTranslationUnit(idx, *cur,
		    (const char *const *)argv + 1, n_opts, NULL, 0,
		    CXTranslationUnit_SkipFunctionBodies);

		if (tu == NULL) {
			fprintf(stderr, "%s: failed to parse '%s'\n", argv[0],
			    *cur);
			return 1;
		}

		clang_visitChildren(clang_getTranslationUnitCursor(tu), visitor,
		    NULL);
	}

	cur = files;
	free(md_files);
	while (head) {
		free(head);
		head = head->next;
	}

	clang_disposeIndex(idx);
	return 0;
}

enum CXChildVisitResult
visitor(CXCursor c, CXCursor p, CXClientData d)
{
	CXSourceLocation loc;
	enum CXCursorKind kind;

	(void)p;
	(void)d;

	loc = clang_getCursorLocation(c);

	if (clang_Location_isInSystemHeader(loc))
		return CXChildVisit_Continue;

	kind = clang_getCursorKind(c);

	if (kind == CXCursor_TypeRef)
		return CXChildVisit_Continue;

	return CXChildVisit_Recurse;
}

struct vec *
insert(struct vec *v, FILE *f)
{
	if (v->len == v->capacity) {
		if (v->capacity == 0) {
			v = realloc(v, sizeof(*v) + 4 * sizeof(f));
			if (v == NULL)
				goto fail;
			v->capacity = 4;
		} else {
			v = realloc(v,
			    sizeof(*v) + v->capacity * 2 * sizeof(f));
			if (v == NULL)
				goto fail;
			v->capacity *= 2;
		}
	}

	v->buf[v->len] = f;
	v->len++;

	return v;

fail:
	return NULL;
}

int
is_md(const char *const str)
{
	size_t n;

	n = strlen(str);
	return (str[n - 3] == '.') && (str[n - 2] == 'm') &&
	    (str[n - 1] == 'd');
}

void
usage(void)
{
	printf("");
}
