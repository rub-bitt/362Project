#ifndef __COMMANDS_H__
#define __COMMANDS_H__

struct commands_t {
    const char *cmd;
    void      (*fn)(int argc, char *argv[]);
};




void command_shell(void);
void exec(int argc, char *argv[]);
void parse_command(char *c);
void add(int argc, char *argv[]);
void mul(int argc, char *argv[]);

// struct commands_t cmds[] = {
//     // { "append", append },
//     { "cat", cat },
//     // { "cd", cd },
//     // { "date", date },
//     // { "dino", dino },
//     { "input", input },
//     // { "lcd_init", lcd_init },
//     { "ls", ls },
//     //{ "mkdir", mkdir },
//     { "mount", mount },
//     // { "pwd", pwd },
//     // { "rm", rm },
//     // { "shout", shout },
//     // { "clear",    clear },
//     // { "drawline", drawline },
//     // { "drawrect", drawrect },
//     // { "drawfillrect", drawfillrect },
// };





#endif /* __COMMANDS_H_ */
