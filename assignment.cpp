#include <iostream>
#include <ncurses.h>
#include <string.h>

#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_LETTERS 10

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

void mainPage();
void loginPage();
void signupPage();
void mainScreenPage(userinfo u);

int main(int argc, char const *argv[]) {
    initscr();

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
    mainPage();

    endwin();
    return 0;
}

void mainPage() {
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
            if (cur == 0) {
                loginPage();
            } else if (cur == 1) {
                signupPage();
            }
        }

        wrefresh(win1);
    }
}

void mainScreenPage(userinfo u) {
    WINDOW *win1 = newwin(24, 80, 0, 0);
    WINDOW *win2 = newwin(6, 80, 18, 0);

    wbkgd(win1, COLOR_PAIR(1));
    wbkgd(win2, COLOR_PAIR(2));

    int message_num = 4; // number of temporary messages

    // print message
    for (int i = 0; i < message_num; i++) {
        char num[1];
        num[0] = (char)(i + 49);
        mvwprintw(win1, i, 2, "Message");
        mvwprintw(win1, i, 9, num);
    }

    // print user infomation
    mvwprintw(win2, 0, 0, "ID: ");
    mvwprintw(win2, 0, 4, u.getid());
    mvwprintw(win2, 1, 0, "Name: ");
    mvwprintw(win2, 1, 6, u.getname());

    // print logout button
    mvwprintw(win2, 0, 72, "  Logout");

    wrefresh(win1);
    wrefresh(win2);

    int key;
    int cur = 0;

    mvwprintw(win1, 0, 0, ">");
    wrefresh(win1);

    while (key != 27) {
        key = getch();

        if (key == KEY_UP) {
            // print reset
            for (int i = 0; i < message_num; i++) {
                mvwprintw(win1, i, 0, " ");
            }
            mvwprintw(win2, 0, 72, "  Logout");
            if (cur > 0 && cur < message_num + 1)
                cur--;

            for (int i = 0; i < message_num; i++) {
                if (i == cur) {
                    mvwprintw(win1, i, 0, ">");
                }
                if (cur == message_num - 1) {
                    mvwprintw(win2, 0, 72, "  Logout");
                }
            }

            wrefresh(win1);
            wrefresh(win2);

        } else if (key == KEY_DOWN) {
            // print reset
            for (int i = 0; i < message_num; i++) {
                mvwprintw(win1, i, 0, " ");
            }
            mvwprintw(win2, 0, 72, "  Logout");
            if (cur > -1 && cur < message_num)
                cur++;

            if (cur == message_num) {
                mvwprintw(win2, 0, 72, "> Logout");
            } else {
                for (int i = 1; i < message_num; i++) {
                    if (i == cur) {
                        mvwprintw(win1, i, 0, ">");
                    }
                }
            }

            wrefresh(win1);
            wrefresh(win2);
        } else {
            if (key == 10) {
                if (cur == message_num) {
                    break;
                }
            }
        }
    }
    mainPage();
}

void loginPage() {
    WINDOW *win1 = newwin(24, 80, 0, 0);
    WINDOW *win2 = newwin(6, 80, 18, 0);

    wbkgd(win1, COLOR_PAIR(1));
    wbkgd(win2, COLOR_PAIR(2));

    mvwprintw(win1, 4, 34, "id: ");      // 38
    mvwprintw(win1, 7, 34, "pw: ");      // 38
    mvwprintw(win1, 10, 38, "Login   "); //

    wrefresh(win1);
    wrefresh(win2);

    int key;
    int cur = -1;
    userinfo *loginuser = (userinfo *)malloc(sizeof(userinfo));
    char loginname[6] = "login";
    loginuser->setname(loginname);

    move(4, 40);
    while (key != 27) {
        key = getch();
        if (cur == -1) {
            move(4, 38);
            curs_set(1);
            cur = 0;
        }
        if (key == KEY_UP) {
            if (cur == 0) {
                continue;
            }
            if (cur == 1) {
                cur = 0;
                move(4, 38);
                curs_set(1);
            } else if (cur == 2) {
                cur = 1;
                mvwprintw(win1, 10, 38, "Login   ");
                move(7, 38);
                curs_set(1);
            }

        } else if (key == KEY_DOWN) {
            if (cur == 0) {
                cur = 1;
                move(7, 38);
                curs_set(1);

            } else if (cur == 1) {
                cur = 2;
                curs_set(0);
                mvwprintw(win1, 10, 38, "> Login");
            }

        } else {
            if (cur == 2) {
                if (key == 10) {
                    if (loginuser->isblank()) {
                        mvwprintw(win2, 0, 27, "                           ");
                        mvwprintw(win2, 0, 27, "Please fill out all blanks");
                        wrefresh(win2);
                        continue;
                    }

                    // open directory
                    DIR *dp;
                    struct dirent *entry;

                    int return_stat;
                    char *file_name;
                    struct stat file_info;

                    mode_t file_mode;

                    char *cwd = (char *)malloc(sizeof(char) * 1024);
                    getcwd(cwd, 1024);

                    dp = opendir(cwd);
                    if (dp != NULL) {
                        int i = 0;
                        while (entry = readdir(dp)) {
                            i++;
                            if ((return_stat =
                                     stat(entry->d_name, &file_info)) == -1) {
                                perror("readdir() error!");
                                exit(0);
                            }

                            file_mode = file_info.st_mode;

                            if (!S_ISREG(file_mode))
                                continue;

                            char dnamearray[1024];
                            strcpy(dnamearray, entry->d_name);
                            char *dname = strtok(dnamearray, ".");
                            if (entry->d_name[strlen(dname)] != '.')
                                continue;
                            dname = strtok(NULL, ".");

                            if ((strcmp(dname, "dat") != 0))
                                continue;

                            // read()
                            userinfo *user =
                                (userinfo *)malloc(sizeof(userinfo));
                            int datfd;
                            char datpath[1024] = "./";
                            strcat(datpath, entry->d_name);
                            ssize_t rSize;

                            datfd = open(datpath, O_RDONLY, 0644);
                            if (datfd == -1) {
                                perror("open() error!");
                                exit(-1);
                            }

                            cout << endl;

                            rSize =
                                read(datfd, (userinfo *)user, sizeof(userinfo));
                            if (rSize == -1) {
                                perror("read() error!");
                                exit(-2);
                            }

                            if (strcmp(loginuser->getid(), user->getid()) ==
                                0) {
                                // login success
                                if (strcmp(loginuser->getpw(), user->getpw()) ==
                                    0) {
                                    mvwprintw(win2, 2, 0, "success!");
                                    wrefresh(win2);

                                    userinfo logined =
                                        userinfo(user->getname(), user->getid(),
                                                 user->getpw());

                                    close(datfd);
                                    free(user);
                                    closedir(dp);
                                    free(cwd);
                                    free(loginuser);
                                    // loginPage();
                                    mainScreenPage(logined);

                                } else { // pw is wrong
                                    mvwprintw(win2, 2, 0, "pw is wrong");
                                    wrefresh(win2);
                                    sleep(3);
                                    close(datfd);
                                    free(user);
                                    closedir(dp);
                                    free(cwd);
                                    free(loginuser);

                                    loginPage();
                                }
                            }

                            close(datfd);
                            free(user);
                        }
                    }
                    closedir(dp);
                    free(cwd);

                    mvwprintw(win2, 0, 27, "                           ");
                    mvwprintw(win2, 0, 34, "ID doesn't exist");
                    wrefresh(win2);
                    int key = getch();
                    free(loginuser);
                    loginPage();
                }

            } else if (cur == 1) { // pw
                string input_pw;
                curs_set(1);

                char ch = key;

                int i = 0;
                while (ch != '\n' && i < MAX_LETTERS) {
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

            } else if (cur == 0) { // id
                string input_id;
                curs_set(1);

                char ch = key;

                int i = 0;
                while (ch != '\n' && i < MAX_LETTERS) {
                    input_id.push_back(ch);
                    mvwprintw(win1, 4, 38 + i, &ch);
                    wrefresh(win1);
                    ch = getch();
                    i++;
                }

                loginuser->setid(input_id);
                mvwprintw(win1, 0, 0, loginuser->getid());
                move(7, 38);
                wrefresh(win1);
                cur = 1;
            }
        }
        wrefresh(win1);
    }
    mainPage();
}

void signupPage() {
    WINDOW *win1 = newwin(24, 80, 0, 0);
    WINDOW *win2 = newwin(6, 80, 18, 0);

    wbkgd(win1, COLOR_PAIR(1));
    wbkgd(win2, COLOR_PAIR(2));

    mvwprintw(win1, 4, 34, "name: "); // 40
    mvwprintw(win1, 7, 34, "id: ");   // 38
    mvwprintw(win1, 10, 34, "pw: ");  // 38
    mvwprintw(win1, 13, 38, "Sign up   ");

    wrefresh(win1);
    wrefresh(win2);

    int key;
    int cur = -1;
    userinfo *newuser = (userinfo *)malloc(sizeof(userinfo));

    move(4, 40);
    while (key != 27) {
        key = getch();
        if (cur == -1) {
            move(4, 40);
            curs_set(1);
            cur = 0;
        }
        if (key == KEY_UP) {
            if (cur == 0) { // name;
                continue;
            }
            if (cur == 1) {
                cur = 0;
                move(4, 40);
                curs_set(1);

            } else if (cur == 2) {
                cur = 1;
                move(7, 38);
                curs_set(1);
            } else if (cur == 3) {
                cur = 2;
                mvwprintw(win1, 13, 38, "Sign up   ");
                wrefresh(win1);
                move(10, 38);
                curs_set(1);
            }

        } else if (key == KEY_DOWN) {
            if (cur == 0) {
                cur = 1;
                move(7, 38);
                curs_set(1);
            } else if (cur == 1) {
                cur = 2;
                move(10, 38);
                curs_set(1);
            } else if (cur == 2) {
                cur = 3;
                curs_set(0);
                mvwprintw(win1, 13, 38, "> Sign up");
                wrefresh(win1);
            }
        } else {
            if (cur == 3) {
                if (key == 10) {
                    if (newuser->isblank()) {
                        mvwprintw(win2, 0, 27, "                           ");
                        mvwprintw(win2, 0, 27, "Please fill out all blanks");
                        wrefresh(win2);
                        continue;
                    }

                    // write new user's info
                    int fd = 0;

                    char pathname[1024] = "./";
                    char pdat[] = ".dat";
                    strcat(pathname, newuser->getid());
                    strcat(pathname, pdat);

                    ssize_t wsize = 0;

                    fd = open(pathname, O_CREAT | O_TRUNC | O_RDWR, 0644);

                    if (fd == -1) {
                        perror("open() error!");
                        exit(-1);
                    }

                    wsize = write(fd, (userinfo *)newuser, sizeof(userinfo));
                    if (wsize == -1) {
                        perror("write() error!");
                        exit(-2);
                    }
                    close(fd);
                    free(newuser);

                    loginPage();
                }

            } else if (cur == 2) { // pw
                string input_pw;
                curs_set(1);

                char ch = key;

                int i = 0;
                while (ch != '\n' && i < MAX_LETTERS) {
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

            } else if (cur == 1) { // id
                string input_id;
                curs_set(1);

                char ch = key;

                int i = 0;
                while (ch != '\n' && i < MAX_LETTERS) {
                    input_id.push_back(ch);
                    mvwprintw(win1, 7, 38 + i, &ch);
                    wrefresh(win1);
                    ch = getch();
                    i++;
                }

                newuser->setid(input_id);
                move(10, 38);
                wrefresh(win1);
                cur = 2;

            } else if (cur == 0) {
                string input_name;
                curs_set(1);

                char ch = key;

                int i = 0;
                while (ch != '\n' && i < MAX_LETTERS) {
                    input_name.push_back(ch);
                    mvwprintw(win1, 4, 40 + i, &ch);
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
    mainPage();
}
