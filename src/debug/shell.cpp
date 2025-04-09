#include "shell/shell.h"

#define DEBUG_MODE 0

#define MAX_ARGS 64
#define MAX_ARG_LENGTH 256

#define STR_PROMPT "Shell> "
#define STR_PROMPT_ERR "Shell[%d]> "
#define STR_BUFFER_OVERFLOW_Fd "\r\nShell buffer overflow! Max length %d\r\nBuffer clean...\r\nWrite the command again\r\n"

#define INDEX_NULLTERM sizeof('\0')
#define INDEX_OFFSET 1
#define INDEX_RESET 0
#define ARRAY_SIZE(_array) (sizeof(_array) / sizeof(*_array))
#define SHELL_BUFFER_SIZE (128 + INDEX_NULLTERM) * sizeof(charshell_t)

shelllist commands[] = {
    {"relay", shellmain_relay},
    {"led", shellmain_led},
    {"echo", shellmain_echo},
    {"wifi", shellmain_wifi},
    {"shell", shellmain_shell},
    {"termo", shellmain_termo},
    {"ls", shellmain_ls},
    {"pwd", shellmain_pwd},
    {"cd", shellmain_cd},
    {"cat", shellmain_cat},
    {NULL, NULL}};

static charshell_t shell_buffer[SHELL_BUFFER_SIZE];
static charshell_t command_buffer[SHELL_BUFFER_SIZE];
struct XSHELL
{
    charshell_t *const buffer;
    charshell_t *const command;
    int index;
    bool buffer_overflow : 1;
    bool get_finish : 1;
    bool command_start : 1;
    int argc;
    char **argv;
    int retval;
} xshell = {
    .buffer = (charshell_t *const)&shell_buffer,
    .command = (charshell_t *const)&command_buffer,
    .index = INDEX_RESET,
    .buffer_overflow = false,
    .get_finish = false,
    .command_start = true,

    .argc = 0,
    .argv = NULL,
    .retval = 0,
};

void shell_command_list(void)
{
    for (int i = 0; commands[i].name != NULL; ++i)
    {
        printf("%s ", commands[i].name);
        fflush(stdout);
    }

    printf("\n");
}

static shellmain find_command(const char *commandName)
{
    for (int i = 0; commands[i].name != NULL; ++i)
    {
        if (strcmp(commandName, commands[i].name) == 0)
            return commands[i].handler;
    }
    return NULL;
}

void free_argv(int argc, char **argv)
{
    if (argv != NULL)
    {
        for (int i = 0; i < argc; i++)
        {
            free(argv[i]);
        }
        free(argv);
    }

    xshell.argv = NULL;
}

static void if_shell_buffer_overflow(const char c)
{
    if (!xshell.buffer_overflow)
        return;

    Serial.print("Index: ");
    Serial.println(xshell.index);

    xshell.index = INDEX_RESET;
    memset(xshell.buffer, 0, ARRAY_SIZE(shell_buffer));
    xshell.buffer_overflow = false;
    char buffer[ARRAY_SIZE(STR_BUFFER_OVERFLOW_Fd)];
    sprintf(buffer, STR_BUFFER_OVERFLOW_Fd, SHELL_BUFFER_SIZE - INDEX_NULLTERM);
    Serial.println(buffer);
}

static int parse_command_line(char *command, int *argc, char ***argv)
{
    char **args = (char **)malloc(MAX_ARGS * sizeof(char *));
    if (args == NULL)
    {
        Serial.println("(Shell): Alloc error");
        return SHELL_ERROR;
    }

    *argc = 0;
    char *str_copy = strdup(command);
    if (str_copy == NULL)
    {
        Serial.println("(Shell): strdup() failed");
        free(args);
        return SHELL_ERROR;
    }

    char *token = strtok(str_copy, " ");
    while (token != NULL)
    {
        if (*argc >= MAX_ARGS)
        {
            Serial.print("Too many arguments (max ");
            Serial.print(MAX_ARGS);
            Serial.println(")");
            break;
        }

        args[*argc] = (char *)malloc(MAX_ARG_LENGTH);
        if (args[*argc] == NULL)
        {
            Serial.println("(Shell): Alloc error");
            for (int i = 0; i < *argc; i++)
            {
                free(args[i]);
            }
            free(args);
            free(str_copy);

            return SHELL_ERROR;
        }

        strncpy(args[*argc], token, MAX_ARG_LENGTH - 1);
        args[*argc][MAX_ARG_LENGTH - 1] = '\0';

        (*argc)++;

        token = strtok(NULL, " ");
    }

    *argv = args;
    free(str_copy);

    return 0;
}

static bool if_end_command(const char c)
{
    if (c != '\n')
        return false;

    strcpy(xshell.command, xshell.buffer);
#if DEBUG_MODE
    Serial.println(xshell.buffer);
    Serial.println(xshell.command);
#endif
    memset(xshell.buffer, 0, ARRAY_SIZE(shell_buffer));
    xshell.index = 0;
    xshell.get_finish = true;

    if (xshell.argv != NULL)
    {
        free_argv(xshell.argc, xshell.argv);
    }

    if (xshell.command[0] != '\n' && xshell.command[0] != '\0')
        parse_command_line(xshell.command, &xshell.argc, &xshell.argv);

    return true;
}

static void buffer_filling(const char c)
{
    if (if_end_command(c) || c == '\0')
        return;

    if (!(xshell.index < SHELL_BUFFER_SIZE - INDEX_OFFSET))
    {
        xshell.buffer_overflow = true;
        return;
    }

    xshell.buffer[xshell.index++] = c;
}

static void echo_printed_characters(void)
{
    char incomingByte = '\0';

    if (Serial.available() > 0)
    {
        incomingByte = Serial.read();
        Serial.print(incomingByte);
    }

    buffer_filling(incomingByte);
    if_shell_buffer_overflow(incomingByte);
}

#if DEBUG_MODE
static int xshellmaindebug(int argc, char **argv)
{
    Serial.print("argc: ");
    Serial.println(argc);

    int base = argc;
    int index = 0;
    while (base--)
    {
        Serial.print("argv: ");
        Serial.println(argv[index++]);
    }
}
#else
#define xshellmaindebug(...)
#endif

static int xshellmain(int argc, char **argv)
{
    shellmain main = NULL;

    if (argv == NULL)
        return EXIT_SUCCESS;

    xshellmaindebug(argc, argv);

    if ((main = find_command(argv[0])) != NULL)
    {
        return main(argc, argv);
    }

    Serial.print("(Shell) Unknown command: ");
    Serial.print(argv[0]);
    Serial.println("");

    return SHELL_ERROR;
}

static void xshellmainhook(void)
{
    if (!xshell.get_finish)
        return;
    int retval = 0;

    retval = xshellmain(xshell.argc, xshell.argv);

    xshell.retval = retval;
    xshell.get_finish = false;
    xshell.command_start = true;

    free_argv(xshell.argc, xshell.argv);
}

static void prompt(void)
{
    if (!xshell.command_start)
        return;

    char buffer[SHELL_BUFFER_SIZE];
    sprintf(buffer, (xshell.retval == 0) ? STR_PROMPT : STR_PROMPT_ERR, xshell.retval);
    Serial.print(buffer);

    xshell.command_start = false;
}

void shell(void)
{
    if (!xshell.get_finish)
    {
        prompt();
        echo_printed_characters();
    }
    xshellmainhook();
}
