#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
 
#define BUF_SIZE 100
void error_handling(char *buf);
 
int main(int argc, char *argv[])
{
    //소켓, timeout값, fd_set값 등을 저장할 변수 선언
    int serv_sock, clnt_sock;
    fd_set reads, cpy_reads;
    socklen_t adr_temp;
    struct sockaddr_in serv_adr, clnt_adr;
    struct timeval timeout;
    int fd_max, str_len, fd_num, i;
    char buf[BUF_SIZE];
 
  //입력이 일치하지 않으면 오류를 출력하고 종료한다.
    if(argc!=2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
 
    //서버측 소켓 생성
    serv_sock=socket(PF_INET, SOCK_STREAM, 0);
    
    //서버의 주소정보를 저장, 이때 네트워크 바이트 순서로 저장한다
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(atoi(argv[1]));
    
    //bind 함수를 통해 소켓에 서버의 주소정보를 할당
    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
        error_handling("bind() error");
 
    //연결 대기 상태로 진입
    if(listen(serv_sock, 5)==-1)
        error_handling("listen() error");
 
    //fd_set형 변수 reads의 모든 비트를 0으로 초기화
    FD_ZERO(&reads);
    //소켓 디스크립터 정보를 등록
    FD_SET(serv_sock, &reads);
    //소켓의 번호를 저장
    fd_max=serv_sock;
 
    while(1) {
        cpy_reads=reads;
        //타임아웃 시간 설정
        timeout.tv_sec=5;
        timeout.tv_usec=5000;
        
  
        //오류가 발생시
        if((fd_num=select(fd_max+1, &cpy_reads, 0, 0, &timeout))==-1) {
            break;
        }
            //Timeout이 발생했다면
        if(fd_num==0) {
            continue;
        }
     
        for(i=0; i<fd_max+1; i++) {
            if(FD_ISSET(i, &cpy_reads)) {
                // 연결요청이 발생시 if문
                if(i==serv_sock) {
                    //연결요청 수락    
                    adr_temp=sizeof(clnt_adr);
                    clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_temp);
                    
                    //해당 연결을 감시 FD_SET을 통해 설정
                    FD_SET(clnt_sock, &reads);
                 
                    //연결을 통해 clnt_sock 값이 증가했다면 fd_max 재설정
                    //(연결종료 된 클라이언트 < 새로 연결된 클라이언트)이라면 
                    if(fd_max<clnt_sock)
                        fd_max=clnt_sock;
                    printf("connected client: %d \n", clnt_sock);
                }
                else {
                     
                    str_len=read(i, buf, BUF_SIZE);
                  
                    if(str_len==0) {    
                        FD_CLR(i, &reads);
                        close(i);
                        printf("closed client: %d \n", i);
                    }
                    //메세지가 왔다면
                    else {
     
                        for(int j=4; j < fd_max + 1; j++) { 
                            write(j, buf, str_len);
                        }
                    }
                }
            }
        }
    }
    close(serv_sock);
    return 0;
}
 
void error_handling(char *buf) {
    fputs(buf, stderr);
    fputc('\n', stderr);
    exit(1);
}
