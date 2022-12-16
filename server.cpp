#include <cstdio>
#include <iostream>
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

using namespace std;

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

void signup(communication *input, int fd1, int fd2) {
    // sign up
    cout << "<" << input->input_id << ">"
         << " try to sign up...";
    fflush(stdout);

    // dup2(fd2, STDOUT_FILENO);

    // id check
    // open directory
    DIR *dp;
    struct dirent *entry;

    int return_stat;
    char *file_name;
    struct stat file_info;

    mode_t file_mode;

    userinfo *newuser = (userinfo *)malloc(sizeof(userinfo));
    newuser->setid(input->input_id);
    newuser->setname(input->input_name);
    newuser->setpw(input->input_pw);

    fflush(stdout);

    char *cwd = (char *)malloc(sizeof(char) * 1024);
    getcwd(cwd, 1024);
    dp = opendir(cwd);
    if (dp != NULL) {

        while ((entry = readdir(dp)) != NULL) {

            if ((return_stat = stat(entry->d_name, &file_info)) == -1) {
                perror("readdir() error!");
                exit(0);
            }

            file_mode = file_info.st_mode;

            if (!S_ISREG(file_mode))
                continue;

            // get extension .dat
            char dnamearray[1024];
            strcpy(dnamearray, entry->d_name);
            char *dname = strtok(dnamearray, ".");
            if (entry->d_name[strlen(dname)] != '.')
                continue;
            dname = strtok(NULL, ".");

            if ((strcmp(dname, "dat") != 0))
                continue;

            // read
            userinfo *user = (userinfo *)malloc(sizeof(userinfo));
            int datfd;
            char datpath[1024] = "./";
            strcat(datpath, entry->d_name);
            ssize_t rSize;

            datfd = open(datpath, O_RDONLY, 0644);
            if (datfd == -1) {
                perror("open() error!");
                exit(-1);
            }

            rSize = read(datfd, (userinfo *)user, sizeof(userinfo));
            if (rSize == -1) {
                perror("read() error!");
                exit(-2);
            }

            if (strcmp(newuser->getid(), user->getid()) ==
                0) { // already exists ID
                input->result = input->id;
                cout << "denied -Already exists ID\n";
                fflush(stdout);

                write(fd2, input,
                      sizeof(communication)); // send id error

                close(datfd);
                free(user);
                closedir(dp);
                free(cwd);
                free(newuser);

                return;
            } else {
                close(datfd);
                free(user);
            }
        }
    }
    closedir(dp);
    free(cwd);

    // sign up
    // write new user's info
    int fd = 0;

    // make path of new user's file
    char pathname[1024] = "./";
    char pdat[] = ".dat";
    strcat(pathname, newuser->getid());
    strcat(pathname, pdat);

    ssize_t wsize = 0;

    // open
    fd = open(pathname, O_CREAT | O_TRUNC | O_RDWR, 0644);

    if (fd == -1) {
        perror("open() error!");
        exit(-1);
    }

    // write
    wsize = write(fd, (userinfo *)newuser, sizeof(userinfo));
    if (wsize == -1) {
        perror("write() error!");
        exit(-2);
    }
    close(fd);
    free(newuser);

    // send success
    input->result = input->success;
    cout << "success" << endl;
    fflush(stdout);

    write(fd2, input,
          sizeof(communication)); // send success
    return;
}

void login(communication *input, int fd1, int fd2) {
    // login
    cout << "<" << input->input_id << ">"
         << " try to login...";
    fflush(stdout);

    // open directory
    DIR *dp;
    struct dirent *entry;

    int return_stat;
    char *file_name;
    struct stat file_info;

    mode_t file_mode;

    userinfo *loginuser = (userinfo *)malloc(sizeof(userinfo));
    loginuser->setid(input->input_id);
    loginuser->setname(input->input_name);
    loginuser->setpw(input->input_pw);

    char *cwd = (char *)malloc(sizeof(char) * 1024);
    getcwd(cwd, 1024);

    dp = opendir(cwd);
    if (dp != NULL) {
        // read current directory
        while ((entry = readdir(dp)) != NULL) {
            if ((return_stat = stat(entry->d_name, &file_info)) == -1) {
                perror("readdir() error!");
                exit(0);
            }

            file_mode = file_info.st_mode;

            if (!S_ISREG(file_mode))
                continue;

            // get extension .dat
            char dnamearray[1024];
            strcpy(dnamearray, entry->d_name);
            char *dname = strtok(dnamearray, ".");
            if (entry->d_name[strlen(dname)] != '.')
                continue;
            dname = strtok(NULL, ".");

            if ((strcmp(dname, "dat") != 0))
                continue;

            // read .dat file
            userinfo *user = (userinfo *)malloc(sizeof(userinfo));
            int datfd; // file discriptor of .dat file
            char datpath[1024] = "./";
            strcat(datpath, entry->d_name);
            ssize_t rSize;

            datfd = open(datpath, O_RDONLY, 0644);
            if (datfd == -1) {
                perror("open() error!");
                exit(-1);
            }

            rSize = read(datfd, (userinfo *)user, sizeof(userinfo));
            if (rSize == -1) {
                perror("read() error!");
                exit(-2);
            }

            if (strcmp(loginuser->getid(), user->getid()) == 0) {
                // login success
                if (strcmp(loginuser->getpw(), user->getpw()) == 0) {

                    // input->input_id = user->getname();
                    strcpy(input->input_name, user->getname());

                    close(datfd);
                    free(user);

                    closedir(dp);
                    free(cwd);

                    free(loginuser);

                    // send success
                    input->result = input->success;
                    cout << "success\n";
                    fflush(stdout);

                    write(fd2, input, sizeof(communication));
                    return;

                } else { // pw is wrong

                    close(datfd);
                    free(user);

                    closedir(dp);
                    free(cwd);

                    free(loginuser);

                    // send pw error
                    input->result = input->pw;
                    cout << "denied -Incorrect Password" << endl;
                    fflush(stdout);

                    write(fd2, input,
                          sizeof(communication)); // send pw error
                    return;
                }
            } else {
                close(datfd);
                free(user);
            }
        }
    }
    closedir(dp);
    free(cwd);

    // id not exist
    free(loginuser);
    input->result = input->id;
    cout << "denied -ID doesn't exist" << endl;
    fflush(stdout);

    write(fd2, input,
          sizeof(communication)); // send id error
    return;
}

void message(communication *input, int fd1, int fd2) {
    // message
    // login
    cout << "<" << input->input_id << ">"
         << " loading user list...";
    fflush(stdout);

    for (int i = 0; i < MAX_USERS; i++) {
        // input->user_list[i] = "\0";
        strcat(input->user_list[i], "\0");
    }

    // find "id.dat" file

    // oepn directory
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

        char *userlist[MAX_USERS];
        for (int i = 0; i < MAX_USERS; i++) {
            strcat(userlist[i], "\0");
        }

        while ((entry = readdir(dp)) != NULL) {
            if ((return_stat = stat(entry->d_name, &file_info)) == -1) {
                perror("readdir() error!");
                exit(0);
            }

            file_mode = file_info.st_mode;

            if (!S_ISREG(file_mode))
                continue;

            // get extension .dat
            char dnamearray[1024];
            strcpy(dnamearray, entry->d_name);

            if ((strcmp(dnamearray, input->input_id)) != 0)
                continue;
            char *dname = strtok(dnamearray, ".");
            if (entry->d_name[strlen(dname)] != '.')
                continue;
            dname = strtok(NULL, ".");

            if ((strcmp(dname, "dat") != 0))
                continue;

            // read
            int datfd;
            char datpath[1024] = "./";
            strcat(datpath, entry->d_name);
            ssize_t rSize;

            datfd = open(datpath, O_RDONLY, 0644);
            if (datfd == -1) {
                perror("open() error!");
                exit(-1);
            }

            lseek(datfd, sizeof(userinfo), SEEK_SET); ////// user list test
            rSize = read(datfd, (char **)userlist, sizeof(char *));
            if (rSize == -1) {
                perror("read() error!");
                exit(-2);
            }

            for (int i = 0; i < MAX_USERS; i++) {
                input->user_list[i] = userlist[i];
                cout << userlist[i] << endl;
                if (userlist[i] = NULL)
                    break;
            }
            close(datfd);
            closedir(dp);
            free(cwd);
        }
    }

    write(fd2, input, sizeof(communication));

    communication opponent;
    read(fd1, &opponent, sizeof(communication));
    // find opponent

    cout << "end" << endl;

    cout << "<" << input->input_id << ">"
         << " enter message window...";
    fflush(stdout);

    do {
        // open directory
        DIR *dp2;
        struct dirent *entry2;

        int return_stat2;
        char *file_name2;
        struct stat file_info2;

        mode_t file_mode2;

        char *cwd2 = (char *)malloc(sizeof(char) * 1024);
        getcwd(cwd, 1024);

        dp = opendir(cwd);
        if (dp != NULL) {
            // read current directory
            while ((entry2 = readdir(dp)) != NULL) {

                if ((return_stat2 = stat(entry2->d_name, &file_info2)) == -1) {
                    perror("readdir() error!");
                    exit(0);
                }

                file_mode2 = file_info2.st_mode;

                if (!S_ISREG(file_mode2))
                    continue;

                // get extension .dat
                char dnamearray2[1024];
                strcpy(dnamearray2, entry2->d_name);

                char *dname2 = strtok(dnamearray2, ".");
                if (entry2->d_name[strlen(dname2)] != '.')
                    continue;

                dname2 = strtok(NULL, ".");

                if ((strcmp(dname2, "dat") != 0))
                    continue;

                // read .dat file
                userinfo *user2 = (userinfo *)malloc(sizeof(userinfo));
                int datfd2; // file discriptor of .dat file
                char datpath2[1024] = "./";
                strcat(datpath2, entry2->d_name);
                ssize_t rSize2;

                datfd2 = open(datpath2, O_RDONLY, 0644);
                if (datfd2 == -1) {
                    perror("open() error!");
                    exit(-1);
                }

                rSize2 = read(datfd2, (userinfo *)user2, sizeof(userinfo));
                if (rSize2 == -1) {
                    perror("read() error!");
                    exit(-2);
                }

                if (strcmp(opponent.input_id, user2->getid()) == 0) {
                    if (strcmp(opponent.input_id, input->input_id) == 0) {
                        opponent.result = opponent.pw;
                        cout << "denied -Input error" << endl;

                        write(fd2, &opponent, sizeof(communication));

                        close(datfd2);
                        free(user2);
                    }
                    // find opponent
                    opponent.result = opponent.success;
                    cout << "success" << endl;
                    fflush(stdout);

                    write(fd2, &opponent,
                          sizeof(communication)); // send success

                    close(datfd2);
                    free(user2);
                    closedir(dp2);
                    free(cwd2);

                    return;

                } else {
                    close(datfd2);
                    free(user2);
                }
            }
        }
        closedir(dp2);
        free(cwd2);

        // id not exist
        opponent.result = opponent.id;
        cout << "denied -ID doesn't exist" << endl;
        fflush(stdout);

        write(fd2, &opponent,
              sizeof(communication)); // send id error

    } while (true);
    return;
}

int main() {
    int fd1, fd2;

    mkfifo(PIPE1, PERMS); // make named pipe S2C

    if ((fd1 = open(PIPE1, O_RDONLY)) == -1) { // open C2S
        cout << "open() C2S error" << endl;
        exit(0);
    }

    if ((fd2 = open(PIPE2, O_WRONLY)) == -1) { // open S2C
        cout << "open() S2C error" << endl;
        exit(0);
    }

    //
    do {
        communication *input = (communication *)malloc(sizeof(communication));
        read(fd1, input, sizeof(communication));
        switch (input->classification) {
        case 0:
            // signup
            signup(input, fd1, fd2);
            break;
        case 1:
            // login
            login(input, fd1, fd2);
            break;
        case 2:
            // message
            message(input, fd1, fd2);
            break;
        }

        free(input);
    } while (true);

    close(fd1);
    return 0;
}
