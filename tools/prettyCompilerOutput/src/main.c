#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

// #define DEBUG

typedef enum {
    Begin,
    File,
    Line,
    Error,
    Type,
    Description,
    Note
} ParserState;

typedef struct {
    char *buff;
    int len;
    int maxLen;
} Buffer;

void init(Buffer *buff);
void append(Buffer *buff, char c);

int main(int argc, char **argv) {

    char buff[4096];
    size_t bytesRead = 0;

    ParserState state = Begin;

    Buffer compiledFile = {0};

    bool isLinkError = false;
    bool isError = false;
    bool hasNote = false;
    Buffer fileName = {0};
    Buffer lineText = {0};
    Buffer type = {0};
    Buffer description = {0};
    Buffer note = {0};

    init(&compiledFile);
    init(&fileName);
    init(&lineText);
    init(&type);
    init(&description);
    init(&note);

    do {    
        bytesRead = fread(buff, sizeof(char), 4096, stdin);

#ifdef DEBUG
        char msg[] = "Reading in: \n";
        char endMsg[] = "\n\nProgramOutput:\n";
        fwrite(msg, sizeof(char), sizeof(msg), stdout);
        fwrite(buff, sizeof(char), bytesRead, stdout);
        fwrite(endMsg, sizeof(char), sizeof(endMsg), stdout);
#endif

        size_t cur = 0;

        while (cur < bytesRead) {

            char curChar = buff[cur];

            if (state == Begin) {
                if (curChar == '\n') {
                    append(&compiledFile, '\0');
                    printf("%s\n", compiledFile.buff);
                    state = File;
                }
                else {
                    append(&compiledFile, curChar);
                }
            }
            else if (state == File) {
                // Assuming that parentheses cannot be in paths
                if (curChar == '(') {
                    // We're at the end of the file
                    append(&fileName, '\0');
                    state = Line;
                }
                // Assuming that colons cannot be in paths
                else if (curChar == ':' && fileName.buff[fileName.len - 1] == ' ') {
                    isLinkError = true;
                    cur++; // There's a space after the colon
                    fileName.len--; // Space before colon in link error
                    append(&fileName, '\0');
                    state = Error;
                }
                else {
                    append(&fileName, curChar);
                }
            }
            else if (state == Line) {
                if (curChar == ')') {
                    // We're at the end of the line
                    append(&lineText, '\0');
                    cur += 2; // move past the colon and space
                    state = Error;
                }
                else {
                    append(&lineText, curChar);
                }
            }
            else if (state == Error) {
                if (curChar == 'f') {
                    isError = true;
                    cur += 5 + 1 + 5; // fatal error
                }
                else if (curChar == 'e') {
                    isError = true;
                    cur += 5;
                }
                else if (curChar == 'w') {
                    isError = false;
                    cur += 7;
                }
                else {
                    fprintf(stderr, "Unknown error char: %c\n", curChar);
                }
                state = Type;
            }
            else if (state == Type) {
                if (curChar == ':') {
                    append(&type, '\0');
                    cur++;
                    state = Description;
                }
                else {
                    append(&type, curChar);
                }
            }
            else if (state == Description) {
                if (curChar == '\n') {
                    append(&description, '\0');
                    state = File;

                    char *subFile = strstr(fileName.buff, "src");
                    if (subFile == NULL)
                        subFile = strstr(fileName.buff, "src");
                    if (subFile == NULL) {
#ifdef DEBUG
                        fprintf(stderr, "Couldn't shorten file name\n");
#endif
                        subFile = fileName.buff;
                    }

                    if (hasNote) {
                        // Bold white
                        printf("\033[1;37m");
                        printf("%s:", subFile);
                        if (!isLinkError)
                            printf("%s:", lineText.buff);
                        
                        // Cyan
                        printf("\033[0;36m");
                        printf(" note: ");
                        printf("\033[0m");

                        printf("%s\n", note.buff);
                    }

                    // Print out error
                    // Bold white
                    printf("\033[1;37m");
                    printf("%s:", subFile);
                    if (!isLinkError)
                        printf("%s:", lineText.buff);

                    if (isError) {
                        // Red
                        printf("\033[0;31m");
                        printf(" error %s: ", type.buff);
                    }
                    else {
                        // Purple
                        printf("\033[0;35m");
                        printf(" warning %s: ", type.buff);
                    }
                    printf("\033[0m");

                    printf("%s\n", description.buff);

                    int line = atoi(lineText.buff);

                    if (line != 0) {

                        // Print out line where it occurs
                        FILE *file = fopen(fileName.buff, "r");

                        if (file != NULL) {

                            int lineNum = 1;
                            Buffer lineOutput = {0};

                            init(&lineOutput);

                            while (lineNum <= line) {
                                int next = fgetc(file);
                                if (next == EOF)
                                    break;

                                char c = (char)next;
                                if (c == '\n') {
                                    lineNum += 1;
                                    continue;
                                }

                                if (lineNum == line) {

                                    append(&lineOutput, c);
                                }
                            }

                            append(&lineOutput, '\0');

                            if (lineOutput.len > 1) {
                                char *textBegin = lineOutput.buff;
                                while (textBegin - lineOutput.buff < lineOutput.len
                                        && *textBegin == ' ')
                                {
                                    textBegin++;
                                }

                                printf("        %s\n\n", textBegin);
                            }
                            
                            fclose(file);

                            free(lineOutput.buff);
                        }
                        else {
#ifdef DEBUG
                            printf("Couldn't find file\n");
#endif
                        }
                    }

                    fileName.len = 0;
                    lineText.len = 0;
                    type.len = 0;
                    description.len = 0;
                    note.len = 0;
                    hasNote = false;
                    isError = false;
                    isLinkError = false;
                }
                else if (curChar == 'e') {
                    append(&description, curChar);

                    if (strncmp(description.buff + description.len - 4, "note", 4) == 0) {
                        hasNote = true;
                        cur += 2; // : space
                        state = Note;
                        description.len -= 6; // : space, note 
                    }
                }
                else {
                    append(&description, curChar);
                }
            }
            else if (state == Note) {
                if (curChar == '\n') {
                    cur--;
                    state = Description;
                    append(&note, '\0');
                }
                else {
                    append(&note, curChar);
                }
            }

            cur++;
        }

    } while (bytesRead == 4096);

    if (bytesRead == -1) {
        fprintf(stderr, "Error Occured reading from stdin\n");
    }

    free(compiledFile.buff);
    free(fileName.buff);
    free(lineText.buff);
    free(type.buff);
    free(description.buff);

    return 0;
}

void init(Buffer *buff) {
    buff->len = 0;
    buff->maxLen = 64;
    buff->buff = malloc(buff->maxLen);
}

void append(Buffer *buff, char c) {
    if (buff->len == buff->maxLen) {
        buff->maxLen *= 2;
        buff->buff = realloc(buff->buff, buff->maxLen);
    }

    buff->buff[buff->len] = c;
    buff->len++;
}
