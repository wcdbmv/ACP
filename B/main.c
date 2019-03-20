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

static inline void _skip(const char **run, size_t skip) {
	assert(run && *run);
	*run += skip;
}

static inline error_t _shutdown_with_free(error_t error, void *pdata) {
	free(pdata);
	return error;
}

// ──── char ──────────────────────────────────────────────────────────────────────────────────────

error_t sread_char(const char **run, char c, size_t skip) {
	assert(run && *run);
	if (**run == c) {
		++*run;
		_skip(run, skip);
		return SUCCESS;
	}
	return INVALID_FORMAT;
}

static inline void write_char(char c) { putchar(c); }
static inline void swrite_char(char **run, char c) {
	assert(run && *run);
	*(*run)++ = c;
}

// ──── number ────────────────────────────────────────────────────────────────────────────────────

typedef int data_t;
#define FORMAT_DATA_T "%d"

error_t sread_number(const char **run, data_t *number, size_t skip) {
	assert(run && *run && number);
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
	assert(run && *run);
	size_t size = sprintf(*run, FORMAT_DATA_T, number);
	*run += size;
}

void swrite_number_with(char **run, data_t number, char c) {
	assert(run && *run);
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
	assert(vector);
	const size_t dimension = maxlu(vector->dimension * STD_BUF_SIZE_MULT, STD_BUF_SIZE);
	return resize_vector(vector, dimension);
}

static inline error_t fit_vector(vector_t *vector, size_t dimension) {
	assert(vector && vector->components);
	assert(dimension && dimension <= vector->dimension);
	return resize_vector(vector, dimension);
}

static inline error_t _shutdown_with_delete_vector(error_t error, vector_t *vector) {
	assert(vector);
	delete_vector(vector);
	return error;
}

error_t sread_vector(const char **run, vector_t *vector, size_t skip) {
	assert(run && *run && vector);
	if (sread_char(run, VECTOR_OPEN_BRACKET, 0) != SUCCESS)
		return NOT_A_VECTOR;
	size_t i = 0;
	while (**run) {
		data_t number;
		if (sread_number(run, &number, 0) != SUCCESS)
			return _shutdown_with_delete_vector(INVALID_FORMAT, vector);
		if (i == vector->dimension && expand_vector(vector) != SUCCESS)
			return ALLOC_FAILURE;
		vector->components[i++] = number;
		if (sread_char(run, VECTOR_CLOSE_BRACKET, 0) == SUCCESS)
			break;
		if (sread_char(run, VECTOR_SEPARATOR, 0) != SUCCESS)
			return _shutdown_with_delete_vector(INVALID_FORMAT, vector);
	}
	if (i < MIN_VECTOR_DIMENSION)
		return _shutdown_with_delete_vector(INVALID_FORMAT, vector);
	_skip(run, skip);
	return fit_vector(vector, i);
}

void write_vector(const vector_t *vector) {
	assert(vector);
	write_char(VECTOR_OPEN_BRACKET);
	for (size_t i = 0; i + 1 < vector->dimension; ++i)
		write_number_with(vector->components[i], VECTOR_SEPARATOR);
	write_number_with(vector->components[vector->dimension - 1], VECTOR_CLOSE_BRACKET);
}

void swrite_vector(char **run, const vector_t *vector) {
	assert(run && *run && vector);
	swrite_char(run, VECTOR_OPEN_BRACKET);
	for (size_t i = 0; i + 1 < vector->dimension; ++i)
		swrite_number_with(run, vector->components[i], VECTOR_SEPARATOR);
	swrite_number_with(run, vector->components[vector->dimension - 1], VECTOR_CLOSE_BRACKET);
}

void swrite_vector_with(char **run, const vector_t *vector, char c) {
	assert(run && *run && vector);
	swrite_vector(run, vector);
	swrite_char(run, c);
}

/* Вспомогательная функция для `_add_vectors`. Определяет максимальную и минимальную размерность */
/* векторов, знак для операции вычитания для большей размерности (если размерность вычитаемого   */
/* больше, то его "хвост" помешается в результат с плюсом, если размерность вычитателя больше,   */
/* то с минусом).                                                                                */
static inline const data_t *max_min_dim(
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
	assert(a && b && c);
	return _add_vectors(a, b, c, base_add, tail_add);
}

static inline error_t subtract_vectors(const vector_t *a, const vector_t *b, vector_t *c) {
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

static inline error_t _shutdown_with_delete_stack(error_t error, stack_node_t **stack_node) {
	assert(stack_node);
	delete_stack(stack_node);
	return error;
}

error_t push(stack_node_t **stack_node, const void *data, size_t size) {
	assert(stack_node && data && size);
	stack_node_t *new_stack_node = malloc(sizeof *new_stack_node);
	if (!new_stack_node)
		return ALLOC_FAILURE;
	if (!(new_stack_node->data = malloc(size)))
		return _shutdown_with_free(ALLOC_FAILURE, new_stack_node);
	memcpy(new_stack_node->data, data, size);
	new_stack_node->next = *stack_node;
	*stack_node = new_stack_node;
	return SUCCESS;
}

/* Удаляет стэк в случае ошибки push */
error_t handle_push_error(stack_node_t **ppstack, void *pdata, size_t size) {
	assert(ppstack && pdata);
	const error_t error = push(ppstack, pdata, size);
	if (error != SUCCESS)
		delete_stack(ppstack);
	return error;
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
	assert(run && *run && operand);
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
	assert(run && *run && operand);
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

static inline error_t _shutdown_with_delete_operand(error_t error, operand_t *operand) {
	assert(operand);
	delete_operand(operand);
	return error;
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
	assert(run && *run && operator);
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

static inline void swrite_operator(char **run, operator_t operator) {
	assert(run && *run);
	swrite_char(run, symb(operator));
}

void swrite_operator_with(char **run, operator_t operator, char c) {
	assert(run && *run);
	swrite_operator(run, operator);
	swrite_char(run, c);
}

// ──── algorithm ─────────────────────────────────────────────────────────────────────────────────

#define POSTFIX_EXPR_SEPARATOR WHITESPACE

/* Следующие вспомогательные функции вынимают из стека не указатель на значение, а само значение */
operand_t pop_operand(stack_node_t **operands) {                                              /* */
	assert(operands && *operands);
	operand_t *p_operand = pop(operands);
	operand_t operand = *p_operand;
	free(p_operand);
	return operand;
}

operator_t pop_operator(stack_node_t **operators) {                                           /* */
	assert(operators && *operators);
	operator_t *p_operator = pop(operators);
	operator_t operator = *p_operator;
	free(p_operator);
	return operator;
}

/* Вспомогательная функция для `shunting yard`. Освобождает выделенную внутри функции память. По */
/* сути это _shutdown_with_delete_stack_and_free.                                                */
error_t _shutdown_shunting_yard(error_t error, stack_node_t **stack, char *expr) {
	assert(stack && expr);
	delete_stack(stack);
	free(expr);
	return error;
}

/* Вспомогательная функция для `shunting_yard`. Кладёт, в соответствии с алгоритмом, оператор    */
/* в стэк, в случае ошибки удаляет его (стэк).                                                   */
error_t handle_push_operator_error(stack_node_t **operators, operator_t operator, char **run) {
	assert(operators && run && *run);
	if (operator == OPEN_BRACKET)
		return handle_push_error(operators, &operator, sizeof operator);
	if (operator == CLOSE_BRACKET) {
		while (true) {
			if (!*operators)
				return INVALID_FORMAT;
			if ((operator = pop_operator(operators)) == OPEN_BRACKET)
				break;
			swrite_operator_with(run, operator, POSTFIX_EXPR_SEPARATOR);
		}
		return SUCCESS;
	}
	                       // priority
	if (*operators && operator <= *(operator_t *) top(*operators))
		swrite_operator_with(run, pop_operator(operators), POSTFIX_EXPR_SEPARATOR);
	return handle_push_error(operators, &operator, sizeof operator);
}

error_t shunting_yard(const char *infix_expr, char **postfix_expr) {
	assert(infix_expr && postfix_expr && strlen(infix_expr));
	*postfix_expr = malloc((2 * strlen(infix_expr) + 1) * sizeof (char));
	if (!*postfix_expr)
		return ALLOC_FAILURE;
	const char *run_infix = infix_expr;
	char *run_postfix = *postfix_expr;
	stack_node_t *operators = NULL;
	while (*run_infix) {
		// maybe it's an operand ?
		operand_t operand;
		error_t error = sread_operand(&run_infix, &operand, 0);
		if (error == SUCCESS) {
			swrite_operand_with(&run_postfix, &operand, POSTFIX_EXPR_SEPARATOR);
			delete_operand(&operand);
			continue;
		}
		else if (error != NOT_AN_OPERAND)  // ALLOC_FAILURE or INVALID_FORMAT
			return _shutdown_shunting_yard(error, &operators, *postfix_expr);

		// okay, it must be an operator
		operator_t operator;
		if (sread_operator(&run_infix, &operator, 0) != SUCCESS)
			return _shutdown_shunting_yard(INVALID_FORMAT, &operators, *postfix_expr);
		if ((error = handle_push_operator_error(&operators, operator, &run_postfix)) != SUCCESS)
			return _shutdown_with_free(error, *postfix_expr);
	}
	while (operators) {
		const operator_t operator = pop_operator(&operators);
		if (operator == OPEN_BRACKET)
			return _shutdown_shunting_yard(INVALID_FORMAT, &operators, *postfix_expr);
		swrite_operator_with(&run_postfix, operator, POSTFIX_EXPR_SEPARATOR);
	}
	*run_postfix = EOS;
	return SUCCESS;
}

/* `_add_operands`, по аналогии с `_add_vectors`, складывает или вычитает операнды               */
typedef error_t (*add_vectors_ft)(const vector_t *, const vector_t *, vector_t *);
error_t _add_operands(const operand_t *a, const operand_t *b, operand_t *c, add_vectors_ft _add) {
	assert(a && b && c && _add);
	if (a->type == VECTOR && b->type == VECTOR)
		return _add(&a->vector, &b->vector, &c->vector);
	return INVALID_FORMAT;
}

static inline error_t add_operands(const operand_t *a, const operand_t *b, operand_t *c) {
	assert(a && b && c);
	return _add_operands(a, b, c, &add_vectors);
}

static inline error_t subtract_operands(const operand_t *a, const operand_t *b, operand_t *c) {
	assert(a && b && c);
	return _add_operands(a, b, c, &subtract_vectors);
}

error_t multiply_operands(const operand_t *a, const operand_t *b, operand_t *c) {
	assert(a && b && c);
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

/* Вспомогательная функция для `calculate`. Освобождает выделенную внутри функции память. По     */
/* сути это _shutdown_with_delete_stack_and_operand.                                             */
error_t _shutdown_calculate(error_t error, stack_node_t **stack, operand_t *operand) {
	assert(stack && operand);
	delete_stack(stack);
	delete_operand(operand);
	return error;
}

/* Вспомогательная функция для `calculate`. Кладёт результат операции в стэк, в случае ошибки    */
/* удаляет его (стэк).                                                                           */
error_t handle_push_operand_error(stack_node_t **operands, operand_t *result, operator_t operator) {
	if (!*operands)
		return INVALID_FORMAT;
	operand_t b = pop_operand(operands);
	if (!*operands) {
		delete_operand(&b);
		return INVALID_FORMAT;
	}
	operand_t a = pop_operand(operands);
	const error_t error = execute(&a, &b, result, operator);
	delete_operand(&a);
	delete_operand(&b);
	if (error != SUCCESS) {
		result->type = NUMBER;
		return _shutdown_with_delete_stack(error, operands);
	}
	return handle_push_error(operands, result, sizeof *result);
}

error_t calculate(const char *expr, vector_t *vector) {
	assert(expr && vector);
	const char *run = expr;
	stack_node_t *operands = NULL;
	operand_t result;
	init_operand(&result);
	while (*run) {
		operand_t operand;
		error_t error = sread_operand(&run, &operand, 1);
		if (error == SUCCESS) {
			// safe push
			if (handle_push_error(&operands, &operand, sizeof operand) != SUCCESS)
				return _shutdown_with_delete_operand(ALLOC_FAILURE, &result);
			continue;
		}
		else if (error != NOT_AN_OPERAND)  // ALLOC_FAILURE
			return _shutdown_calculate(error, &operands, &result);

		// it must be an operator
		operator_t operator;
		sread_operator(&run, &operator, 1);
		if ((error = handle_push_operand_error(&operands, &result, operator)) != SUCCESS)
			return _shutdown_with_delete_operand(error, &result);
	}
	if (!operands)
		return _shutdown_with_delete_operand(INVALID_FORMAT, &result);
	result = pop_operand(&operands);
	if (operands)
		return _shutdown_calculate(INVALID_FORMAT, &operands, &result);
	*vector = result.vector;
	return SUCCESS;
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

// TODO: refactor
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
	assert(line);
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

static inline int _shutdown_with_error(void) {
	puts("[error]");
	return 0;
}

int main(void) {
	char *infix_expr = read_lines();
	if (!infix_expr)
		return _shutdown_with_error();
	collapse(infix_expr);
	char *postfix_expr;
	error_t error = shunting_yard(infix_expr, &postfix_expr);
	delete_line(infix_expr);
	if (error != SUCCESS)
		return _shutdown_with_error();
	vector_t vector;
	error = calculate(postfix_expr, &vector);
	delete_line(postfix_expr);
	if (error != SUCCESS)
		return _shutdown_with_error();
	write_vector(&vector);
	delete_vector(&vector);
}
