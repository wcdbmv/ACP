/*
 * Задача B-7. Программа калькулятор для векторов
 *
 * Керимов А.
 * АПО-13
 */

// О функциях ввода и вывода
//
// Функции вида `error_t sread_TYPE(const char **run, TYPE *data, size_t skip);` считывают из
// строки `*run` переменную `data` и, при успешном считывании, перемещают `*run` в конец считанной
// переменной, пропуская затем `skip` символов. Иначе возвращают ошибку NOT_A_TYPE.
//
// Функции вида `void write_TYPE(const TYPE *data);` записывают в stdout переменную `data`.
//
// Функции вида `void swrite_TYPE(char **run, const TYPE *data);` записывают в строку `*run`
// переменную `data` и перемещают `*run` на количество записанных символов.
//
// Функции вида `void swrite_TYPE_with(char **run, const TYPE *data, char c);` помимо этого
// записывают ещё и символ `c`.

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

// ──── error ─────────────────────────────────────────────────────────────────────────────────────

typedef enum {
	SUCCESS = 0,
	ALLOC_FAILURE,
	NOT_A_NUMBER,
	NOT_A_VECTOR,
	NOT_AN_OPERAND,
	NOT_AN_OPERATOR,
	INVALID_FORMAT,
} error_t;

// ──── common ────────────────────────────────────────────────────────────────────────────────────

#define STD_BUF_SIZE 1024
#define STD_BUF_SIZE_MULT 2

static inline size_t maxlu(size_t a, size_t b) { return a > b ? a : b; }

#define EOL '\n'
#define EOS '\0'
#define WHITESPACE ' '

#define WRAPPER(x) do { x } while (0)
#define GOTO_ERR(err, lbl) WRAPPER( error = err; goto lbl; )

static inline void _skip(const char **run, size_t skip) { *run += skip; }

// ──── char ──────────────────────────────────────────────────────────────────────────────────────

error_t sread_char(const char **run, char c, size_t skip) {
	assert(run);
	if (**run == c) {
		++*run;
		_skip(run, skip);
		return SUCCESS;
	}
	return INVALID_FORMAT;
}

static inline void write_char(char c) { putchar(c); }
static inline void swrite_char(char **run, char c) { *(*run)++ = c; }

// ──── number ────────────────────────────────────────────────────────────────────────────────────

typedef int data_t;
#define FORMAT_DATA_T "%d"

error_t sread_number(const char **run, data_t *number, size_t skip) {
	assert(run && number);
	if (**run == '+' || **run == '-')
		return NOT_A_NUMBER;
	int n_chars = 0;
	if (sscanf(*run, FORMAT_DATA_T "%n", number, &n_chars) < 1)
		return NOT_A_NUMBER;
	*run += n_chars + skip;
	return SUCCESS;
}

static inline void write_number(data_t number) { printf(FORMAT_DATA_T, number); }
static inline void write_number_with(data_t number, char c) { printf(FORMAT_DATA_T "%c", number, c); }

void swrite_number(char **run, data_t number) {
	assert(run);
	size_t size = sprintf(*run, FORMAT_DATA_T, number);
	*run += size;
}

void swrite_number_with(char **run, data_t number, char c) {
	assert(run);
	swrite_number(run, number);
	swrite_char(run, c);
}

// ──── vector ────────────────────────────────────────────────────────────────────────────────────

#define MIN_VECTOR_DIMENSION  2
#define VECTOR_OPEN_BRACKET  '{'
#define VECTOR_CLOSE_BRACKET '}'
#define VECTOR_SEPARATOR     ','

typedef struct {
	size_t dimension;
	data_t *components;
} vector_t;

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
	if (vector->dimension)
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

static inline error_t expand_vector(vector_t *vector) {
	const size_t dimension = maxlu(vector->dimension * STD_BUF_SIZE_MULT, STD_BUF_SIZE);
	return resize_vector(vector, dimension);
}

static inline error_t fit_vector(vector_t *vector, size_t dimension) {
	assert(vector && vector->components);
	assert(dimension && dimension <= vector->dimension);
	return resize_vector(vector, dimension);
}

error_t sread_vector(const char **run, vector_t *vector, size_t skip) {
	assert(run && vector);
	if (sread_char(run, VECTOR_OPEN_BRACKET, 0) != SUCCESS)
		return NOT_A_VECTOR;
	size_t i = 0;
	error_t error = SUCCESS;
	while (**run) {
		data_t number;
		if ((error = sread_number(run, &number, 0)) != SUCCESS)
			GOTO_ERR(INVALID_FORMAT, free_result);
		if (i == vector->dimension && expand_vector(vector) != SUCCESS)
			return ALLOC_FAILURE;
		vector->components[i++] = number;
		if (sread_char(run, VECTOR_CLOSE_BRACKET, 0) == SUCCESS)
			break;
		if (sread_char(run, VECTOR_SEPARATOR, 0) != SUCCESS)
			GOTO_ERR(INVALID_FORMAT, free_result);
	}
	if (i < MIN_VECTOR_DIMENSION)
		GOTO_ERR(INVALID_FORMAT, free_result);
	_skip(run, skip);
	return fit_vector(vector, i);

free_result:
	delete_vector(vector);
	return error;
}

void write_vector(const vector_t *vector) {
	assert(vector);
	write_char(VECTOR_OPEN_BRACKET);
	for (size_t i = 0; i + 1 < vector->dimension; ++i)
		write_number_with(vector->components[i], VECTOR_SEPARATOR);
	write_number_with(vector->components[vector->dimension - 1], VECTOR_CLOSE_BRACKET);
}

void swrite_vector(char **run, const vector_t *vector) {
	assert(run && vector);
	swrite_char(run, VECTOR_OPEN_BRACKET);
	for (size_t i = 0; i + 1 < vector->dimension; ++i)
		swrite_number_with(run, vector->components[i], VECTOR_SEPARATOR);
	swrite_number_with(run, vector->components[vector->dimension - 1], VECTOR_CLOSE_BRACKET);
}

void swrite_vector_with(char **run, const vector_t *vector, char c) {
	assert(run && vector);
	swrite_vector(run, vector);
	swrite_char(run, c);
}

/* Вспомогательная функция для `_add_vectors`. Определяет максимальную и минимальную размерность */
/* векторов, знак для операции вычитания для большей размерностей (если размерность вычитаемого  */
/* больше, то его "хвост" помешается в результат с плюсом, если размерность вычитателя больше,   */
/* то с минусом).                                                                                */
const data_t *max_min_dim(
		const vector_t *a, const vector_t *b,
		size_t *max_dimension, size_t *min_dimension,
		data_t *sign) {
	assert(a && b && max_dimension && min_dimension && sign);
	if (a->dimension >= b->dimension) {
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

typedef data_t (*add_ft)(data_t, data_t);              //   │
data_t base_add(data_t a, data_t b) { return a + b; }  //   │
data_t base_sub(data_t a, data_t b) { return a - b; }  //   │
data_t tail_add(data_t sign, data_t d) { return d; }   // ◄─┘
data_t tail_sub(data_t sign, data_t d) { return sign * d; }

#pragma GCC diagnostic pop

/* Складывает или вычитает вектора */
error_t _add_vectors(const vector_t *a, const vector_t *b, vector_t *c, add_ft base, add_ft tail) {
	assert(a && b && c && base && tail);
	size_t max_dimension, min_dimension;
	const data_t *components_of_max;
	data_t sign;
	components_of_max = max_min_dim(a, b, &max_dimension, &min_dimension, &sign);
	if (create_vector(c, max_dimension) != SUCCESS)
		return ALLOC_FAILURE;
	for (size_t i = 0; i != min_dimension; ++i)
		c->components[i] = base(a->components[i], b->components[i]);
	for (size_t i = min_dimension; i != max_dimension; ++i)
		c->components[i] = tail(sign, components_of_max[i]);
	return SUCCESS;
}

static inline error_t add_vectors(const vector_t *a, const vector_t *b, vector_t *c) {
	return _add_vectors(a, b, c, base_add, tail_add);
}

static inline error_t subtract_vectors(const vector_t *a, const vector_t *b, vector_t *c) {
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

// ──── stack ─────────────────────────────────────────────────────────────────────────────────────

typedef struct stack_node_t {
	void *data;
	struct stack_node_t *next;
} stack_node_t;

const void *top(stack_node_t *stack_node) {
	assert(stack_node);
	return stack_node->data;
}

void *pop(stack_node_t **stack_node) {
	assert(stack_node);
	stack_node_t *prev = *stack_node;
	void *data = prev->data;
	*stack_node = (*stack_node)->next;
	free(prev);
	return data;
}

void delete_stack(stack_node_t **stack_node) {
	assert(stack_node);
	while (*stack_node)
		free(pop(stack_node));
}

error_t push(stack_node_t **stack_node, const void *data, size_t size) {
	assert(stack_node && data && size);
	stack_node_t *new_stack_node = malloc(sizeof *new_stack_node);
	if (!new_stack_node)
		goto free_stack;
	if (!(new_stack_node->data = malloc(size)))
		goto free_new_stack;
	memcpy(new_stack_node->data, data, size);
	new_stack_node->next = *stack_node;
	*stack_node = new_stack_node;
	return SUCCESS;
free_new_stack:
	free(new_stack_node);
free_stack:
	delete_stack(stack_node);
	return ALLOC_FAILURE;
}

// ──── operand ───────────────────────────────────────────────────────────────────────────────────

typedef struct {
	enum {
		NUMBER = 0,
		VECTOR
	} type;
	union {
		data_t number;
		vector_t vector;
	};
} operand_t;

/* Инициализирует операнд пустым вектором */
void init_operand(operand_t *operand) {
	assert(operand);
	operand->type = VECTOR;
	create_vector(&operand->vector, 0);
}

error_t sread_operand(const char **run, operand_t *operand, size_t skip) {
	assert(run && operand);
	init_operand(operand);
	if (sread_number(run, &operand->number, skip) == SUCCESS) {
		operand->type = NUMBER;
		return SUCCESS;
	}
	error_t error = sread_vector(run, &operand->vector, skip);
	if (error == NOT_A_VECTOR)
		return NOT_AN_OPERAND;
	return error;  // SUCCESS, ALLOC_FAILURE or INVALID_FORMAT
}

void swrite_operand_with(char **run, const operand_t *operand, char c) {
	assert(run && operand);
	switch (operand->type) {
	case NUMBER:
		swrite_number_with(run, operand->number, c);
		return;
	case VECTOR:
		swrite_vector_with(run, &operand->vector, c);
		return;
	default:
		assert(0);
	}
}

void delete_operand(operand_t *operand) {
	assert(operand);
	if (operand->type == VECTOR)
		delete_vector(&operand->vector);
}

// ──── operator ──────────────────────────────────────────────────────────────────────────────────

#define OPEN_BRACKET_SYMB  '('
#define CLOSE_BRACKET_SYMB ')'
#define PLUS_SYMB          '+'
#define MINUS_SYMB         '-'
#define MULTIPLY_SYMB      '*'

typedef enum {
	OPEN_BRACKET,
	CLOSE_BRACKET,
	PLUS,
	MINUS,
	MULTIPLY,
} operator_t;

error_t sread_operator(const char **run, operator_t *operator, size_t skip) {
	assert(run && operator);
	if (sread_char(run, OPEN_BRACKET_SYMB, skip) == SUCCESS)
		*operator = OPEN_BRACKET;
	else if (sread_char(run, CLOSE_BRACKET_SYMB, skip) == SUCCESS)
		*operator = CLOSE_BRACKET;
	else if (sread_char(run, PLUS_SYMB, skip) == SUCCESS)
		*operator = PLUS;
	else if (sread_char(run, MINUS_SYMB, skip) == SUCCESS)
		*operator = MINUS;
	else if (sread_char(run, MULTIPLY_SYMB, skip) == SUCCESS)
		*operator = MULTIPLY;
	else
		return NOT_AN_OPERATOR;
	return SUCCESS;
}

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
		return EOS;
	}
}

static inline void swrite_operator(char **run, operator_t operator) { swrite_char(run, symb(operator)); }

void swrite_operator_with(char **run, operator_t operator, char c) {
	swrite_operator(run, operator);
	swrite_char(run, c);
}

// ──── algorithm ─────────────────────────────────────────────────────────────────────────────────

#define POSTFIX_EXPR_SEPARATOR WHITESPACE

#define SAFE_PUSH(ppstack, pdata) WRAPPER( \
	if ((error = push(ppstack, pdata, sizeof *pdata)) != SUCCESS) \
		goto free_result; )


/* Следующие вспомогательные функции вынимают из стека не указатель на значение, а само значение */
#define _RETURN_POP_DATA(ppstack, type) WRAPPER( \
	type *pdata = pop(ppstack); \
	type data = *pdata; \
	free(pdata); \
	return data; )

operand_t pop_operand(stack_node_t **operands) {                                              /* */
	assert(operands);
	_RETURN_POP_DATA(operands, operand_t);
}

operator_t pop_operator(stack_node_t **operators) {                                           /* */
	assert(operators);
	_RETURN_POP_DATA(operators, operator_t);
}

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
		if ((error = sread_operand(&run_infix, &operand, 0)) == SUCCESS) {
			swrite_operand_with(&run_postfix, &operand, POSTFIX_EXPR_SEPARATOR);
			delete_operand(&operand);
			continue;
		}
		else if (error != NOT_AN_OPERAND)  // ALLOC_FAILURE or INVALID_FORMAT
			goto free_stack;
		// okay, it must be an operator
		operator_t operator;
		if ((error = sread_operator(&run_infix, &operator, 0)) != SUCCESS)
			GOTO_ERR(INVALID_FORMAT, free_stack);
		if (operator == OPEN_BRACKET)
			SAFE_PUSH(&operators, &operator);
		else if (operator == CLOSE_BRACKET)
			while (true) {
				if (!operators)
					GOTO_ERR(INVALID_FORMAT, free_result);
				if ((operator = pop_operator(&operators)) == OPEN_BRACKET)
					break;
				swrite_operator_with(&run_postfix, operator, POSTFIX_EXPR_SEPARATOR);
			}
		else {                   // priority
			if (operators && operator <= *(operator_t *) top(operators))
				swrite_operator_with(&run_postfix, pop_operator(&operators), POSTFIX_EXPR_SEPARATOR);
			SAFE_PUSH(&operators, &operator);
		}
	}
	while (operators) {
		const operator_t operator = pop_operator(&operators);
		if (operator == OPEN_BRACKET)
			GOTO_ERR(INVALID_FORMAT, free_stack);
		swrite_operator_with(&run_postfix, operator, POSTFIX_EXPR_SEPARATOR);
	}
	*run_postfix = EOS;
	return SUCCESS;

free_stack:
	delete_stack(&operators);
free_result:
	free(*postfix_expr);
	return error;
}

/* `_add_operands`, по аналогии с `_add_vectors`, складывает или вычитает операнды               */

typedef error_t (*add_vectors_ft)(const vector_t *, const vector_t *, vector_t *);
error_t _add_operands(const operand_t *a, const operand_t *b, operand_t *c, add_vectors_ft _add) {
	if (a->type == VECTOR && b->type == VECTOR)
		return _add(&a->vector, &b->vector, &c->vector);
	return INVALID_FORMAT;
}

static inline error_t add_operands(const operand_t *a, const operand_t *b, operand_t *c) {
	return _add_operands(a, b, c, &add_vectors);
}

static inline error_t subtract_operands(const operand_t *a, const operand_t *b, operand_t *c) {
	return _add_operands(a, b, c, &subtract_vectors);
}

error_t multiply_operands(const operand_t *a, const operand_t *b, operand_t *c) {
	if (a->type == NUMBER) {
		if (b->type == VECTOR)
				return multiply_vector(a->number, &b->vector, &c->vector);
		return INVALID_FORMAT;
	}
	if (b->type == NUMBER)
		return multiply_vector(b->number, &a->vector, &c->vector);
	return INVALID_FORMAT;
}

error_t execute(const operand_t *a, const operand_t *b, operand_t *c, operator_t operator) {
	assert(a && b && c);
	switch (operator) {
	case PLUS:
		return add_operands(a, b, c);
	case MINUS:
		return subtract_operands(a, b, c);
	case MULTIPLY:
		return multiply_operands(a, b, c);
	default:
		assert(0);
	}
}

error_t calculate(const char *expr, vector_t *vector) {
	assert(expr && vector);
	error_t error = SUCCESS;
	const char *run = expr;
	stack_node_t *operands = NULL;
	operand_t result;
	init_operand(&result);
	while (*run) {
		operand_t operand;
		if ((error = sread_operand(&run, &operand, 1)) == SUCCESS) {
			SAFE_PUSH(&operands, &operand);
			continue;
		}
		else if (error != NOT_AN_OPERAND)  // ALLOC_FAILURE
			goto free_stack;
		// it must be an operator
		operator_t operator;
		sread_operator(&run, &operator, 1);
		if (!operands)
			GOTO_ERR(INVALID_FORMAT, free_result);
		operand_t b = pop_operand(&operands);
		if (!operands)
			GOTO_ERR(INVALID_FORMAT, free_result);
		operand_t a = pop_operand(&operands);
		error = execute(&a, &b, &result, operator);
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

// ──── line ──────────────────────────────────────────────────────────────────────────────────────

#define GETLINE_ERROR -1

char *read_line() {
	char *line = NULL;
	size_t n = 0;
	if (getline(&line, &n, stdin) == GETLINE_ERROR) {
		free(line);
		return NULL;
	}
	char *peol = strchr(line, EOL);
	if (peol)
		*peol = EOS;
	return line;
}

char *read_lines() {
	size_t buf_size = STD_BUF_SIZE;
	size_t size = 0;
	char *line, *lines = malloc(buf_size);
	if (!lines)
		return NULL;
	*lines = EOS;
	while ((line = read_line())) {
		const size_t add = strlen(line);
		const size_t new_size = size + add;
		if (new_size + 1 > buf_size) {
			while (new_size + 1 > (buf_size *= STD_BUF_SIZE_MULT))
				;
			char *tmp = realloc(lines, buf_size);
			if (!tmp)
				goto free_line;
			lines = tmp;
		}
		strncat(lines + size, line, add);
		size = new_size;
		free(line);
	}
	if (!size)
		goto free_lines;
	return lines;

free_line:
	free(line);
free_lines:
	free(lines);
	return NULL;
}

void collapse(char *line) {
	char *run = line;
	while (*run) {
		while (*run == WHITESPACE)
			++run;
		*line++ = *run++;
	}
	*line = EOS;
}

static inline void delete_line(char *line) { free(line); }

// ──── main ──────────────────────────────────────────────────────────────────────────────────────

#define SHUTDOWN_WITH_ERROR WRAPPER( puts("[error]"); return 0; )

int main(void) {
	char *infix_expr = read_lines();
	if (!infix_expr)
		SHUTDOWN_WITH_ERROR;
	collapse(infix_expr);
	char *postfix_expr;
	error_t error = shunting_yard(infix_expr, &postfix_expr);
	delete_line(infix_expr);
	if (error != SUCCESS)
		SHUTDOWN_WITH_ERROR;
	vector_t vector;
	error = calculate(postfix_expr, &vector);
	delete_line(postfix_expr);
	if (error != SUCCESS)
		SHUTDOWN_WITH_ERROR;
	write_vector(&vector);
	delete_vector(&vector);
}
