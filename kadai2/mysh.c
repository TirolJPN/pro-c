/*----------------------------------------------------------------------------
 *  簡易版シェル
 *--------------------------------------------------------------------------*/

/*
 *  インクルードファイル
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
// 追加したインクルードファイル
#include <dirent.h>

/*
 *  定数の定義
 */

#define BUFLEN    1024     /* コマンド用のバッファの大きさ */
#define MAXARGNUM  256     /* 最大の引数の数 */
#define HISTORY_NUM 32     /* 最大コマンド履歴数 */


/*
 *  ローカルプロトタイプ宣言
 */

int parse(char [], char *[]);
void execute_command(char *[], int);

/*----------------------------------------------------------------------------
 *
 *  関数名   : main
 *
 *  作業内容 : シェルのプロンプトを実現する
 *
 *  引数     :
 *
 *  返り値   :
 *
 *  注意     :
 *
 *--------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    char command_buffer[BUFLEN]; /* コマンド用のバッファ */
    char *args[MAXARGNUM];       /* 引数へのポインタの配列 */
    int command_status;          /* コマンドの状態を表す
                                    command_status = 0 : フォアグラウンドで実行
                                    command_status = 1 : バックグラウンドで実行
                                    command_status = 2 : シェルの終了
                                    command_status = 3 : 何もしない */

    char prompt[]= "Command";
    /*
     *  variables for pushd, dirs and popd command
     */
    typedef struct t_directory_stack{
      char directory[128];
      struct t_directory_stack *next;
    }directory_stack;

    directory_stack * d_s_head = NULL;
    directory_stack * d_s_new = NULL;
    directory_stack * d_s_tmp = NULL;
    char corrent_working_directory[128];

    /*
     * for script function
     */
    FILE * script_file;

    /*
     *  variables for history command
     */
    typedef struct t_history{
      char command[128];
      int order;
      int index;
      struct t_history *next;
    }history;
    history * h_front = NULL;
    history * h_new = NULL;
    history * h_tmp = NULL;
    history * h_back = NULL;
    char history_buf[50];
    int history_order = 1;
    int t=0;

    /*
     *  variables for !string command
     */
    char tmp_string[128];
    int tmp_string_flag = 0;

    /*
     *  variables for !command
     */
    char tmp_wild_cwd[128];
    char tmp_wild_directory_string[128];
    char tmp_wild_file_string[128];
    DIR* tmp_wild_directory;
    int tmp_wild_flag = 0;

    /*
     *  variables for alias and unalias command
     */
     typedef struct t_alias{
       char original_command[10];
       char alias_command[10];
       struct t_alias * next;
     }alias;

     alias * a_back = NULL;
     alias * a_front = NULL;
     alias * a_tmp = NULL;
     alias * a_del = NULL;
     alias * a_new = NULL;
     int a_found_flag = 0;
     char alias_buffer[50];
     char alias_command_buffer[BUFLEN];

     char *search[MAXARGNUM];

    /*
     *  無限にループする
     */

    for(;;) {

        /*
         *  プロンプトを表示する
         */
        printf("%s : ",prompt);


        /*
         *  標準入力から１行を command_buffer へ読み込む
         *  入力が何もなければ改行を出力してプロンプト表示へ戻る
         */

        if(fgets(command_buffer, BUFLEN, stdin) == NULL) {
            printf("\n");
            continue;
        }

        /*
         *  入力されたバッファ内のコマンドを解析する
         *
         *  返り値はコマンドの状態
         */
        command_status = parse(command_buffer, args);

        /*
         * for wild card
         */
        if(args[1] != NULL && strncmp(args[1], "*", 1) == 0){
          if(args[2] != NULL){ //他に引数があればそれを保存
            sprintf(tmp_wild_directory_string,"%s",args[2]);
            tmp_wild_flag = 1;
          }
          getcwd(corrent_working_directory,128);
          tmp_wild_directory = opendir(corrent_working_directory);
          if (tmp_wild_directory != NULL){
            sprintf(command_buffer, "%s", args[0]);
            struct dirent* dent;
            for(dent = readdir(tmp_wild_directory); dent != NULL; dent = readdir(tmp_wild_directory)){
                if (strncmp(dent -> d_name, ".", 1) != 0){
                  sprintf(tmp_wild_file_string, " %s", dent->d_name);
                  strcat(command_buffer, tmp_wild_file_string);
                }
            }
            closedir(tmp_wild_directory);
            if(tmp_wild_flag == 1){
              sprintf(command_buffer, "%s %s",command_buffer, tmp_wild_directory_string);
            }
            tmp_wild_flag = 0;
            strcat(command_buffer, "\n");
            command_status = parse(command_buffer, args);
          }
        }



        if((strncmp(args[0], "!",1)) == 0){    // !! command
          if(args[1] != NULL){
            printf("Invalid argument\n");
          }else{
            if(strncmp(args[0], "!!", 2) == 0 && strlen(args[0]) == 2){
              if(h_back == NULL){
                printf("Command not found.\n");
              }else{
                strcpy(command_buffer, h_front -> command);
                command_status = parse(command_buffer, args);
              }
            }else{                              // !string command
              strcpy(tmp_string, args[0] + 1);
              h_tmp = h_back;
              while(1){
                printf("strcmp:%d tmp_p_string:%s\n",strncmp(tmp_string, h_tmp -> command, strlen(tmp_string)), tmp_string);
                if(strncmp(tmp_string, h_tmp -> command, strlen(tmp_string)) == 0){
                  sprintf(command_buffer,"%s", h_tmp -> command);
                  tmp_string_flag = 1;
                }
                printf("%s\n", h_tmp -> command);
                h_tmp = h_tmp -> next;
                if(h_tmp == NULL) break;
              }
              if(tmp_string_flag == 0){
                printf("Command not found.\n");
              }else{
                command_status = parse(command_buffer, args);
              }
              tmp_string_flag = 0;
            }
          }
        }
        /*
         *  終了コマンドならばプログラムを終了
         *  引数が何もなければプロンプト表示へ戻る
         */

        if(command_status == 2) {
            printf("done.\n");
            exit(EXIT_SUCCESS);
        } else if(command_status == 3) {
            continue;
        }

        /*
         *  コマンドを実行する
         */
        /*
         *  for history command
         */
        if(h_back == NULL){
          if((h_front = (history *)malloc(sizeof(history))) == NULL){
            printf("malloc error.\n");
          }else{
            h_back = h_front;
            h_front -> next = NULL;
            h_front -> order = history_order++;
            h_front -> index = 0;
            sprintf(h_front -> command,"%s ",args[0]);
            for(int i = 1;args[i] != NULL; i++){
              sprintf(history_buf, "%s ", args[i]);
              strcat(h_front -> command,history_buf);
            }
          }
        }else{
          if(h_back -> index == 31){
            h_tmp = h_back -> next;
            free(h_back);
            h_back = h_tmp;
          }
          if((h_new = (history *)malloc(sizeof(history))) == NULL){
            printf("malloc error.\n");
          }else{
           //更新
            h_tmp = h_back;
            while(1){
             h_tmp -> index = h_tmp -> index + 1;
              h_tmp = h_tmp -> next;
              if(h_tmp == NULL){
               break;
              }
            }
            h_new -> next = NULL;
            h_new -> order = history_order++;
            h_new -> index = 0;
            sprintf(h_new -> command,"%s ",args[0]);
            for(int i = 1;args[i] != NULL; i++){
              sprintf(history_buf, "%s ", args[i]);
              strcat(h_new -> command,history_buf);
            }
            h_front -> next = h_new;
            h_front = h_new;
            h_tmp = h_back;
          }
        }

        /*
         *  for alias command
         */
        if(a_back != NULL){
          a_tmp = a_back;
          while(1){
            if(strcmp(args[0], a_tmp -> alias_command) == 0){
              a_found_flag = 1;
              break;
            }
            a_tmp = a_tmp -> next;
            if(a_tmp == NULL){
              break;
            }
          }
          if (a_found_flag == 1){
            /*  args[0]をそのまま書き換えると，args[1]も書き換わる可能性がある．
             *  例) % alias md mkdir
             *  % md bbb と入力するとmkdir irが実行される．
             *  そこでバッファ文字列alias_command_bufferを用意し，そこに変換したコマンド文字列を格納する．
             */
            sprintf(alias_command_buffer, "%s ", a_tmp -> original_command);
            if(args[1] != NULL){
              for(int i = 1; args[i] != NULL; i++){
                sprintf(alias_buffer, "%s ", args[i]);
                strcat(alias_command_buffer, alias_buffer);
              }
            }
            printf("%s\n",alias_command_buffer);
            strcpy(command_buffer, alias_command_buffer);
            command_status = parse(command_buffer, args);
            a_found_flag = 0;
          }
        }

        /*
         * propmt command
         */
        if(strcmp(args[0], "prompt") == 0){
          if(args[1] == NULL){ //フォルトの文字列 "Command : "に変更
            strcpy(prompt,"Command");
            continue;
          }else if(args[2] != NULL){
            printf("Invalid argument.\n");
          }else{
            strcpy(prompt, args[1]);
            continue;
          }
        }

        /*
         * for script function
         */
        if(strcmp(args[0], argv[0]) == 0){
          if(args[1] == NULL){
            printf("Invalid argument.\n");
          }else if(strcmp(args[1], "<") != 0){
            printf("Invalid argument.\n");
          }else if(args[2] == NULL){
            printf("Invalid argument.\n");
          }else{
            if ((script_file = fopen(args[2], "r")) == NULL) {
              fprintf(stderr, "filure to open %s.\n", args[2]);
              exit(EXIT_FAILURE);
            }
            while ( fgets(command_buffer, BUFLEN, script_file) != NULL ) {
              command_status = parse(command_buffer, args);
              execute_command(args, command_status);
            }
          }
        }

        /*
         * alias command
         */
        if(strcmp(args[0], "alias") == 0){
          if(args[1] == NULL){  // 引数が与えられていなかったら
            // 一覧表示
            a_tmp = a_back;
            while(a_tmp != NULL){
              printf("%10s %10s\n", a_tmp -> alias_command, a_tmp -> original_command);
              a_tmp = a_tmp -> next;
            }
          }else if(args[2] == NULL || args[3] != NULL){
            printf("Invalid argument.\n");
          }else {
            if(a_back == NULL){
              if((a_new = (alias *)malloc(sizeof(alias))) == NULL ){
                printf("malloc error.\n");
              }
              a_back = a_new;
              a_front = a_new;
              strcpy(a_back -> alias_command, args[1]);
              strcpy(a_back -> original_command, args[2]);
              a_new -> next = NULL;
            }else{
              if((a_new = (alias *)malloc(sizeof(alias))) == NULL){
                printf("malloc error.\n");
              }
              a_front -> next = a_new;
              strcpy(a_new -> alias_command, args[1]);
              strcpy(a_new -> original_command, args[2]);
              a_new -> next = NULL;
              a_front = a_new;
            }
          }
          continue;
        }

        /*
         *  unalias command
         */
        if((strcmp(args[0], "unalias")) == 0){
          printf("back:%s front:%s\n", a_back -> alias_command, a_front -> alias_command);
          if((args[1] == NULL) || (args[2] != NULL)){
            printf("Invalid argument.\n");
          }else{
            if(strcmp(a_back -> alias_command, args[1]) == 0){
              a_tmp = a_back ->next;
              free(a_back);
              a_back = a_tmp;
            }else{
              a_del = a_back;
              while(1){
                if(strcmp((a_del -> next) -> alias_command, args[1]) == 0){
                  a_found_flag = 1;
                  break;
                }
                a_del = a_del -> next;
                if(a_del -> next == NULL) break;
              }
              if(a_found_flag == 1){
                a_tmp = a_del -> next;
                a_del -> next = a_tmp -> next;
                free(a_tmp);
              }else{
                printf("not found such command.\n");
              }
            }
          }
          a_found_flag = 0;
          continue;
        }

        /*
         * cd command
         */

        if(strcmp(args[0], "cd")==0){
          if(args[1] == NULL){
            if(chdir(getenv("HOME")) == -1){
              perror("cd");
            }
          }else{
            if(chdir(args[1]) == -1){
              perror("cd");
            }
            continue;
          }
        }

        /*
         * pushd command
         */
        if(strcmp(args[0], "pushd") == 0){
          if(args[1] != NULL){
            printf("Invalid argument.\n");
          }else{
            getcwd(corrent_working_directory, 128);
            if(d_s_head == NULL){
              if((d_s_head = (directory_stack *)malloc(sizeof(directory_stack))) == NULL){
                printf("malloc error.\n");
              }
              d_s_head -> next = NULL;
              strcpy(d_s_head -> directory, corrent_working_directory);
            }else{
              if((d_s_new = (directory_stack *)malloc(sizeof(directory_stack))) == NULL){
                printf("malloc error.\n");
              }
              d_s_new -> next = d_s_head;
              strcpy(d_s_new -> directory, corrent_working_directory);
              d_s_head = d_s_new;
            }
            continue;
          }
        }
        /*
         *  dirs command
         */
        if(strcmp(args[0], "dirs") == 0){
          if(args[1] != NULL){
            printf("Invalid argument.\n");
          }else{
            d_s_tmp = d_s_head;
            while(d_s_tmp != NULL){
              printf("%s\n", d_s_tmp -> directory);
              d_s_tmp = d_s_tmp -> next;
            }
            continue;
          }
        }

        /*
         * popd command
         */
        if(strcmp(args[0], "popd") == 0){
          if(args[1] != NULL){
            printf("Invalid argument.\n");
          }else{
            if(chdir(d_s_head -> directory) == -1){
              perror("cd");
            }
            if(d_s_head == NULL){
              printf("no directory in directory_stack\n");
            }else{
              d_s_tmp = d_s_head -> next;
              free(d_s_head);
              d_s_head = d_s_tmp;
              continue;
            }
          }
        }

        /*
         *  history command
         */
        if(strcmp(args[0] , "history") == 0){
          if(args[1] != NULL){
            printf("Invalid argument.\n");
          }else{
            h_tmp = h_back;
            while(1){
              if(h_tmp == NULL){
                break;
              }
              printf("%3d : %s\n", h_tmp -> order, h_tmp -> command);
              h_tmp = h_tmp -> next;
              continue;
            }
          }
        }

        /*
         * search command
         */
        if(strcmp(args[0], "search") == 0){
          if(args[1] == NULL){
            printf("Invalid argument.\n");
          }else{
            if(args[2] == NULL){
              getcwd(corrent_working_directory,128);
              tmp_wild_directory = opendir(corrent_working_directory);
            }else{
              tmp_wild_directory = opendir(args[2]);
            }
            if (tmp_wild_directory != NULL){
              struct dirent* dent;
              for(dent = readdir(tmp_wild_directory); dent != NULL; dent = readdir(tmp_wild_directory)){
                  if (strncmp(dent -> d_name, ".", 1) != 0){
                    if(strstr(dent -> d_name, args[1]) != NULL){
                      printf("%s\n", dent -> d_name);
                    }
                  }
              }
          }
        }
      }

    execute_command(args, command_status);
    }


    return 0;
}

/*----------------------------------------------------------------------------
 *
 *  関数名   : parse
 *
 *  作業内容 : バッファ内のコマンドと引数を解析する
 *
 *  引数     :
 *
 *  返り値   : コマンドの状態を表す :
 *                0 : フォアグラウンドで実行
 *                1 : バックグラウンドで実行
 *                2 : シェルの終了
 *                3 : 何もしない
 *
 *  注意     :
 *
 *--------------------------------------------------------------------------*/

int parse(char buffer[],        /* バッファ */
          char *args[])         /* 引数へのポインタ配列 */
{
    int arg_index;   /* 引数用のインデックス */
    int status;   /* コマンドの状態を表す */

    /*
     *  変数の初期化
     */

    arg_index = 0;
    status = 0;

    /*
     *  バッファ内の最後にある改行をヌル文字へ変更
     */

    *(buffer + (strlen(buffer) - 1)) = '\0';

    /*
     *  バッファが終了を表すコマンド（"exit"）ならば
     *  コマンドの状態を表す返り値を 2 に設定してリターンする
     */

    if(strcmp(buffer, "exit") == 0) {

        status = 2;
        return status;
    }

    /*
     *  バッファ内の文字がなくなるまで繰り返す
     *  （ヌル文字が出てくるまで繰り返す）
     */

    while(*buffer != '\0') {

        /*
         *  空白類（空白とタブ）をヌル文字に置き換える
         *  これによってバッファ内の各引数が分割される
         */

        while(*buffer == ' ' || *buffer == '\t') {
            *(buffer++) = '\0';
        }

        /*
         * 空白の後が終端文字であればループを抜ける
         */

        if(*buffer == '\0') {
            break;
        }

        /*
         *  空白部分は読み飛ばされたはず
         *  buffer は現在は arg_index + 1 個めの引数の先頭を指している
         *
         *  引数の先頭へのポインタを引数へのポインタ配列に格納する
         */

        args[arg_index] = buffer;
        ++arg_index;

        /*
         *  引数部分を読み飛ばす
         *  （ヌル文字でも空白類でもない場合に読み進める）
         */

        while((*buffer != '\0') && (*buffer != ' ') && (*buffer != '\t')) {
            ++buffer;
        }
    }

    /*
     *  最後の引数の次にはヌルへのポインタを格納する
     */

    args[arg_index] = NULL;

    /*
     *  最後の引数をチェックして "&" ならば
     *
     *  "&" を引数から削る
     *  コマンドの状態を表す status に 1 を設定する
     *
     *  そうでなければ status に 0 を設定する
     */

    if(arg_index > 0 && strcmp(args[arg_index - 1], "&") == 0) {

        --arg_index;
        args[arg_index] = '\0';
        status = 1;

    } else {

        status = 0;

    }

    /*
     *  引数が何もなかった場合
     */

    if(arg_index == 0) {
        status = 3;
    }

    /*
     *  コマンドの状態を返す
     */

    return status;
}

/*----------------------------------------------------------------------------
 *
 *  関数名   : execute_command
 *
 *  作業内容 : 引数として与えられたコマンドを実行する
 *             コマンドの状態がフォアグラウンドならば、コマンドを
 *             実行している子プロセスの終了を待つ
 *             バックグラウンドならば子プロセスの終了を待たずに
 *             main 関数に返る（プロンプト表示に戻る）
 *
 *  引数     :
 *
 *  返り値   :
 *
 *  注意     :
 *
 *--------------------------------------------------------------------------*/

void execute_command(char *args[],    /* 引数の配列 */
                     int command_status)     /* コマンドの状態 */
{
    int pid;      /* プロセスＩＤ */
    int status;   /* 子プロセスの終了ステータス */

    /*
     *  子プロセスを生成する
     *
     *  生成できたかを確認し、失敗ならばプログラムを終了する
     */

    /******** Your Program ********/
    if((pid = fork()) == -1){
      perror("fork");
      exit(1);
    }

    /*
     *  子プロセスの場合には引数として与えられたものを実行する
     *
     *  引数の配列は以下を仮定している
     *  ・第１引数には実行されるプログラムを示す文字列が格納されている
     *  ・引数の配列はヌルポインタで終了している
     */

    /******** Your Program ********/
    if(pid == 0){
      execvp(args[0], args);
      exit(0);
    }
    /*
     *  コマンドの状態がバックグラウンドなら関数を抜ける
     */

    /******** Your Program ********/
    if(command_status == 1){
      return;
    }
    /*
     *  ここにくるのはコマンドの状態がフォアグラウンドの場合
     *
     *  親プロセスの場合に子プロセスの終了を待つ
     */
    /******** Your Program ********/
    if(pid > 0){
      wait(&status);
    }
    return;
}

/*-- END OF FILE -----------------------------------------------------------*/
