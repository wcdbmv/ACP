#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STD_BUF_SIZE 256
#define STD_BUF_SIZE_MULT 2

void strip_line(char *line) {
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
	for (int i = 0; i != n; ++i)
		strip_line(lines[i]);
}

void print_lines(char **lines, size_t n) {
	for (int i = 0; i != n; ++i)
		puts(lines[i]);
}

void delete_lines(char **lines, size_t n) {
	for (int i = 0; i != n; ++i)
		free(lines[i]);
	free(lines);
}


char *read_line() {
	size_t buf_size = STD_BUF_SIZE;
	char *buf = malloc(buf_size * sizeof (char));
	if (!buf)
		return NULL;

	if (!fgets(buf, buf_size - 1, stdin)) {
		free(buf);
		return NULL;
	}

	char *peol;
	while (!(peol = strchr(buf, '\n'))) {
		char *buf2 = malloc(buf_size * sizeof (char));
		if (!buf2) {
			free(buf);
			return NULL;
		}

		if (!fgets(buf2, buf_size - 1, stdin)) {
			free(buf2);
			if (feof(stdin))
				break;
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

	*peol = '\0';
	buf_size = (size_t) (peol - buf) + 1;
	char *new_buf = realloc(buf, buf_size * sizeof (char));
	if (!new_buf) {
		// hehe
		free(buf);
		return NULL;
	}

	buf = new_buf;
	return buf;
}

char **read_lines(size_t *n) {
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
		if (*n < buf_size) {
			char **new_buf = realloc(buf, *n * sizeof (char *));
			if (!new_buf) {
				// huhu
				delete_lines(buf, *n);
				return NULL;
			}
			
			buf = new_buf;
		}

		return buf;
	}

	delete_lines(buf, *n);
	return NULL;
}

int main(void) {
	setbuf(stdout, NULL);

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