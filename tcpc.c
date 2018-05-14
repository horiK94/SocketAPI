#include <stdio.h> //printf(), fprintf(), perror()
#include <sys/socket.h> //socket(), bind(), accept(), listen()
#include <arpa/inet.h> // struct sockaddr_in, struct sockaddr, inet_ntoa(), inet_aton()
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset(), strcmp()
#include <unistd.h> //close()

#define MSGSIZE 1024
#define BUFSIZE (MSGSIZE + 1)

// クライアントサイド
/*
    流れは
    ソケット生成
    ソケット接続要求
    受信/送信
*/
int main(int argc, char* argv[]) {

    int sock; //local socket descripter
    struct sockaddr_in servSockAddr; //server internet socket address
    /*  サーバーサイドと同じ説明
        接続先のIPアドレスやポート番号の情報を保持するための構造体.
        struct sockaddr_in{  
            short sin_family;       //AddressFamily(目的の通信をするために必要な通信プロトコルをまとめたもの?)
            unsigned short sin_port;    //IP port
        struct in_addr sin_addr;        //IP Address
            char sin_zero[8];          //SOCKADDRと同じサイズの構造体にするための詰め物
        };  
    */
    unsigned short servPort; //server port number
    char recvBuffer[BUFSIZE];//receive temporary buffer
    char sendBuffer[BUFSIZE]; // send temporary buffer

    // 引数は3つ必要
    if (argc != 3) {
        fprintf(stderr, "argument count mismatch error.\n");
        exit(EXIT_FAILURE);
    }

    memset(&servSockAddr, 0, sizeof(servSockAddr));
    
    servSockAddr.sin_family = AF_INET;      // AF_INETはIPv4を表す

    /*
        inet_aton(): '127.0.0.1'といったIPアドレス(の文字列)をネットワークバイトオーダの)バイナリ値へ変換してin_addr構造体に格納する。
        ネットワークバイトオーダー: ネットワーク上で、バイナリデータをやり取りするときに使われるバイトオーダー（バイトの並び順）
        ちなみに…TCP/IPではヘッダなどをビッグエンディアンで送ることが定められている(データ領域のオーダーはソフトウェアやプロトコルが個別に規定する)。
        アドレスが有効な時は0以外, 無効な時は0を返す
    */
    if (inet_aton(argv[1], &servSockAddr.sin_addr) == 0) {
        fprintf(stderr, "Invalid IP Address.\n");
        exit(EXIT_FAILURE);
    }

    //servPotにatoi(argv[2])を代入して、この値が0なら無効なポート番号として返す
    if ((servPort = (unsigned short) atoi(argv[2])) == 0) {
        fprintf(stderr, "invalid port number.\n");
        exit(EXIT_FAILURE);
    }

    servSockAddr.sin_port = htons(servPort);

    // ソケットの生成
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ){
        perror("socket() failed.");
        exit(EXIT_FAILURE);
    }

    // サーバとの接続
    /*
        第１引数にはsocket()が返したソケット番号を入れる
        第２引数には接続先のサーバーを指定するsockaddr構造体のポインタを入れる
        (ネット通信では、scoketadd_in構造体ではなく、sockaddr構造体にキャストしてから与える必要があることに注意)
    */
    if (connect(sock, (struct sockaddr*) &servSockAddr, sizeof(servSockAddr)) < 0) {
        perror("connect() failed.");
        exit(EXIT_FAILURE);
    }

    // inet_ntoa: 整数のIPアドレスからドット表記のIPアドレス(192.168.0.1みたいなの)の文字列を取得する
    printf("connect to %s\n", inet_ntoa(servSockAddr.sin_addr));

    while(1){
        printf("please enter the characters:");
        // 入力待ち
        /*
            fgets:
            http://www.c-tipsref.com/reference/stdio/fgets.html 
            戻り値: 第１引数
            １文字も読み取れない場合は空ポインタ(NULL)
            読み取り失敗時: 空ポインタ

            stdin: 標準入力のこと
        */
        if (fgets(sendBuffer, BUFSIZE, stdin) == NULL){
            fprintf(stderr, "invalid input string.\n");
            exit(EXIT_FAILURE);
        }

        // ソケットへメッセージを送信する　send = write(2)
        /*
            第１引数: socket番号
            第２引数: 送信するメッセージが入ったバッファーのポインター
            第３引数: 送信するメッセージのサイズ
            第４引数: 呼び出しの方法を指定するフラグをセットする

            正常に実行された場合、send() は、
            0 または送信されたバイト数を示す 0 以上の値を戻します
        */
        if (send(sock, sendBuffer, strlen(sendBuffer), 0) < 0) {
            perror("send() failed.");
            exit(EXIT_FAILURE);
        }


        int byteRcvd  = 0;
        int byteIndex = 0;
        while (byteIndex < MSGSIZE) {
            // recv()は受信バッファキューに溜まっているバイト列をユーザープロセスに取り込むシステムコール
            byteRcvd = recv(sock, &recvBuffer[byteIndex], 1, 0);
            printf("%d", sock);
            
            if (byteRcvd > 0) {
                if (recvBuffer[byteIndex] == '\n'){
                    recvBuffer[byteIndex] = '\0';
                    if (strcmp(recvBuffer, "quit") == 0) {
                        close(sock);
                        return EXIT_SUCCESS;
                    } else {
                        break;
                    }
                }
                byteIndex += byteRcvd;
            } else if(byteRcvd == 0){
                perror("ERR_EMPTY_RESPONSE");
                exit(EXIT_FAILURE);
            } else {
                perror("recv() failed.");
                exit(EXIT_FAILURE);
            }
        }
        printf("server return: %s\n", recvBuffer);
    }

    return EXIT_SUCCESS;
}
