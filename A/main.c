/*
 * Построчная обработка текста с удалением групп повторяющихся пробелов.
 * 
 * void strip_lines(char **lines, size_t n);
 * Процедура обработки должна быть оформлена в виде отдельной функции, которой
 * подаётся на вход массив строк, выделенных в динамической памяти, и его
 * длина. На выход функция должна возвращать массив обработанных строк.
 *
 * Керимов А.
 * АПО-13
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define STD_BUF_SIZE 1024
#define STD_BUF_SIZE_MULT 2

void strip_line(char *line);
void strip_lines(char **lines, size_t n);
void print_lines(char **lines, size_t n);
void delete_lines(char **lines, size_t n);
char *read_line(void);
char **read_lines(size_t *n);

int main(void) {
	size_t n;
	char **lines = read_lines(&n);
	if (!lines) {
		puts("[error]");
		return EXIT_SUCCESS;
	}

	strip_lines(lines, n);
	print_lines(lines, n);
	delete_lines(lines, n);
}

void strip_line(char *line) {
	assert(line);
	char *run = line;
	while (*run) {
		if (*run == ' ') {
			*line++ = *run++;
			while (*run == ' ')
				++run;
		}
		else
			*line++ = *run++;
	}
	*line = '\0';
}

void strip_lines(char **lines, size_t n) {
	for (size_t i = 0; i != n; ++i)
		strip_line(lines[i]);
}

void print_lines(char **lines, size_t n) {
	for (size_t i = 0; i != n; ++i)
		printf("%s", lines[i]);
}


void delete_lines(char **lines, size_t n) {
	for (size_t i = 0; i != n; ++i)
		free(lines[i]);
	free(lines);
}

/* simplier and better than getline(): returns fit line */
char *read_line(void) {
	size_t buf_size = STD_BUF_SIZE;
	char *buf = malloc(buf_size * sizeof (char));
	if (!buf)
		return NULL;

	if (!fgets(buf, buf_size - 1, stdin)) {
		free(buf);
		return NULL;
	}

	char *peol;
	// maybe line > STD_BUF_SIZE
	while (!(peol = strchr(buf, '\n'))) {
		char *buf2 = malloc(buf_size * sizeof (char));
		if (!buf2) {
			free(buf);
			return NULL;
		}

		if (!fgets(buf2, buf_size - 1, stdin)) {
			free(buf2);
			if (feof(stdin)) {
				// here should have been the EOL if missing
				peol = strchr(buf, '\0') - 1;
				break;
			}
			// else — ferror(stdin)
			free(buf);
			return NULL;
		}

		buf_size *= STD_BUF_SIZE_MULT;
		char *new_buf = realloc(buf, buf_size * sizeof (char));
		if (!new_buf) {
			free(buf2);
			free(buf);
			return NULL;
		}

		buf = new_buf;
		strncat(buf, buf2, buf_size);
		free(buf2);
	}

	// fit line
	buf_size = (size_t) (peol - buf) + 2;
	char *new_buf = realloc(buf, buf_size * sizeof (char));
	if (!new_buf) {
		free(buf);
		return NULL;
	}

	buf = new_buf;
	return buf;
}

char **read_lines(size_t *n) {
	assert(n);
	size_t buf_size = STD_BUF_SIZE;
	char **buf = malloc(buf_size * sizeof (char *));
	if (!buf)
		return NULL;

	*n = 0;
	while ((buf[*n] = read_line())) {
		if (++*n == buf_size) {
			buf_size *= STD_BUF_SIZE_MULT;
			char **new_buf = realloc(buf, buf_size * sizeof (char *));
			if (!new_buf) {
				delete_lines(buf, *n);
				return NULL;
			}

			buf = new_buf;
		}
	}

	if (feof(stdin)) {
		if (*n && *n < buf_size) {
			char **new_buf = realloc(buf, *n * sizeof (char *));
			if (!new_buf) {
				delete_lines(buf, *n);
				return NULL;
			}

			buf = new_buf;
		}

		return buf;
	}

	// else — ferror(stdin)
	delete_lines(buf, *n);
	return NULL;
}
