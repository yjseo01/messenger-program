#include <iostream>
#include <ncurses.h>
#include <string.h>

#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_USERS 1024
#define MAX_LETTERS 20
#define PERMS 0666

#define PIPE1 "./C2S"
#define PIPE2 "./S2C"

struct Server_Client_Communication {

    // Request Classification
    enum classification_enum { signup, login, message };
    int classification;

    // Request Content
    char input_name[MAX_LETTERS + 1];
    char input_id[MAX_LETTERS + 1];
    char input_pw[MAX_LETTERS + 1];

    // Request Result;
    enum result_enum { success, id, pw };
    int result;

    // message context
    char *user_list[MAX_USERS];

} typedef communication;

using namespace std;

class userinfo {
  private:
    char name[MAX_LETTERS + 1];
    char id[MAX_LETTERS + 1];
    char pw[MAX_LETTERS + 1];

  public:
    userinfo();
    userinfo(char *n, char *i, char *p);

    void setname(string n) { strcpy(name, n.c_str()); }
    void setid(string i) { strcpy(id, i.c_str()); }
    void setpw(string p) { strcpy(pw, p.c_str()); }

    char *getname() { return name; }
    char *getid() { return id; }
    char *getpw() { return pw; }

    bool isblank();
};

userinfo::userinfo() {
    memset(name, '\0', MAX_LETTERS + 1);
    memset(id, '\0', MAX_LETTERS + 1);
    memset(pw, '\0', MAX_LETTERS + 1);
}

userinfo::userinfo(char *n, char *i, char *p) {
    strcpy(name, n);
    strcpy(id, i);
    strcpy(pw, p);
}

bool userinfo::isblank() {
    if (name[0] == '\0' || id[0] == '\0' || pw[0] == '\0')
        return true;
    return false;
}

void startPage(int fd1, int fd2);
void loginPage(int fd1, int fd2);
void signupPage(int fd1, int fd2);
void mainPage(userinfo u, int fd1, int fd2);
void messageWindow(userinfo u, communication o, int fd1, int fd2);

int main(int argc, char const *argv[]) {
    initscr();

    int fd1, fd2;

    mkfifo(PIPE2, PERMS); // make named pipe S2C

    if ((fd1 = open(PIPE1, O_WRONLY)) == -1) { // open C2S
        cout << "open() C2S error" << endl;
        exit(0);
    }

    if ((fd2 = open(PIPE2, O_RDONLY)) == -1) { // open S2C
        cout << "open() S2C error" << endl;
        exit(0);
    }

    if (has_colors == FALSE) {
        puts("Terminal does not support colors!");
        endwin();
        return 1;
    } else {
        start_color();
        init_pair(1, COLOR_BLUE, COLOR_WHITE);
        init_pair(2, COLOR_WHITE, COLOR_BLUE);
    }
    refresh();

    curs_set(0);
    startPage(fd1, fd2);

    close(fd2);
    endwin();
    return 0;
}

void startPage(int fd1, int fd2) {
    WINDOW *win1 = newwin(24, 80, 0, 0);
    WINDOW *win2 = newwin(6, 80, 18, 0);

    wbkgd(win1, COLOR_PAIR(1));
    wbkgd(win2, COLOR_PAIR(2));

    mvwprintw(win1, 6, 34, "Login");
    mvwprintw(win1, 9, 34, "Sign up");

    wrefresh(win1);
    wrefresh(win2);

    int cur = 1; // cur == 0: login, cur == 1: signup
    noecho();
    cbreak();
    keypad(stdscr, true);

    int key;

    while (key != 27) {
        key = getch();

        if (key == KEY_UP) {
            mvwprintw(win1, 6, 34, "> Login    ");
            mvwprintw(win1, 9, 34, "Sign up    ");
            cur = 0;
        } else if (key == KEY_DOWN) {
            mvwprintw(win1, 6, 34, "Login    ");
            mvwprintw(win1, 9, 34, "> Sign up    ");
            cur = 1;
        } else if (key == 10) {
            if (cur == 0) { // login
                loginPage(fd1, fd2);
            } else if (cur == 1) { // sign up
                signupPage(fd1, fd2);
            }
        }

        wrefresh(win1);
    }
    return;
}

void mainPage(userinfo u, int fd1, int fd2) {

    WINDOW *win1 = newwin(24, 80, 0, 0);
    WINDOW *win2 = newwin(6, 80, 18, 0);

    wbkgd(win1, COLOR_PAIR(1));
    wbkgd(win2, COLOR_PAIR(2));

    // get user_list
    communication myself;
    strcpy(myself.input_id, u.getid());
    myself.classification = myself.message;

    write(fd1, &myself, sizeof(myself));

    read(fd2, &myself, sizeof(myself));

    int message_num = 0;
    for (message_num; message_num < MAX_USERS; message_num++) {
        if (myself.user_list[message_num] == NULL)
            break;
    }

    int user_list_pages; // number of user list
    user_list_pages = message_num / 14;

    message_num % 14 == 0 ? user_list_pages = message_num / 14
                          : user_list_pages = message_num / 14 + 1;

    if (user_list_pages == 0)
        user_list_pages = 1;

    int cur_list_page = 1;

    // print screen
    mvwprintw(win1, 0, 1, "Input user ID: ");
    mvwprintw(win1, 1, 1, ">>");
    for (int r = 0; r < 80; r++)
        mvwprintw(win1, 2, r, "=");

    // print list
    mvwprintw(win1, 3, 0, "User List (%d / %d)", cur_list_page,
              user_list_pages);
    for (int i = 0; i < message_num && i < 14; i++)
        mvwprintw(win1, i + 4, 2, "%s", myself.user_list[i]);

    // print user infomation
    mvwprintw(win2, 0, 0, "ID: ");
    mvwprintw(win2, 0, 4, "%s", u.getid());
    mvwprintw(win2, 1, 0, "Name: ");
    mvwprintw(win2, 1, 6, "%s", u.getname());

    // print logout button
    mvwprintw(win2, 0, 72, "  Logout");

    wrefresh(win1);
    wrefresh(win2);

    int key;
    int cur = 0;
    int escpressed = 0;

    wrefresh(win1);

    while (true) {
        mvwprintw(win1, 0, 16, "                 ");
        mvwprintw(win1, 1, 4, "                  ");
        wrefresh(win1);

        key = getch();
        curs_set(0);

        // print list
        mvwprintw(win1, 3, 0, "User List (%d / %d)", cur_list_page,
                  user_list_pages);
        wrefresh(win1);
        for (int i = 0; i + 14 * (cur_list_page - 1) < message_num && i < 14;
             i++)
            mvwprintw(win1, i + 4, 2, "%s",
                      myself.user_list[i + 14 * (cur_list_page - 1)]);

        if (key != 27)
            escpressed = 0;

        if (key == KEY_RIGHT) { // page up
            if (cur_list_page < user_list_pages - 1) {
                cur_list_page += 1;
            }
            continue;

        } else if (key == KEY_LEFT) { // page down
            if (cur_list_page > 1) {
                cur_list_page -= 1;
            }
            continue;

        } else if (key == KEY_UP) {
            // print reset
            for (int i = 0;
                 i + 14 * (cur_list_page - 1) < message_num && i < 14; i++) {
                mvwprintw(win1, i + 4, 0, " ");
            }
            mvwprintw(win2, 0, 72, "  Logout");

            if (cur > -1 && cur < message_num + 1)
                cur--;

            if (cur == -1) { // input id
                move(0, 16);
                curs_set(1);
            }

            for (int i = 0; i + 14 * (cur_list_page - 1) < message_num &&
                            i < 14 && cur > -1;
                 i++) {
                if (i == cur) {
                    mvwprintw(win1, i + 4, 0, ">");
                }
                if (cur == message_num - 1) {
                    mvwprintw(win2, 0, 72, "  Logout");
                }
            }

            wrefresh(win1);
            wrefresh(win2);

        } else if (key == KEY_DOWN) {
            // print reset
            for (int i = 0;
                 i + 14 * (cur_list_page - 1) < message_num && i < 14; i++) {
                mvwprintw(win1, i + 4, 0, " ");
            }
            mvwprintw(win2, 0, 72, "  Logout");
            if (cur > -1 && cur < message_num)
                cur++;

            if (cur == 14) { // logout selected
                mvwprintw(win2, 0, 72, "> Logout");
            } else {
                for (int i = 1;
                     i + 14 * (cur_list_page - 1) < message_num && i < 14;
                     i++) { // message selected
                    if (i == cur) {
                        mvwprintw(win1, i + 4, 0, ">");
                    }
                }
            }

            wrefresh(win1);
            wrefresh(win2);
        } else {
            if (key == 27) {
                escpressed++;
                if (escpressed == 1) {
                    cur = 14;

                    // reset message
                    for (int i = 0;
                         i + 14 * (cur_list_page - 1) < message_num && i < 14;
                         i++) {
                        mvwprintw(win1, i + 4, 0, " ");
                    }

                    // select logout
                    mvwprintw(win2, 0, 72, "> Logout");
                    wrefresh(win1);
                    wrefresh(win2);
                } else if (escpressed == 2) {
                    mainPage(u, fd1, fd2);
                    return;
                }
            } else if (cur == -1) { // id input

                mvwprintw(win1, 1, 4, "                  ");
                wrefresh(win1);

                char input_id[MAX_LETTERS + 1];
                curs_set(1);

                char ch = key;

                int i = 0;
                while (ch != '\n' && i < MAX_LETTERS && ch != 27) {

                    input_id[i] = ch;
                    mvwprintw(win1, 0, 16 + i, "%c", ch);
                    wrefresh(win1);
                    ch = getch();
                    i++;
                }
                input_id[i] = '\0';

                communication opponent;
                // opponent.input_id = input_id;
                opponent.classification = opponent.message;
                strcpy(opponent.input_id, input_id);

                write(fd1, &opponent, sizeof(communication));

                read(fd2, &opponent, sizeof(communication));

                if (opponent.result == opponent.success) {
                    // enter message window
                    mvwprintw(win1, 1, 4, "%s success!", opponent.input_id);
                    wrefresh(win1);

                    messageWindow(u, opponent, fd1, fd2);

                    break;
                } else if (opponent.result == opponent.id) {
                    mvwprintw(win1, 1, 4, "ID doesn't exist");
                    wrefresh(win1);

                } else {
                    mvwprintw(win1, 1, 4, "Can't input your ID");
                    wrefresh(win1);
                }

                sleep(3);

                curs_set(0);
                move(4, 0);
                cur = 0;
            }
            if (key == 10) {
                if (cur == 14) { // if logout selected
                    break;
                }
            }
        }
    }
    startPage(fd1, fd2);
    return;
}

void messageWindow(userinfo u, communication o, int fd1, int fd2) {
    WINDOW *win1 = newwin(24, 80, 0, 0);
    WINDOW *win2 = newwin(6, 80, 18, 0);

    wbkgd(win1, COLOR_PAIR(1));
    wbkgd(win2, COLOR_PAIR(2));

    // print messages

    mvwprintw(win2, 0, 0, "Message with %s", o.input_id);
    mvwprintw(win2, 1, 0, "Input message: ");

    move(1, 15);
    curs_set(1);

    wrefresh(win1);
    wrefresh(win2);
    string input_message;

    char ch = getch();

    int i = 0;
    while (ch != '\n' && i < MAX_LETTERS && ch != 27) {
        input_message.push_back(ch);
        mvwprintw(win2, 1, 15 + i, "%s", &ch);
        wrefresh(win2);
        ch = getch();
        i++;
    }

    sleep(5);
}

void loginPage(int fd1, int fd2) {
    WINDOW *win1 = newwin(24, 80, 0, 0);
    WINDOW *win2 = newwin(6, 80, 18, 0);

    wbkgd(win1, COLOR_PAIR(1));
    wbkgd(win2, COLOR_PAIR(2));

    mvwprintw(win1, 4, 34, "id: ");
    mvwprintw(win1, 7, 34, "pw: ");
    mvwprintw(win1, 10, 38, "Login   ");

    wrefresh(win1);
    wrefresh(win2);

    int key;
    int cur = -1;
    userinfo *loginuser = (userinfo *)malloc(sizeof(userinfo));
    char loginname[6] = "login";
    loginuser->setname(loginname);

    move(4, 40);
    while (key != 27) { // esc not pressed
        key = getch();
        if (cur == -1) {
            move(4, 38);
            curs_set(1);
            cur = 0;
        }
        if (key == KEY_UP) {
            if (cur == 0) { // id
                continue;
            }
            if (cur == 1) { // pw -> id
                cur = 0;
                move(4, 38);
                curs_set(1);
            } else if (cur == 2) { // login -> pw
                cur = 1;
                mvwprintw(win1, 10, 38, "Login   ");
                wrefresh(win1);
                move(7, 38);
                curs_set(1);
            }

        } else if (key == KEY_DOWN) {
            if (cur == 0) { // id -> pw
                cur = 1;
                move(7, 38);
                curs_set(1);

            } else if (cur == 1) { // pw -> login
                cur = 2;
                curs_set(0);
                mvwprintw(win1, 10, 38, "> Login");
                wrefresh(win1);
            }

        } else {
            if (cur == 2) {
                if (key == 10) { // enter ///////////////////////////////////

                    if (loginuser->isblank()) {
                        mvwprintw(win2, 0, 27, "                           ");
                        mvwprintw(win2, 0, 27, "Please fill out all blanks");
                        wrefresh(win2);

                        int anykey = getch();

                        free(loginuser);
                        loginPage(fd1, fd2);
                    }
                    communication logininfo;
                    strcpy(logininfo.input_id, loginuser->getid());
                    strcpy(logininfo.input_name, loginuser->getname());
                    strcpy(logininfo.input_pw, loginuser->getpw());
                    logininfo.classification = logininfo.login;

                    write(fd1, &logininfo,
                          sizeof(logininfo)); // send login

                    read(fd2, &logininfo, sizeof(logininfo));

                    int anykey;

                    switch (logininfo.result) {
                    case 0: {
                        // success!
                        userinfo logined =
                            userinfo(logininfo.input_name, logininfo.input_id,
                                     logininfo.input_pw);

                        free(loginuser);
                        mainPage(logined, fd1, fd2);
                        return;
                    }
                    case 1: {
                        // id error
                        mvwprintw(win2, 0, 27, "                           ");
                        mvwprintw(win2, 0, 34, "ID doesn't exist");
                        wrefresh(win2);

                        anykey = getch();

                        free(loginuser);
                        loginPage(fd1, fd2);
                        return;
                    }
                    case 2: {
                        // pw error
                        mvwprintw(win2, 0, 27, "                           ");
                        mvwprintw(win2, 0, 27, "pw is wrong");
                        wrefresh(win2);

                        anykey = getch();

                        free(loginuser);

                        loginPage(fd1, fd2);
                        return;
                    }
                    }
                }
            } else if (cur == 1) { // pw input
                string input_pw;
                curs_set(1);

                char ch = key;

                int i = 0;
                while (ch != '\n' && i < MAX_LETTERS && ch != 27) {
                    input_pw.push_back(ch);
                    mvwprintw(win1, 7, 38 + i, "*");
                    wrefresh(win1);
                    ch = getch();
                    i++;
                }

                loginuser->setpw(input_pw);
                move(10, 38);
                curs_set(0);
                mvwprintw(win1, 10, 38, "> Login");
                wrefresh(win1);
                cur = 2;

            } else if (cur == 0) { // id input
                string input_id;
                curs_set(1);

                char ch = key;

                int i = 0;
                while (ch != '\n' && i < MAX_LETTERS && ch != 27) {
                    input_id.push_back(ch);
                    mvwprintw(win1, 4, 38 + i, "%s", &ch);
                    wrefresh(win1);
                    ch = getch();
                    i++;
                }

                loginuser->setid(input_id);
                mvwprintw(win1, 0, 0, "%s", loginuser->getid());
                move(7, 38);
                wrefresh(win1);
                cur = 1;
            }
        }
    }

    curs_set(0);
    startPage(fd1, fd2);
    return;
}

void signupPage(int fd1, int fd2) {
    WINDOW *win1 = newwin(24, 80, 0, 0);
    WINDOW *win2 = newwin(6, 80, 18, 0);

    wbkgd(win1, COLOR_PAIR(1));
    wbkgd(win2, COLOR_PAIR(2));

    mvwprintw(win1, 4, 34, "name: ");
    mvwprintw(win1, 7, 34, "id: ");
    mvwprintw(win1, 10, 34, "pw: ");
    mvwprintw(win1, 13, 38, "Sign up   ");

    wrefresh(win1);
    wrefresh(win2);

    int key = -1;
    int cur = -1;
    userinfo *newuser = (userinfo *)malloc(sizeof(userinfo));

    move(4, 40);
    while (key != 27) { // esc not pressed
        key = getch();

        if (cur == -1) {
            move(4, 40);
            curs_set(1);
            cur = 0;
        }
        if (key == KEY_UP) { // move

            if (cur == 0) { // name;
                continue;
            }
            if (cur == 1) { // id -> name
                cur = 0;
                move(4, 40);
                curs_set(1);

            } else if (cur == 2) { // pw -> id
                cur = 1;
                move(7, 38);
                curs_set(1);
            } else if (cur == 3) { // sign up -> pw
                cur = 2;
                mvwprintw(win1, 13, 38, "Sign up   ");
                wrefresh(win1);
                move(10, 38);
                curs_set(1);
            }

        } else if (key == KEY_DOWN) {

            if (cur == 0) { // name -> id
                cur = 1;
                move(7, 38);
                curs_set(1);
            } else if (cur == 1) { // id -> pw
                cur = 2;
                move(10, 38);
                curs_set(1);
            } else if (cur == 2) { // pw -> sign up
                cur = 3;
                curs_set(0);
                mvwprintw(win1, 13, 38, "> Sign up");
                wrefresh(win1);
            }
        } else {

            if (cur == 3) {
                if (key == 10) { ////////////////////////enter

                    if (newuser->isblank()) {

                        mvwprintw(win2, 1, 2, "isblank()");
                        wrefresh(win2);

                        mvwprintw(win2, 0, 27, "                           ");
                        mvwprintw(win2, 0, 27, "Please fill out all blanks");
                        wrefresh(win2);

                        int anykey = getch();

                        free(newuser);
                        signupPage(fd1, fd2);
                        return;
                    }
                    communication *signupinfo =
                        (communication *)malloc(sizeof(communication));
                    strcpy(signupinfo->input_id, newuser->getid());
                    strcpy(signupinfo->input_name, newuser->getname());
                    strcpy(signupinfo->input_pw, newuser->getpw());
                    signupinfo->classification = signupinfo->signup;

                    write(fd1, signupinfo,
                          sizeof(communication)); // send signup

                    read(fd2, signupinfo, sizeof(communication)); // read result

                    switch (signupinfo->result) {
                    case 0: {
                        // success!
                        free(newuser);
                        loginPage(fd1, fd2);
                        free(signupinfo);
                        return;
                    }
                    case 1: {
                        // id error
                        mvwprintw(win2, 0, 27, "                           ");
                        mvwprintw(win2, 0, 27, "Already exists ID");
                        wrefresh(win2);

                        int anykey = getch();

                        free(newuser);
                        free(signupinfo);
                        signupPage(fd1, fd2);
                        return;
                    }
                    default:
                        free(newuser);
                        free(signupinfo);
                    }
                }
            } else if (cur == 2) { // pw input
                string input_pw;
                curs_set(1);

                char ch = key;

                int i = 0;
                while (ch != '\n' && i < MAX_LETTERS && ch != 27) {
                    input_pw.push_back(ch);
                    mvwprintw(win1, 10, 38 + i, "*");
                    wrefresh(win1);
                    ch = getch();
                    i++;
                }

                newuser->setpw(input_pw);
                move(13, 38);
                curs_set(0);
                mvwprintw(win1, 13, 38, "> Sign up");
                wrefresh(win1);
                cur = 3;

            } else if (cur == 1) { // id input
                string input_id;
                curs_set(1);

                char ch = key;

                int i = 0;
                while (ch != '\n' && i < MAX_LETTERS && ch != 27) {
                    input_id.push_back(ch);
                    mvwprintw(win1, 7, 38 + i, "%s", &ch);
                    wrefresh(win1);
                    ch = getch();
                    i++;
                }

                newuser->setid(input_id);
                move(10, 38);
                wrefresh(win1);
                cur = 2;

            } else if (cur == 0) { // name input
                string input_name;
                curs_set(1);

                char ch = key;

                int i = 0;
                while (ch != '\n' && i < MAX_LETTERS && ch != 27) {
                    input_name.push_back(ch);
                    mvwprintw(win1, 4, 40 + i, "%s", &ch);
                    wrefresh(win1);
                    ch = getch();
                    i++;
                }

                newuser->setname(input_name);
                move(7, 38);
                wrefresh(win1);
                cur = 1;
            }
        }
    }
    curs_set(0);
    startPage(fd1, fd2);
    return;
}
