/*
 * Построчная обработка текста с удалением групп повторяющихся пробелов.
 * 
 * char **remove_extra_whitespaces_in_text(const char **text, size_t n);
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
#include <stdbool.h>

#define STD_CHUNK_SIZE 64
#define STD_BUF_SIZE 1024
#define STD_BUF_SIZE_MULT 2

char *remove_extra_whitespaces_in_line(const char *line);
char **remove_extra_whitespaces_in_text(const char **text, size_t n);
void print_text(const char **text, size_t n);
void delete_text(char **text, size_t n);
char *read_line(void);
char **read_text(size_t *n);
bool handle_realloc_line_error(char **line, size_t size);
bool handle_realloc_text_error(char ***text, size_t size, size_t n);
int shutdown_with_error(void);

int main(void) {
	size_t n;
	char **raw_text = read_text(&n);
	if (!raw_text)
		return shutdown_with_error();

	char **corrected_text = remove_extra_whitespaces_in_text(
			(const char **) raw_text, n);
	if (!corrected_text)
		return shutdown_with_error();

	print_text((const char **) corrected_text, n);
	delete_text(corrected_text, n);
	delete_text(raw_text, n);
}

char *remove_extra_whitespaces_in_line(const char *line) {
	assert(line);

	const size_t size = strlen(line) + 1;
	char *corrected_line = malloc(size * sizeof *corrected_line);
	if (!corrected_line)
		return NULL;

	char *run = corrected_line;
	while (*line) {
		if (*line == ' ') {
			*run++ = *line++;
			while (*line == ' ')
				++line;
		}
		else
			*run++ = *line++;
	}
	*run = '\0';

	const size_t new_size = run - corrected_line + 1;
	if (new_size < size) {
		char *tmp = realloc(corrected_line, new_size * sizeof *tmp);
		if (!tmp) {
			free(corrected_line);
			return NULL;
		}
		corrected_line = tmp;
	}
	return corrected_line;
}

char **remove_extra_whitespaces_in_text(const char **text, size_t n) {
	assert(text && n);

	char **corrected_text = malloc(n * sizeof *corrected_text);
	if (!corrected_text)
		return NULL;

	for (size_t i = 0; i != n; ++i)
		if (!(corrected_text[i] = remove_extra_whitespaces_in_line(text[i])))
			delete_text(corrected_text, i);

	return corrected_text;
}

void print_text(const char **text, size_t n) {
	assert(text);
	for (size_t i = 0; i != n; ++i)
		printf("%s", text[i]);
}


void delete_text(char **text, size_t n) {
	assert((text && n) || !n);
	for (size_t i = 0; i != n; ++i)
		free(text[i]);
	free(text);
}

/* returns amount of readed chars */
size_t read_chunk(char *line) {
	assert(line);
	char chunk[STD_CHUNK_SIZE];
	if (!fgets(chunk, STD_CHUNK_SIZE, stdin))
		return 0;
	const size_t len = strlen(chunk);
	memcpy(line, chunk, (len + 1) * sizeof *line);
	return len;
}

/* if error — frees memory, sets pointer to NULL and returns true */
bool handle_realloc_line_error(char **pline, size_t size) {
	assert(pline && *pline && size);
	char *tmp = realloc(*pline, size);
	if (!tmp)
		free(*pline);
	*pline = tmp;
	return !tmp;
}

char *read_line(void) {
	size_t buf_size = STD_BUF_SIZE;
	char *buf = malloc(buf_size * sizeof *buf);
	if (!buf)
		return NULL;

	size_t offset = 0;
	while (true) {
		if (offset + STD_CHUNK_SIZE - 1 > buf_size) {
			buf_size *= STD_BUF_SIZE_MULT;
			if (handle_realloc_line_error(&buf, buf_size * sizeof *buf))
				return NULL;
		}
		const size_t len = read_chunk(buf + offset);
		offset += len;
		if (len + 1 != STD_CHUNK_SIZE || buf[offset] == '\n')
			break;
	}

	if (!offset) {
		if (handle_realloc_line_error(&buf, 1 * sizeof *buf))
			return NULL;
		*buf = '\0';
	}
	else if (offset + 1 < buf_size)
		handle_realloc_line_error(&buf, buf_size * sizeof *buf);

	return buf;
}

/* if error — deletes text, sets pointer to NULL and returns true */
bool handle_realloc_text_error(char ***ptext, size_t size, size_t n) {
	assert(ptext && *ptext && size);
	char **new_text = realloc(*ptext, size);
	if (!new_text)
		delete_text(*ptext, n);
	*ptext = new_text;
	return !new_text;
}

char **read_text(size_t *n) {
	assert(n);
	size_t buf_size = STD_BUF_SIZE;
	char **buf = malloc(buf_size * sizeof *buf);
	if (!buf)
		return NULL;

	*n = 0;
	while ((buf[*n] = read_line())) {
		if (!*buf[(*n)++])
			break;
		if (*n == buf_size) {
			buf_size *= STD_BUF_SIZE_MULT;
			if (handle_realloc_text_error(&buf, buf_size * sizeof *buf, *n))
				return NULL;
		}
	}

	if (ferror(stdin)) {
		delete_text(buf, *n);
		return NULL;
	}

	if (*n && *n < buf_size)
		handle_realloc_text_error(&buf, *n * sizeof *buf, *n);

	return buf;
}

int shutdown_with_error(void) {
	puts("[error]");
	return EXIT_SUCCESS;
}
