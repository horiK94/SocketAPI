#include <stdio.h> //printf(), fprintf(), perror()
#include <sys/socket.h> //socket(), bind(), accept(), listen()
#include <arpa/inet.h> // struct sockaddr_in, struct sockaddr, inet_ntoa()
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset()
#include <unistd.h> //close()

#define QUEUELIMIT 5

// サーバサイド
int main(int argc, char* argv[]) {

    int servSock; //server socket descripter
    int clitSock; //client socket descripter
    // socketaddr_inの説明は下
    struct sockaddr_in servSockAddr; //server internet socket address
    struct sockaddr_in clitSockAddr; //client internet socket address
    unsigned short servPort; //server port number
    unsigned int clitLen; // client internet socket address length

    // argcは引数の個数. 今回は./tcpd.c 8080のように２つ無いとエラーが出るようになっている
    if ( argc != 2) {
        fprintf(stderr, "argument count mismatch error.\n");
        exit(EXIT_FAILURE);
    }

    //servPotにatoi(argv[1])を代入して、この値が0なら無効なポート番号として返す
    /*
        文字列で表現された数値をint型の数値に変換する。
        変換不能なアルファベットなどの文字列の場合は0を返すが、数値が先頭にあればその値を返す。

        つまり、ポート番号のチェックをしているというより、ポート番号を記入すべき第２引数が数字になってないときエラーを表示する
    */
    if ((servPort = (unsigned short) atoi(argv[1])) == 0) {
        fprintf(stderr, "invalid port number.\n");
        exit(EXIT_FAILURE);
    }

    // http://research.nii.ac.jp/~ichiro/syspro98/server.html
    // http://neineigh.hatenablog.com/entry/2013/09/28/185053
    /*
    1. socketの生成 (socket)
    2. 登録 (bind)
    3. 接続準備 (listen)
    4. 接続待機
    5. データの送受信
    6. 切断
    という流れがある

    scoketは第一引数にプロトコルファミリと呼ばれるプロトコルの種類を指定する。 PF_INETとしてインターネットを利用することとし、
    第二引数を SOCK_STREAMとしてコネクション型のソケットであることを宣言する。
    第3引数は使用プロトコルを指定する．0で自動設定。IPPROTO_TCPと書かないとダメの場合もある模様。
    成功したときはソケットの番号を返し、失敗したときは負(-1)を返す
    */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ){
        perror("socket() failed.");     // 引数の標準エラー出力を出力する
        exit(EXIT_FAILURE);
    }

    //memset: buf の先頭から n バイト分 ch をセットします。buf を超えてセットした場合の動作は未定義。
    /*
        void *memset(void *buf, int ch, size_t n);
        void *buf　：　セット先のメモリブロック
        int ch　　：　セットする文字
        size_t n　：　セットバイト数 
    */
    memset(&servSockAddr, 0, sizeof(servSockAddr));
    /*
        接続先のIPアドレスやポート番号の情報を保持するための構造体.
        struct sockaddr_in{  
            short sin_family;       //AddressFamily(目的の通信をするために必要な通信プロトコルをまとめたもの?)
            unsigned short sin_port;    //IP port
        struct in_addr sin_addr;        //IP Address
            char sin_zero[8];          //SOCKADDRと同じサイズの構造体にするための詰め物
        };  
    */ 
    servSockAddr.sin_family      = AF_INET;
    // htonl()でネットワークバイトオーダに変換してから代入
    /*
        sin.sin_addr.s_addr:
        通常INADDR_ANYで指定する．
        特定のIPアドレスを指定するとそのアドレスにきた要求だけを受け付けるようになる．
        INADDR_ANYだとどれでも要求を受け付ける．
    */
    servSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    /*
    sin.sin_portを使用するポート番号で初期化．
    htons()で同じく変換してから代入．
    htonl()との違いは，引数と返り値がlの方はu_longで，sの方はs_short．
    */
    servSockAddr.sin_port        = htons(servPort);

    // socketの登録を行う
    /*
    bind()はソケットのアドレスやポート番号、通信方式を登録する。
    第１引数には登録するソケットの番号を与える(socket()で返された値を代入する)
    第２引数にはそのソケット自体のアドレスやポート番号を定義する((struct sockaddr *)でsockaddrのポインタ変数に変換してる)
    第３引数は第２引数のsizeを代入する

    int bind(int sockfd, const struct sockaddr *addr,
         socklen_t addrlen);
    */
    if (bind(servSock, (struct sockaddr *) &servSockAddr, sizeof(servSockAddr) ) < 0 ) {
        perror("bind() failed.");
        exit(EXIT_FAILURE);
    }


    // listen: ソケット待機準備 (サーバーがクライアントから接続要求の受け入れ用意をする)
    /*
        第１引数: 接続準備する対象のソケット番号
        第２引数: 接続要求のための待ち行列の最大数
        => 複数の接続要求が来た時に、サーバーが処理中だと接続要求を失うことがあるため処理きれない接続要求を一時的に待ち状態にする必要がある.その数が入る。
        最大数はOSに寄るため、5以下にすることが多いらしい

        失敗は-1を返す。
    */
    if (listen(servSock, QUEUELIMIT) < 0) {
        perror("listen() failed.");
        exit(EXIT_FAILURE);
    }

    // クライアントの要求まち
    while(1) {
        clitLen = sizeof(clitSockAddr);
        //accept: クライアントからの接続要求の待機を待つのに使う。acceptが実行されると、一旦停止し接続要求後にaccept直後からスタートする
        if ((clitSock = accept(servSock, (struct sockaddr *) &clitSockAddr, &clitLen)) < 0) {
            perror("accept() failed.");
            exit(EXIT_FAILURE);
        }

        printf("connected from %s.\n", inet_ntoa(clitSockAddr.sin_addr));
        close(clitSock);
    }


    return EXIT_SUCCESS;
}