#define _CRT_SECURE_NO_WARNINGS
#define _UTIL_H_
#define _SCAN_H_
#define _GLOBALS_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXRESERVED 8
#define MAXTOKENLEN 40
#define BUFLEN 256
#define TRUE 1
#define FALSE 0

char tokenString[MAXTOKENLEN + 1];
static char lineBuf[BUFLEN];
static int linepos = 0; //linebuf current position
static int bufsize = 0; //current size of buffer string
int lineno=TRUE;
int EchoSource=TRUE;
int TraceScan=TRUE;

FILE* source; //source file
FILE* listing; //listing output text file

typedef enum
{START, INNUM, INID, INCAL, MULTI_CAL,INCH, DONE,INASSIGN,MULTI_ASSIGN,IN_COMMENT1,IN_COMMENT2, IN_COMMENT3, IN_COMMENT_FINAL,ERROR_STATE} StateType;

typedef enum {
	//book keeping tokens
	ENDFILE, ERROR,
	//reserved word
	IF, THEN, ELSE, END, REPEAT, UNTIL, READ, WRITE,
	//multicharacter token
	ID, NUM,
	//special symbols
	ASSIGN, LPAREN, RPAREN, SEMI, OPEN, CLOSE, DOT, COMMA, MULTI, CAL, MUL_ASSIGN, BACKSLASH, INDEX_OPEN, INDEX_CLOSE, QUESTION,SHAP
}TokenType;

static struct {
	char* str;
	TokenType tok;
} reservedWords[MAXRESERVED] = { {"if",IF},{"then",THEN},{"else",ELSE},{"end",END}, {"repeat",REPEAT},{"until",UNTIL},{"read",READ},{"write",WRITE} };

static char getNextChar(void);
static void ungetNextChar(void);
static TokenType reservedLookup(char* s, TokenType currentToken);
TokenType getToken(void);
void printToken(TokenType token, const char* tokenString);
void Trace_Scan(TokenType currentToken);
void init_char(char* a);

int main() {
	char pgm[20];
	char token[MAXTOKENLEN];
	//소스 파일 입력
	scanf("%s", pgm);

	source = fopen(pgm, "r");

	if (source == NULL) {
		fprintf(stderr, "FILE is not found\n");
		exit(1);
	}

	listing = stdout;
	fprintf(listing, "\n TINY COMLIATION: %s\n", pgm);

	getToken();
}

void printToken(TokenType token, const char* tokenString) {
	switch (token) {
	case IF:
	case THEN:
	case ELSE:
	case END:
	case REPEAT:
	case UNTIL:
	case READ:
	case WRITE:
		fprintf(listing, "reserved words: %s\n", tokenString);
		break;
	case ASSIGN: 
		fprintf(listing, ":\n"); 
		break;
	case QUESTION:
		fprintf(listing, "?\n");
		break;
	case SHAP:
		fprintf(listing, "#\n");
		break;
	case BACKSLASH:
		fprintf(listing, "/\n");
		break;
	case MUL_ASSIGN:
		fprintf(listing, ":=\n");
		break;
	case LPAREN: 
		fprintf(listing, "{\n"); 
		break;
	case RPAREN: 
		fprintf(listing, "}\n"); 
		break;
	case INDEX_OPEN:
		fprintf(listing, "[\n");
		break;
	case INDEX_CLOSE:
		fprintf(listing, "]\n");
		break;
	case SEMI: 
		fprintf(listing, ";\n"); 
		break;
	case OPEN:
		fprintf(listing, "(\n");
		break;
	case CLOSE:
		fprintf(listing, ")\n");
		break;
	case DOT:
		fprintf(listing, ".\n");
		break;
	case COMMA:
		fprintf(listing, ",\n");
		break;
	case ENDFILE: 
		fprintf(listing, "EOF\n"); 
		break;
	case CAL:
		fprintf(listing, "%s\n", tokenString);
		break;
	case MULTI:
		fprintf(listing, "%s\n", tokenString);
		break;
	case NUM:
		fprintf(listing,"NUM, val = %s\n", tokenString);
		break;
	case ID:
		fprintf(listing,"ID, name = %s\n", tokenString);
		break;
	case ERROR:
		fprintf(listing,"ERROR: %s\n", tokenString);
		break;
	default: 
		fprintf(listing, "Unknown token, %d\n", token);
	}
}

static char getNextChar(void) {
	if (!(linepos < bufsize)) {
		lineno++;
		if (fgets(lineBuf, BUFLEN - 1, source)) {
			if (EchoSource) {
				fprintf(listing, "%4d: %s", lineno, lineBuf);
				bufsize = strlen(lineBuf);
				linepos = 0;
				return lineBuf[linepos++];
			}
		}
		else {
			return EOF;
		}
	}
	else {
		return lineBuf[linepos++];
	}
}

static void ungetNextChar(void) {
	linepos--;
}

static TokenType reservedLookup(char* s, TokenType currentToken) {
	int i;
	for (i = 0; i < MAXRESERVED; i++) {
		if (!strcmp(s, reservedWords[i].str)) {
			return reservedWords[i].tok;
		}
	}
	return currentToken;
}

//getToken
TokenType getToken(void) {
	int tokenStringIndex = 0;

	TokenType currentToken=NUM;
	int check = 0;
	StateType state = START;
	int save;
	while ((currentToken!=ENDFILE)) {
		int c = getNextChar();
		save = TRUE;
		switch (state) {
		case START:
			if (isdigit(c)) {
				state = INNUM;
			}
			else if (isalpha(c)) {
				state = INID;
			}
			else if ((c == '+') || (c == '-') || (c == '*') || (c == '>') || (c == '<') || (c == '=') || (c == '!')) {
				state = INCAL;
			}
			else if (c == '/') {
				state = IN_COMMENT1;
			}
			else if (c == ':') {
				state = INASSIGN;
			}
			else if ((c == ' ') || (c == '\t') || (c == '\n')) {
				save = FALSE;
			}
			//other
			else {
				state = DONE;
				switch (c) {
				case EOF:
					save = FALSE;
					currentToken = ENDFILE;
					break;
				case '?':
					currentToken = QUESTION;
					break;
				case '[':
					currentToken = INDEX_OPEN;
					break;
				case ']':
					currentToken = INDEX_CLOSE;
					break;
				case '{':
					currentToken = LPAREN;
					break;
				case '}':
					currentToken = RPAREN;
					break;
				case ';':
					currentToken = SEMI;
					break;
				case '(':
					currentToken = OPEN;
					break;
				case ')':
					currentToken = CLOSE;
					break;
				case '.':
					currentToken = DOT;
					break;
				case ',':
					currentToken = COMMA;
					break;
				case '#':
					currentToken = SHAP;
					break;
				default:
					currentToken = FALSE;
					break;
				}
			}
			break;
		case INNUM:
			if (!isdigit(c)) {
				ungetNextChar();
				save = FALSE;
				currentToken = NUM;
				state = DONE;
			}
			break;
		case INID:
			if (!isalpha(c)) {
				ungetNextChar();
				save = FALSE;
				currentToken = ID;
				state = DONE;
			}
			break;
		case INCAL:
			if (c == '=') {
				save = TRUE;
				state = MULTI_CAL;
			}
			else {
				state = DONE;
				save = FALSE;
				currentToken = CAL;
			}
			break;
		case INASSIGN:
			if (c == '=') {
				save = TRUE;
				state = MULTI_ASSIGN;
			}
			else {
				state = DONE;
				save = FALSE;
				currentToken = ASSIGN;
			}
			break;
		case MULTI_ASSIGN:
			save = FALSE;
			state = DONE;
			currentToken = MUL_ASSIGN;
			break;
		case MULTI_CAL:
			save = FALSE;
			state = DONE;
			currentToken = MULTI;
			break;
		case IN_COMMENT1:
			if (c == '*') {
				save = FALSE;
				state = IN_COMMENT2;
				tokenString[0] = '\0';
				tokenStringIndex = 0;
			}
			else if (c == '=') {
				save = TRUE;
				state = MULTI_CAL;
			}
			else {
				save = TRUE;
				state = DONE;
				currentToken = BACKSLASH;
			}
			break;
		case IN_COMMENT2:
			save = FALSE;
			if (c == '*') {
				save = FALSE;
				state = IN_COMMENT3;
			}
			break;
		case IN_COMMENT3:
			save = FALSE;
			if (c == '/') {
				state = IN_COMMENT_FINAL;
			}
			else {
				state = ERROR_STATE;
			}
			break;
		case IN_COMMENT_FINAL:
			save = FALSE;
			if ((c == '\n') || (c == '\t')) {
				state = START;
			}
			else {
				state = ERROR_STATE;
			}
			break;
		case ERROR_STATE:
			fprintf(listing, "\tstop ending before\n");
			state = DONE;
			save = FALSE;
			currentToken = ERROR;
			return;
			break;
		case DONE:
			state = START;
			tokenStringIndex = 0;
			save = FALSE;
			ungetNextChar();
			break;
		}
		if ((save) && (tokenStringIndex <= MAXTOKENLEN)) {
			tokenString[tokenStringIndex++] = c;
		}
		if (state == DONE) {
			check = 1;
			tokenString[tokenStringIndex++] = '\0';
			currentToken = reservedLookup(tokenString, currentToken);	
			
		}
		if (check == 1) {
			Trace_Scan(currentToken);
			init_char(tokenString);
			check = 0;
		}
	}
	
	return currentToken;
}

void Trace_Scan(TokenType currentToken) {
	fprintf(listing, "\t%d : ", lineno);
	printToken(currentToken, tokenString);
}

void init_char(char* a) {
	int len = strlen(a);
	for (int i = 0; i < len; i++) {
		a[i] = '\0';
	}
}