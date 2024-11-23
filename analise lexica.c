#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKENS 1000
#define MAX_TOKEN_LENGTH 100

// Enumeração para tipos de tokens
typedef enum {
    KEYWORD,
    TYPE,
    IDENTIFIER,
    NUM_LITERAL,
    STRING_LITERAL,
    SEMICOLON,
    COMMA,
    OPERATOR,
    ASSIGNMENT,
    OPEN_PARENTHESIS,
    CLOSE_PARENTHESIS,
    OPEN_BRACE,
    CLOSE_BRACE,
    OPEN_BRACKET,
    CLOSE_BRACKET,
    COMPARATOR,
    UNKNOWN
} TokenType;

// Estrutura para armazenar um token
typedef struct {
    char value[MAX_TOKEN_LENGTH];
    int line;
    TokenType type;
    int size;  // Novo campo: Tamanho do token
} Token;

// Lista de palavras-chave e tipos
const char *keywords[] = {"if", "else", "while", "for", "return", "break", "continue", "switch", "case", "default"};
const char *types[] = {"int", "float", "char", "double", "void", "long", "short", "signed", "unsigned"};

// Variáveis globais
Token tokens[MAX_TOKENS];
int tokenCount = 0;

// Função para identificar o tipo de token
TokenType identifyTokenType(const char *word) {
    for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(word, keywords[i]) == 0)
            return KEYWORD;
    }
    for (int i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
        if (strcmp(word, types[i]) == 0)
            return TYPE;
    }
    if (isdigit(word[0]) || (word[0] == '-' && isdigit(word[1])))
        return NUM_LITERAL;
    if (isalpha(word[0]) || word[0] == '_')
        return IDENTIFIER;
    return UNKNOWN;
}

// Função para adicionar um token à lista
void addToken(const char *value, int line, TokenType type) {
    if (tokenCount >= MAX_TOKENS) {
        fprintf(stderr, "Erro: Número máximo de tokens excedido.\n");
        exit(EXIT_FAILURE);
    }
    Token *token = &tokens[tokenCount++];
    strncpy(token->value, value, MAX_TOKEN_LENGTH);
    token->line = line;
    token->type = type;
    token->size = strlen(value); // Armazena o tamanho do token
}

// Função principal de análise léxica
void lexicalAnalysis(const char *code) {
    const char *ptr = code;
    int lineNumber = 1;

    while (*ptr) {
        // Ignorar espaços e quebras de linha
        if (isspace(*ptr)) {
            if (*ptr == '\n') {
                lineNumber++;
            }
            ptr++;
            continue;
        }

        // Ignorar comentários de linha
        if (*ptr == '/' && *(ptr + 1) == '/') {
            while (*ptr && *ptr != '\n') ptr++;
            continue;
        }

        // Ignorar comentários de bloco '/**/'
        if (*ptr == '/' && *(ptr + 1) == '*') {
            ptr += 2; // Avançar sobre '/*'
            int insideBlockComment = 1;
            while (insideBlockComment && *ptr) {
                if (*ptr == '*' && *(ptr + 1) == '/') {
                    ptr += 2;
                    insideBlockComment = 0;
                } else if (*ptr == '\n') {
                    lineNumber++;
                    ptr++;
                } else {
                    ptr++;
                }
            }
            continue;
        }

        // Delimitadores
        if (strchr(";,(){}[]", *ptr)) {
            char token[2] = {*ptr, '\0'};
            addToken(token, lineNumber, (*ptr == ';') ? SEMICOLON :
                                          (*ptr == ',') ? COMMA :
                                          (*ptr == '(') ? OPEN_PARENTHESIS :
                                          (*ptr == ')') ? CLOSE_PARENTHESIS :
                                          (*ptr == '{') ? OPEN_BRACE :
                                          (*ptr == '}') ? CLOSE_BRACE :
                                          (*ptr == '[') ? OPEN_BRACKET :
                                          CLOSE_BRACKET);
            ptr++;
            continue;
        }

        // Verificador de Operadores e atribuidores
        if (strchr("=+-*/><!", *ptr)) {
            char token[3] = {*ptr, '\0', '\0'};
            if (*(ptr + 1) == '=') {
                token[1] = *(ptr + 1);
                ptr++;
            }
            addToken(token, lineNumber, (*ptr == '=') ? ASSIGNMENT : OPERATOR);
            ptr++;
            continue;
        }

        // Verificação de números (incluindo números de ponto flutuante)
        if (isdigit(*ptr) || (*ptr == '.' && isdigit(*(ptr + 1)))) {
            char number[MAX_TOKEN_LENGTH];
            int length = 0;
            int hasDot = 0;
            while (isdigit(*ptr) || (*ptr == '.' && !hasDot)) {
                if (*ptr == '.') {
                    hasDot = 1; // Marca a presença de um ponto decimal
                }
                if (length < MAX_TOKEN_LENGTH - 1) {
                    number[length++] = *ptr;
                }
                ptr++;
            }
            number[length] = '\0';
            addToken(number, lineNumber, NUM_LITERAL);
            continue;
        }

        // Identificadores e palavras-chave
        if (isalnum(*ptr) || *ptr == '_') {
            char word[MAX_TOKEN_LENGTH];
            int length = 0;
            while (isalnum(*ptr) || *ptr == '_') {
                if (length < MAX_TOKEN_LENGTH - 1) {
                    word[length++] = *ptr;
                }
                ptr++;
            }
            word[length] = '\0';
            addToken(word, lineNumber, identifyTokenType(word));
            continue;
        }

        // Verificador de Token desconhecido
        char unknown[2] = {*ptr, '\0'};
        addToken(unknown, lineNumber, UNKNOWN);
        ptr++;
    }
}

// Função para exibir os tokens
void printTokens() {
    printf("\nTokens encontrados:\n");
    for (int i = 0; i < tokenCount; i++) {
        printf("Token: %-15s Linha: %-4d Tipo: %-2d Tamanho: %-3d Byte\n",
               tokens[i].value, tokens[i].line, tokens[i].type, tokens[i].size);
    }
}

// Função principal
int main() {
    // Abrir o arquivo fonte
    FILE *file = fopen("../input.txt", "r");

    // Verificador de erro
    if (!file) {
        perror("Erro ao abrir o arquivo");
        return EXIT_FAILURE;
    }

    // Ler todo o conteúdo do arquivo
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);
    char *code = malloc(fileSize + 1);

    // Verificador de problema de alocação de memória
    if (!code) {
        perror("Erro ao alocar memória");
        fclose(file);
        return EXIT_FAILURE;
    }
    fread(code, 1, fileSize, file);
    code[fileSize] = '\0';
    fclose(file);

    // Analisar o código
    printf("Analisando código do arquivo: ../input.txt\n");
    lexicalAnalysis(code);
    printTokens();

    // Limpar memória
    free(code);
    return EXIT_SUCCESS;
}