#include "../include/hw6.h"
#define BASE_10 10

bst_sf* insert_bst_sf(matrix_sf *mat, bst_sf *root) {
    if (root == NULL) {
        // Create a new node if the root is NULL
        root = (bst_sf*) malloc(sizeof(bst_sf));
        root->mat = mat;
        root->left_child = root->right_child = NULL;
    } 
    else if (mat->name < root->mat->name)
        // Recursively insert into the left subtree if the matrix name is smaller
        root->left_child = insert_bst_sf(mat, root->left_child);
    else
        // Recursively insert into the right subtree if the matrix name is larger
        root->right_child = insert_bst_sf(mat, root->right_child);
    return root;
}

// Find a matrix in a binary search tree by its name
matrix_sf* find_bst_sf(char name, bst_sf *root) {
    return root == NULL ? NULL : root->mat->name == name ? root->mat : root->mat->name < name ? 
           find_bst_sf(name, root->right_child) : find_bst_sf(name, root->left_child);
}

// Free the memory allocated for a binary search tree
void free_bst_sf(bst_sf *root) {
    if(root == NULL) return;
    // Recursively free the left and right subtrees
    free_bst_sf(root->left_child);
    free_bst_sf(root->right_child);
    free(root);
}

// Add two matrices element-wise
matrix_sf* add_mats_sf(const matrix_sf *mat1, const matrix_sf *mat2) {
    const size_t VALUES_LENGTH = (size_t) (mat1->num_rows * mat1->num_cols);
    int values[VALUES_LENGTH];
    size_t i;
    for (i = 0; i < VALUES_LENGTH; i++)
        values[i] = mat1->values[i] + mat2->values[i];
    return copy_matrix(mat1->num_rows, mat1->num_cols, values);
}

// Multiply two matrices
matrix_sf* mult_mats_sf(const matrix_sf *mat1, const matrix_sf *mat2) {
    int values[mat1->num_rows * mat2->num_cols];
    size_t i, j, k;
    for (i = 0; i < mat1->num_rows; i++) {
        for (j = 0; j < mat2->num_cols; j++) {
            values[j + i * mat2->num_cols] = 0;
            for (k = 0; k < mat1->num_cols; k++)
                values[j + i * mat2->num_cols] += mat1->values[k + i * mat1->num_cols] * mat2->values[j + k * mat2->num_cols];
        }
    }
   return copy_matrix(mat1->num_rows, mat2->num_cols, values);
}

// Transpose a matrix
matrix_sf* transpose_mat_sf(const matrix_sf *mat) {
    int values[mat->num_rows * mat->num_cols];
    size_t i, j;
    for (i = 0; i < mat->num_rows; i++)
        for (j = 0; j < mat->num_cols; j++)
            values[j * mat->num_rows + i] = mat->values[i * mat->num_cols + j];
    return copy_matrix(mat->num_cols, mat->num_rows, values);
}

// Extract a substring from a given range
char* substring(const char* start, const char* end){
    size_t length = end - start;
    char* str = (char*) malloc(length + 1);
    strncpy(str, start, length);
    *(str + length) = '\0';
    return str;
}

// Extract an integer from a substring
unsigned int extractNum(const char** expr) {
    while (!isdigit(**expr)) (*expr)++;
    const char* START = *expr;
    while (isdigit(**expr)) (*expr)++;
    const char* STR = substring(START, *expr);
    const unsigned int NUM = (unsigned int) strtoul(STR, NULL, BASE_10);
    free((void*) STR);
    return NUM;
}

// Create a matrix from a given name and expression
matrix_sf* create_matrix_sf(char name, const char *expr) {
    const unsigned int ROWS = extractNum(&expr);
    const unsigned int COLUMNS = extractNum(&expr);
    // Parsing list of values
    while (*expr++ != '[');
    const char *START = expr++;
    while (*expr != ']') expr++;
    const char* DELIMITER = " \t\n\r\f\v;,";
    char *token = strtok(substring(START, expr), DELIMITER);
    int index = 0, values[ROWS * COLUMNS];
    while (token != NULL && index < MAX_LINE_LEN) {
        values[index] = atoi(token);
        token = strtok(NULL, DELIMITER);
        while (token && *token && strchr(DELIMITER, *token)) token++;
        index++;
    }
    // Creating and returning matrix
    matrix_sf* m = copy_matrix(ROWS, COLUMNS, values);
    m->name = name;
    return m;
}

// Get the precedence of an operator for infix to postfix conversion
int getPrecedence(char op) {
    switch (op) {
        case '\'': return 3;
        case '*': return 2;
        case '+': return 1;
        default: return 0;
    }
}

// Convert an infix expression to postfix
char* infix2postfix_sf(char *infix) {
    const size_t LEN = strlen(infix);
    char *postfix = (char *)malloc(LEN + 1);
    char stack[LEN];
    int top = -1;
    size_t i, j = 0;
    for (i = 0; i < LEN; i++) {
        const char CUR = infix[i];
        if (isspace(CUR)) continue; // Ignore spaces
        if ((CUR >= 'A' && CUR <= 'Z')) postfix[j++] = CUR; // Operand found
        //Handling parentheses
        else if (CUR == '(') stack[++top] = CUR;
        else if (CUR == ')') {
            while (top != -1 && stack[top] != '(') postfix[j++] = stack[top--];
            top--;
        }
        else { // Operator found
            while (top != -1 && getPrecedence(CUR) <= getPrecedence(stack[top]))
                postfix[j++] = stack[top--];
            stack[++top] = CUR;
        }
    }
    while (top != -1) postfix[j++] = stack[top--]; // Adding final items in stack to postfix
    postfix[j] = '\0';
    return postfix;
}

// Evaluate a matrix expression in postfix notation
matrix_sf* evaluate_expr_sf(char name, char *expr, bst_sf *root) {
    const char* POSTFIX = infix2postfix_sf(expr);
    const size_t LEN = strlen(POSTFIX);
    matrix_sf* stack[LEN / 2];
    int top = -1;
    size_t i;
    for (i = 0; i < LEN; i++) {
        switch (POSTFIX[i]) {
            case '\'':
                // Transpose the matrix at the top of the stack
                stack[top] = transpose_mat_sf(stack[top]);
                break;
            case '*':
                // Multiply matrices
                top--; // Decrement top first
                stack[top] = mult_mats_sf(stack[top], stack[top + 1]);
                break;
            case '+':
                // Add matrices
                top--; // Decrement top first
                stack[top] = add_mats_sf(stack[top], stack[top + 1]);
                break;
            default:
                // Push the matrix onto the stack
                stack[++top] = find_bst_sf(POSTFIX[i], root);
                break;
        }
    }
    free((void*) POSTFIX);
    stack[top]->name = name;
    return stack[top];
}

// Execute a script provided in a file and return the resulting matrix
matrix_sf *execute_script_sf(char *filename) {
    bst_sf *root = NULL;
    matrix_sf *m = NULL;
    size_t max_line_size = MAX_LINE_LEN;
    char *str = (char*) malloc(max_line_size + 1);
    FILE *file = fopen(filename, "r");
    while (getline(&str, &max_line_size, file) != -1) {
        char name = *str;
        while (*str++ != '=');
        while (isspace(*str)) str++;
        m = isdigit(*str) ? create_matrix_sf(name, str) : evaluate_expr_sf(name, str, root);
        root = insert_bst_sf(m, root);
    }
    fclose(file);
    return m;
}

// This is a utility function used during testing. Feel free to adapt the code to implement some of
// the assignment. Feel equally free to ignore it.
matrix_sf *copy_matrix(unsigned int num_rows, unsigned int num_cols, int values[]) {
    matrix_sf *m = malloc(sizeof(matrix_sf)+num_rows*num_cols*sizeof(int));
    m->name = '?';
    m->num_rows = num_rows;
    m->num_cols = num_cols;
    memcpy(m->values, values, num_rows*num_cols*sizeof(int));
    return m;
}

// Don't touch this function. It's used by the testing framework.
// It's been left here in case it helps you debug and test your code.
void print_matrix_sf(matrix_sf *mat) {
    assert(mat != NULL);
    assert(mat->num_rows <= 1000);
    assert(mat->num_cols <= 1000);
    printf("%d %d ", mat->num_rows, mat->num_cols);
    for (unsigned int i = 0; i < mat->num_rows*mat->num_cols; i++) {
        printf("%d", mat->values[i]);
        if (i < mat->num_rows*mat->num_cols-1)
            printf(" ");
    }
    printf("\n");
}
