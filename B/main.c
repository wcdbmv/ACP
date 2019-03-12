/*
 * Задача B-7. Программа калькулятор для векторов
 *
 * Керимов А.
 * АПО-13
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define STD_BUF_SIZE 1024
#define STD_BUF_SIZE_MULT 2

typedef int data_t;
#define FORMAT_DATA_T "%d"

static inline __attribute__((always_inline)) size_t maxlu(size_t a, size_t b) {
	return a > b ? a : b;
}

typedef enum {
	SUCCESS = 0,
	ALLOC_FAILURE,
	NOT_A_NUMBER,
	NOT_A_VECTOR,
	NOT_AN_OPERAND,
	NOT_AN_OPERATOR,
	INVALID_FORMAT,
} error_t;

typedef struct {
	size_t dimension;
	data_t *components;
} vector_t;

#define VECTOR_OPEN_BRACKET  '{'
#define VECTOR_CLOSE_BRACKET '}'
#define VECTOR_SEPARATOR     ','

error_t create_vector(vector_t *vector, size_t dimension) {
	assert(vector);
	vector->dimension = dimension;
	vector->components = NULL;
	if (dimension && !(vector->components = malloc(dimension * sizeof (data_t))))
		return ALLOC_FAILURE;
	return SUCCESS;
		
}

void delete_vector(vector_t *vector) {
	assert(vector);
	free(vector->components);
	create_vector(vector, 0);
}

error_t resize_vector(vector_t *vector, size_t new_dimension) {
	assert(vector);
	data_t *new_components = realloc(vector->components, new_dimension * sizeof (data_t));
	if (!new_components)
		return ALLOC_FAILURE;
	vector->dimension = new_dimension;
	vector->components = new_components;
	return SUCCESS;
}

error_t expand_vector(vector_t *vector) {
	size_t dimension = maxlu(vector->dimension * STD_BUF_SIZE_MULT, STD_BUF_SIZE);
	return resize_vector(vector, dimension);
}

error_t fit_vector(vector_t *vector, size_t dimension) {
	assert(vector && vector->components);
	assert(dimension && dimension <= vector->dimension);
	return resize_vector(vector, dimension);
}

error_t sread_number(const char *begin, data_t *number, const char **end) {
	int n_chars = 0;
	*end = begin;
	if (*begin == '+' || *begin == '-')
		return NOT_A_NUMBER;
	if (sscanf(*end, FORMAT_DATA_T "%n", number, &n_chars) < 1)
		return NOT_A_NUMBER;
	*end += n_chars;
	return SUCCESS;
}

static inline __attribute__((always_inline)) void write_number(data_t number) {
	printf(FORMAT_DATA_T, number);
}

void swrite_number(char *begin, data_t number, char **end) {
	size_t size = sprintf(begin, FORMAT_DATA_T, number);
	*end = begin + size;
}

error_t sread_vector(const char *begin, vector_t *vector, const char **end) {
	assert(begin && vector);
	*end = begin;
	if (**end != VECTOR_OPEN_BRACKET)
		return NOT_A_VECTOR;
	++*end;
	size_t i = 0;
	while (true) {
		data_t number;
		const char *next;
		if (sread_number(*end, &number, &next) != SUCCESS) {
			if (vector->dimension)
				delete_vector(vector);
			return INVALID_FORMAT;
		}
		*end = next;
		if (i == vector->dimension && expand_vector(vector) != SUCCESS) {
			return ALLOC_FAILURE;
		}
		vector->components[i++] = number;
		if (**end == VECTOR_CLOSE_BRACKET) {
			++*end;
			break;
		}
		if (**end != VECTOR_SEPARATOR) {
			delete_vector(vector);
			return INVALID_FORMAT;
		}
		++*end;
	}
	if (i < 2) {
		delete_vector(vector);
		return INVALID_FORMAT;
	}
	return fit_vector(vector, i);
}

void write_vector(const vector_t *vector) {
	putchar(VECTOR_OPEN_BRACKET);
	for (size_t i = 0; i + 1 < vector->dimension; ++i) {
		write_number(vector->components[i]);
		putchar(VECTOR_SEPARATOR);
	}
	write_number(vector->components[vector->dimension - 1]);
	putchar(VECTOR_CLOSE_BRACKET);
}

void swrite_vector(char *begin, const vector_t *vector, char **end) {
	*end = begin;
	*(*end)++ = VECTOR_OPEN_BRACKET;
	for (size_t i = 0; i + 1 < vector->dimension; ++i) {
		char *next = *end;
		swrite_number(*end, vector->components[i], &next);
		*end = next;
		*(*end)++ = VECTOR_SEPARATOR;
	}
	char *next = *end;
	swrite_number(*end, vector->components[vector->dimension - 1], &next);
	*end = next;
	*(*end)++ = VECTOR_CLOSE_BRACKET;
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
	*max_dimension = b->dimension;
	*min_dimension = a->dimension;
	*sign = -1;
	return b->components;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"    // for this

typedef data_t (*f_t)(data_t, data_t);                 //   |
data_t base_add(data_t a, data_t b) { return a + b; }  //   |
data_t base_sub(data_t a, data_t b) { return a - b; }  //   |
data_t tail_add(data_t sign, data_t d) { return d; }   // <——
data_t tail_sub(data_t sign, data_t d) { return sign * d; }

#pragma GCC diagnostic pop

error_t  _add_vectors(const vector_t *a, const vector_t *b, vector_t *c, f_t base, f_t tail) {
	assert(a && b && c && base && tail);
	size_t max_dimension, min_dimension;
	const data_t *components_of_max;
	data_t sign;
	components_of_max = max_min_dim(a, b, &max_dimension, &min_dimension, &sign);
	if (create_vector(c, max_dimension) != SUCCESS) {
		return ALLOC_FAILURE;
	}
	for (size_t i = 0; i != min_dimension; ++i)
		c->components[i] = base(a->components[i], b->components[i]);
	for (size_t i = min_dimension; i != max_dimension; ++i)
		c->components[i] = tail(sign, components_of_max[i]);
	return SUCCESS;
}

error_t add_vectors(const vector_t *a, const vector_t *b, vector_t *c) {
	assert(a && b && c);
	return _add_vectors(a, b, c, base_add, tail_add);
}

error_t subtract_vectors(const vector_t *a, const vector_t *b, vector_t *c) {
	assert(a && b && c);
	return _add_vectors(a, b, c, base_sub, tail_sub);
}

error_t multiply_vector(data_t number, const vector_t *vector, vector_t *result) {
	assert(vector && result);
	if (create_vector(result, vector->dimension) != SUCCESS)
		return ALLOC_FAILURE;
	for (size_t i = 0; i != result->dimension; ++i)
		result->components[i] = vector->components[i] * number;
	return SUCCESS;
}

typedef struct stack_node_t {
	void *data;
	struct stack_node_t *next;
} stack_node_t;

void *pop(stack_node_t **stack_node) {
	assert(stack_node);
	stack_node_t *prev = *stack_node;
	void *data = prev->data;
	*stack_node = (*stack_node)->next;
	free(prev);
	return data;
}

const void *top(stack_node_t *stack_node) {
	assert(stack_node);
	return stack_node->data;
}

void delete_stack(stack_node_t **stack_node) {
	assert(stack_node);
	while (*stack_node)
		free(pop(stack_node));
}

error_t push(stack_node_t **stack_node, const void *data, size_t size) {
	assert(stack_node && data && size);
	stack_node_t *new_stack_node = malloc(sizeof *new_stack_node);
	if (!new_stack_node) {
		delete_stack(stack_node);
		return ALLOC_FAILURE;
	}
	if (!(new_stack_node->data = malloc(size))) {
		free(new_stack_node);
		delete_stack(stack_node);
		return ALLOC_FAILURE;
	}
	memcpy(new_stack_node->data, data, size);
	new_stack_node->next = *stack_node;
	*stack_node = new_stack_node;
	return SUCCESS;
}

typedef struct {
	enum {
		NUMBER,
		VECTOR
	} type;
	union {
		data_t number;
		vector_t vector;
	};
} operand_t;

error_t sread_operand(const char *begin, operand_t *operand, const char **end) {
	assert(begin && operand && end);
	create_vector(&operand->vector, 0);
	if (sread_number(begin, &operand->number, end) == SUCCESS) {
		operand->type = NUMBER;
		return SUCCESS;
	}
	error_t error = sread_vector(begin, &operand->vector, end);
	if (error == SUCCESS) {
		operand->type = VECTOR;
		return SUCCESS;
	}
	if (error == NOT_A_VECTOR)
		return NOT_AN_OPERAND;
	return error; // ALLOC_FAILURE or INVALID_FORMAT
}

void swrite_operand(char *begin, const operand_t *operand, char **end) {
	assert(begin && operand && end);
	switch (operand->type) {
	case NUMBER:
		swrite_number(begin, operand->number, end);
		return;
	case VECTOR:
		swrite_vector(begin, &operand->vector, end);
		return;
	default:
		assert(0);
	}
}

void delete_operand(operand_t *operand) {
	if (operand->type == VECTOR)
		delete_vector(&operand->vector);
}

typedef enum {
	OPEN_BRACKET,
	CLOSE_BRACKET,
	PLUS,
	MINUS,
	MULTIPLY,
} operator_t;

#define OPEN_BRACKET_SYMB  '('
#define CLOSE_BRACKET_SYMB ')'
#define PLUS_SYMB          '+'
#define MINUS_SYMB         '-'
#define MULTIPLY_SYMB      '*'

char symb(operator_t operator) {
	switch (operator) {
	case OPEN_BRACKET:
		return OPEN_BRACKET_SYMB;
	case CLOSE_BRACKET:
		return CLOSE_BRACKET_SYMB;
	case PLUS:
		return PLUS_SYMB;
	case MINUS:
		return MINUS_SYMB;
	case MULTIPLY:
		return MULTIPLY_SYMB;
	default:
		return '\0';
	}
}

error_t sread_operator(const char *begin, operator_t *operator, const char **end) {
	assert(begin && operator && end);
	*end = begin;
	switch (**end) {
	case OPEN_BRACKET_SYMB:
		*operator = OPEN_BRACKET;
		break;
	case CLOSE_BRACKET_SYMB:
		*operator = CLOSE_BRACKET;
		break;
	case PLUS_SYMB:
		*operator = PLUS;
		break;
	case MINUS_SYMB:
		*operator = MINUS;
		break;
	case MULTIPLY_SYMB:
		*operator = MULTIPLY;
		break;
	default:
		return NOT_AN_OPERATOR;
	}
	++*end;
	return SUCCESS;
}

void swrite_operator(char *begin, operator_t operator, char **end) {
	*end = begin;
	*(*end)++ = symb(operator);
}

operand_t pop_operand(stack_node_t **operands) {
	assert(operands);
	operand_t *p_operand = pop(operands);
	operand_t operand = *p_operand;
	free(p_operand);
	return operand;
}

operator_t pop_operator(stack_node_t **operators) {
	assert(operators);
	operator_t *p_operator = pop(operators);
	operator_t operator = *p_operator;
	free(p_operator);
	return operator;
}

error_t sread_operand_from(const char **run, operand_t *operand) {
	assert(run && operand);
	const char *next;
	error_t error = sread_operand(*run, operand, &next);
	*run = next;
	return error;
}

error_t sread_operator_from(const char **run, operator_t *operator) {
	assert(run && operator);
	const char *next;
	error_t error = sread_operator(*run, operator, &next);
	*run = next;
	return error;
}

#define EXPR_SEPARATOR ' '

void swrite_operand_to(char **run, const operand_t *operand) {
	assert(run && operand);
	char *next;
	swrite_operand(*run, operand, &next);
	*run = next;
	*(*run)++ = EXPR_SEPARATOR;
}

void swrite_operator_to(char **run, operator_t operator) {
	assert(run);
	char *next;
	swrite_operator(*run, operator, &next);
	*run = next;
	*(*run)++ = EXPR_SEPARATOR;
}

#define WRAPPER(x) do { x } while (0)
#define GOTO_ERR(err, lbl) WRAPPER( error = err; goto lbl; )
#define SAFE_PUSH(ppstack, pdata) WRAPPER( if ((error = push(ppstack, pdata, sizeof *pdata)) != SUCCESS) goto free_result; )

error_t shunting_yard(const char *infix_expr, char **postfix_expr) {
	assert(infix_expr && postfix_expr && strlen(infix_expr));
	*postfix_expr = malloc((2 * strlen(infix_expr) + 1) * sizeof (char));
	if (!*postfix_expr)
		return ALLOC_FAILURE;
	const char *run_infix = infix_expr;
	char *run_postfix = *postfix_expr;
	stack_node_t *operators = NULL;
	error_t error = SUCCESS;
	while (*run_infix) {
		// maybe it's an operand ?
		operand_t operand;
		if ((error = sread_operand_from(&run_infix, &operand)) == SUCCESS) {
			swrite_operand_to(&run_postfix, &operand);
			delete_operand(&operand);
			continue;
		}
		else if (error != NOT_AN_OPERAND) // ALLOC_FAILURE or INVALID_FORMAT
			goto free_stack;
		// okay, it must be an operator
		operator_t operator;
		if ((error = sread_operator_from(&run_infix, &operator)) != SUCCESS)
			GOTO_ERR(INVALID_FORMAT, free_stack);
		if (operator == OPEN_BRACKET)
			SAFE_PUSH(&operators, &operator);
		else if (operator == CLOSE_BRACKET) {
			while (true) {
				if (!operators)
					GOTO_ERR(INVALID_FORMAT, free_result);
				if ((operator = pop_operator(&operators)) == OPEN_BRACKET)
					break;
				swrite_operator_to(&run_postfix, operator);
			}
		}
		else {                   // priority
			if (operators && operator <= *(operator_t *) top(operators))
				swrite_operator_to(&run_postfix, pop_operator(&operators));
			SAFE_PUSH(&operators, &operator);
		}
	}
	while (operators) {
		const operator_t operator = pop_operator(&operators);
		if (operator == OPEN_BRACKET)
			GOTO_ERR(INVALID_FORMAT, free_stack);
		swrite_operator_to(&run_postfix, operator);
	}
	*run_postfix = '\0';
	return SUCCESS;

free_stack:
	delete_stack(&operators);
free_result:
	free(*postfix_expr);
	return error;
}

// skip whitespace
error_t sw_sread_operand_from(const char **run, operand_t *operand) {
	assert(run && operand);
	const error_t error = sread_operand_from(run, operand);
	if (error == SUCCESS)
		++*run;
	return error;
}

error_t sw_sread_operator_from(const char **run, operator_t *operator) {
	assert(run && operator);
	const error_t error = sread_operator_from(run, operator);
	if (error == SUCCESS)
		++*run;
	return error;
}

error_t exec(const operand_t *a, const operand_t *b, operand_t *c, operator_t operator) {
	assert(a && b && c);
	error_t error = SUCCESS;
	switch (operator) {
	case PLUS:
		if (a->type == VECTOR && b->type == VECTOR)
			error = add_vectors(&a->vector, &b->vector, &c->vector);
		else
			error = INVALID_FORMAT;
		break;
	case MINUS:
		if (a->type == VECTOR && b->type == VECTOR)
			error = subtract_vectors(&a->vector, &b->vector, &c->vector);
		else
			error = INVALID_FORMAT;
		break;
	case MULTIPLY:
		if (a->type == VECTOR)
			if (b->type == NUMBER)
				error = multiply_vector(b->number, &a->vector, &c->vector);
			else
				error = INVALID_FORMAT;
		else if (b->type == VECTOR)
			error = multiply_vector(a->number, &b->vector, &c->vector);
		else
			error = INVALID_FORMAT;
		break;
	default:
		assert(0);
	}
	return error;
}

error_t calculate(const char *expr, vector_t *vector) {
	assert(expr);
	const char *run = expr;
	stack_node_t *operands = NULL;
	error_t error = SUCCESS;
	operand_t result;
	result.type = VECTOR;
	create_vector(&result.vector, 0);
	while (*run) {
		operand_t operand;
		if ((error = sw_sread_operand_from(&run, &operand)) == SUCCESS) {
			SAFE_PUSH(&operands, &operand);
			continue;
		}
		else if (error != NOT_AN_OPERAND)  // ALLOC_FAILURE
			goto free_stack;
		// it must be an operator
		operator_t operator;
		sw_sread_operator_from(&run, &operator);
		if (!operands)
			GOTO_ERR(INVALID_FORMAT, free_result);
		operand_t b = pop_operand(&operands);
		if (!operands)
			GOTO_ERR(INVALID_FORMAT, free_result);
		operand_t a = pop_operand(&operands);
		error = exec(&a, &b, &result, operator);
		delete_operand(&a);
		delete_operand(&b);
		if (error != SUCCESS) {
			result.type = NUMBER;
			goto free_stack;
		}
		SAFE_PUSH(&operands, &result);
	}
	if (!operands)
		GOTO_ERR(INVALID_FORMAT, free_result);
	result = pop_operand(&operands);
	if (operands)
		GOTO_ERR(INVALID_FORMAT, free_stack);
	*vector = result.vector;
	return SUCCESS;

free_stack:
	delete_stack(&operands);
free_result:
	delete_operand(&result);
	return error;
}

char *read_line() {
	char *line = NULL;
	size_t n = 0;
	if (getline(&line, &n, stdin) == -1) {
		free(line);
		return NULL;
	}
	char *peol = strchr(line, '\n');
	if (peol)
		*peol = '\0';
	return line;
}

char *read_lines() {
	size_t buf_size = STD_BUF_SIZE;
	size_t size = 0;
	char *line, *lines = malloc(buf_size);
	if (!lines)
		return NULL;
	*lines = '\0';
	while ((line = read_line())) {
		size_t add = strlen(line);
		if (size + add + 1 > buf_size) {
			while (size + add + 1 > (buf_size *= STD_BUF_SIZE_MULT))
				;
			char *tmp = realloc(lines, buf_size);
			if (!tmp) {
				free(line);
				free(lines);
				return NULL;
			}
			lines = tmp;
		}
		strncat(lines + size, line, add);
		size += add;
		free(line);
	}
	if (!size) {
		free(lines);
		return NULL;
	}
	return lines;
}

void collapse(char *line) {
	char *run = line;
	while (*run) {
		while (*run == ' ')
			++run;
		*line++ = *run++;
	}
	*line = '\0';
}

static inline __attribute__((always_inline)) void delete_line(char *line) {
	free(line);
}

int main(void) {
	char *infix_expr = read_lines();
	if (!infix_expr) {
		puts("[error]");
		return 0;
	}
	collapse(infix_expr);
	char *postfix_expr;
	error_t error = shunting_yard(infix_expr, &postfix_expr);
	delete_line(infix_expr);
	if (error != SUCCESS) {
		puts("[error]");
		return 0;
	}
	vector_t vector;
	error = calculate(postfix_expr, &vector);
	delete_line(postfix_expr);
	if (error != SUCCESS) {
		puts("[error]");
		return 0;
	}
	write_vector(&vector);
	delete_vector(&vector);
}
