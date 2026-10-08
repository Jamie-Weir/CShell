//HISTORY
char* history[20] = {0};
int history_length = 0;
void shell_add_history(char* input);
int history_alias = 0;

//ALIAS
int alias_number;
int aliascommands_number;
char* alias_commands[10][512];
char* aliases[10];
int offset = 0;
int alias_limit = 0;
int alias_found = 0;

