#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#define STD_BUF_SIZE 1024
#define STD_BUF_SIZE_MULT 2

typedef int data_t;
#define FORMAT_DATA_T "%d"

static inline __attribute__((always_inline)) size_t maxlu(size_t a, size_t b) {
	return a > b ? a : b;
}

typedef struct {
	size_t dimension;
	data_t *components;
} vector_t;

typedef enum {
	NONE,
	PLUS,
	MINUS,
	MULTIPLY
} operator_t;

vector_t create_vector(size_t dimension) {
	vector_t vector = {dimension, NULL};
	if (dimension)
		vector.components = malloc(dimension * sizeof (data_t));
	return vector;
}

bool alloc_vector(vector_t *vector) {
	assert(vector);
	vector->dimension = STD_BUF_SIZE;
	vector->components = malloc(vector->dimension * sizeof (data_t));
	return vector->components;
}

bool resize_vector(vector_t *vector, size_t new_dimension) {
	assert(vector && vector->dimension);
	data_t *new_components = realloc(vector->components,
	                                 new_dimension * sizeof (data_t));
	if (!new_components)
		return false;
	vector->dimension = new_dimension;
	vector->components = new_components;
	return true;
}

bool expand_vector(vector_t *vector) {
	size_t dimension = maxlu(vector->dimension * STD_BUF_SIZE_MULT,
	                         STD_BUF_SIZE);
	return resize_vector(vector, dimension);
}

bool fit_vector(vector_t *vector, size_t dimension) {
	assert(vector && vector->components);
	assert(dimension && dimension <= vector->dimension);
	return resize_vector(vector, dimension);
}

void delete_vector(vector_t *vector) {
	free(vector->components);
	*vector = create_vector(0);
}


bool read_num(data_t *num) {
	return scanf(FORMAT_DATA_T, num) == 1;
}

char read_char(void) {
	char c;
	while ((c = getchar()) == ' ' || c == '\n')
		;
	return c;
}

bool read_vector(vector_t *vector) {
	assert(vector);
	char c = read_char();
	if (c != '{')
		return false;
	size_t i = 0;
	while (true) {
		data_t num;
		if (!read_num(&num))
			return false;
		if (i == vector->dimension && !expand_vector(vector))
			return false;
		vector->components[i++] = num;
		if ((c = read_char()) == '}')
			break;
		if (c != ',')
			return false;
	}
	return fit_vector(vector, i);
}

void write_vector(const vector_t *vector) {
	putchar('{');
	for (size_t i = 0; i + 1 < vector->dimension; ++i) {
		printf(FORMAT_DATA_T, vector->components[i]);
		putchar(',');
	}
	printf(FORMAT_DATA_T, vector->components[vector->dimension - 1]);
	putchar('}');
}

const data_t *max_min_dim(
		const vector_t *a, const vector_t *b, 
		size_t *max_dimension, size_t *min_dimension,
		data_t *sign
) {
	if (a->dimension > b->dimension) {
                *max_dimension = a->dimension;
                *min_dimension = b->dimension;
		*sign = 1;
                return a->components;
        }
        else {
                *max_dimension = b->dimension;
                *min_dimension = a->dimension;
		*sign = -1;
                return b->components;
        }

}

typedef data_t (*f_t)(data_t, data_t);

data_t base_add(data_t a, data_t b) {
	return a + b;
}

data_t base_sub(data_t a, data_t b) {
	return a - b;
}

data_t tail_add(data_t sign, data_t d) {
	return d;
}

data_t tail_sub(data_t sign, data_t d) {
	return sign * d;
}

vector_t _add_vectors(const vector_t *a, const vector_t *b, f_t base, f_t tail) {
        assert(a && b);
        size_t max_dimension, min_dimension;
        const data_t *components_of_max;
        data_t sign;
        components_of_max = max_min_dim(a, b, &max_dimension, &min_dimension, &sign);
        vector_t c = create_vector(max_dimension); // check this
        for (size_t i = 0; i != min_dimension; ++i)
                c.components[i] = base(a->components[i], b->components[i]);
        for (size_t i = min_dimension; i != max_dimension; ++i)
                c.components[i] = tail(sign, components_of_max[i]);
        return c;
}

vector_t add_vectors(const vector_t *a, const vector_t *b) {
	return _add_vectors(a, b, base_add, tail_add);
}

vector_t subtract_vectors(const vector_t *a, const vector_t *b) {
	return _add_vectors(a, b, base_sub, tail_sub);
}
