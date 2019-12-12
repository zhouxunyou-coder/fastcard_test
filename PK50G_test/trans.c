#include "trans.h"
#define _GNU_SOURCE 1
#include <sched.h>
#include <ctype.h>
#include <signal.h>
#include <linux/ioctl.h>
cfg_t cfg;

#define INCREASE_NUM(a) ((a) = (++(a) >= 0x7FFFFFFF)?1:(a))

int udp_connect()
{
  int sockfd = -1;
  int size = 1024*1024*300;
  // struct sockaddr_in addr;  

  // addr.sin_family = PF_INET;
  // addr.sin_port = htons(port);
  // addr.sin_addr.s_addr = inet_addr(ip);
  // socklen_t socklen;
  // socklen = sizeof(struct sockaddr_in);
  CHK2(sockfd, socket(PF_INET, SOCK_DGRAM, 0));

  setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (void *)&size, sizeof(int));

  return sockfd;
}

int udp_sendto(int sockfd)
{
  static unsigned int seq = 0, nseq = 0, dseq = 0;
  unsigned int t = 0;
  struct timeval start; 
  struct timeval end;
  unsigned long speed = 0;

  int slen = -1;
  unsigned long snum = 0;
  unsigned long ssize = 0;
  char sbuf[TRANS_CMSGSIZE] = {0};

  gettimeofday(&start, NULL);
  msg_t *msg = (msg_t *)sbuf;
  strcpy(msg->data, "Helloworld");

  struct sockaddr_in addr;  

  addr.sin_family = PF_INET;
  addr.sin_port = htons(cfg.port);
  addr.sin_addr.s_addr = inet_addr(cfg.ip);
  socklen_t socklen;
  socklen = sizeof(struct sockaddr_in);


  while(1){
    INCREASE_NUM(seq);
    msg->seq = seq;


    CHK2(slen, sendto(sockfd, sbuf, TRANS_MSGSIZE, 0, (struct sockaddr *)&addr, socklen));
    //printf("[send] len = %d, seq = %d, data = %s\n", slen, msg->seq, msg->data);

    ssize += slen;
    snum ++;
    //printf("[send] len = %d, seq = %d, data = %s\n", slen, msg->seq, msg->data);
    if(snum == (unsigned long)cfg.bagnum){
      usleep(cfg.delay);
      snum = 0;
    }
    gettimeofday(&end, NULL);
    t = (long)((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec));
    if(t >= 1000000){//1s

      memcpy(&start, &end, sizeof(struct timeval));

      speed = (long)(ssize/t);
      printf("speed = %ldMB/s, seq = %d\n", (speed*1000000)/(1024*1024), msg->seq);	

      gettimeofday(&start, NULL);
      ssize = 0;
    }
  }	
}

int udp_listen()
{
  int sockfd = -1;
  int size = 1024*1024*300;
  struct sockaddr_in addr, their_addr;  

  addr.sin_family = PF_INET;
  addr.sin_port = htons(cfg.port);
  addr.sin_addr.s_addr = inet_addr(cfg.ip);
  socklen_t socklen;
  socklen = sizeof(struct sockaddr_in);

  CHK2(sockfd, socket(PF_INET, SOCK_DGRAM, 0));

  setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void *)&size, sizeof(int));

  CHK(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)));

  return sockfd;
}

int udp_recvfrom(int sockfd)
{
  static unsigned int seq = 1;
  int rlen = -1;
  char rbuf[TRANS_CMSGSIZE] = {0};
  msg_t *msg = (msg_t *)rbuf;
  memset(rbuf, '\0', sizeof(rbuf));
  time_t t1, t2;
  time(&t1);
  t2 = t1;
  int flag = 0;
  system("date");
  while(1){
    CHK2(rlen, recvfrom(sockfd, rbuf, TRANS_MSGSIZE, 0, NULL, NULL));
    //printf("[recv] len = %d, seq = %d, data = %s\n", rlen, msg->seq, msg->data);

    if(msg->seq != seq){

      //printf("[error] seq error, recv:%d local:%d!!!\n", msg->seq, seq);
      //system("cat /proc/sys/asic/ipq_len");
      seq = msg->seq;
      //if (flag == 0){
        //system("cat /proc/sys/asic/ipq_len");
        //system("cat /proc/meminfo");
        //system("date");
      //}
      flag = 1;
      //exit(-1);
    }

    time(&t2); 
    if ((t2 - t1) > 3){
      t1 = t2; 
      if (flag){
        printf("[error] seq error!!!!!!!!!!!!!\n");
      }
      flag = 0;
    }

    INCREASE_NUM(seq);
  }
}

char *opensd(int *sockfd)
{
  if(!strcmp(cfg.type, "C")){
    *sockfd = udp_connect();
  }
  else{
    *sockfd = udp_listen();
  }
}

char *closesd(int *sockfd)
{
  close(*sockfd);
}

char *sendsd_message(int sockfd)
{
  udp_sendto(sockfd);	
}

char *recvsd_message(int sockfd)
{
  udp_recvfrom(sockfd);
}

char *openchar(int *fd)
{
  int channel_id = 1;

  //mknod("/dev/ipq", S_IFCHR | S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH, makedev(220, 0));

  if(!strcmp(cfg.type, "C")){
    CHK2(*fd, open("/dev/ipq", O_WRONLY));
    if (ioctl(*fd, IPQ_CMD_ATTACH_CHID, channel_id) != 0) {
      printf("Set channel ID failed\n", channel_id);
      exit(-1);
    }
  }
  else{
    CHK2(*fd, open("/dev/ipq", O_RDONLY));
    if (ioctl(*fd, IPQ_CMD_SET_CHID, channel_id) != 0) {
      printf("Set channel ID failed\n", channel_id);
      exit(-1);
    }
  }


}

char *closechar(int *fd)
{
  close(*fd);
}

char *sendchar_message(int fd)
{
  static unsigned int seq = 0, nseq = 0, dseq = 0;
  long t = 0;
  struct timeval start; 
  struct timeval end;
  struct timeval t1, t2;
  unsigned long speed = 0;

  int slen = -1;
  //    unsigned long snum = 0;
  unsigned long ssize = 0;
  char *sbuf = NULL;
  int pos = 0;
  int size = cfg.bagnum * TRANS_CMSGSIZE;
  int i = 0;
  msg_t *msg = NULL;
  long n1 = 0;
  long count =  1;
  long total = cfg.bagnum;
  long minnum = 0;
  long datalen = 0;

  n1 = cfg.delay * 1024 * 1024/152200;

  minnum = (n1 > total) ? total:n1;
  total = minnum;

  sbuf = (char *) malloc(size + TRANS_CMSGSIZE);	
  memset(sbuf, 1, size + TRANS_CMSGSIZE);
  gettimeofday(&start, NULL);
  gettimeofday(&end, NULL);
  gettimeofday(&t1, NULL);
  gettimeofday(&t2, NULL);

  while(1){

    pos = 0;
    for (i = 0; i < total; i ++){

      msg = (msg_t *)(sbuf + pos);
      memset(msg->data, 0xFF, TRANS_CMSGSIZE - sizeof(msg_t));
      //strcpy(msg->data, "Helloworld");
      msg->speed = seq;
      INCREASE_NUM(seq);
      msg->seq = seq;
      pos += TRANS_CMSGSIZE;
      count ++;
    }	

    datalen = pos;

    pos = 0;

    while(pos < datalen){
      slen = write(fd, sbuf + pos, datalen - pos);
      if (slen != datalen){
        printf("write=%d\n", slen);
      }
      if(slen <= 0){
        perror("write");
        continue;
      }

      ssize += slen;
      pos += slen;
    }
    //printf("[send] len = %d, seq = %d, data = %s\n", slen, msg->seq, msg->data);
    //usleep(1000); 

    if (count >= n1){
SLEEP:
      gettimeofday(&t2, NULL);
      t = (long)((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec));
      t = 10000 - t ;//- 20000;
      //printf("t=%d\n", t);
      if (t > 0){
        usleep(t);
      }

      gettimeofday(&t1, NULL);
      count = 0;
      total = minnum;
    }
    else{
      total = n1 - count;
      total = (total > minnum)? minnum:total;
    }

    gettimeofday(&end, NULL);


    t = (long)((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec));

    if(t >= 1000000){//1s

      //memcpy(&start, &end, sizeof(struct timeval));

      speed = (long)(ssize/t);
      printf("speed = %ldMB/s, seq = %d\n", (speed*1000000)/(1024*1024), msg->seq);	

      gettimeofday(&start, NULL);
      ssize = 0;
    }
  }	
}

char *recvchar_message(int fd)
{
  static unsigned int seq = 1;
  int rlen = -1;
  char *rbuf = NULL;
  time_t t1, t2;
  int flag = 0;
  int pos = 0;
  int bufflen = TRANS_CMSGSIZE * 1025;
  msg_t *msg = NULL;

  rbuf = (char *)malloc(bufflen);
  msg = (msg_t *)rbuf;
  int i = 0;
  int counter = 0;

  time(&t1);
  t2 = t1;
  system("date");
  while(1){
    rlen = read(fd, rbuf, bufflen);
    if(rlen <= 0){
      usleep(1000);
      continue;
      printf("rlen = %d\n", rlen);
      perror("read");
    }

    pos = 0;
    //printf("[recv] len = %d, seq = %d, data = %s\n", rlen, msg->seq, msg->data);
    while(rlen > 0){

      msg = (msg_t *)(rbuf + pos);

      if(msg->seq != seq){
        printf("[%s] msg->seq=%u  seq=%u\n", __func__, msg->seq, seq);
        /*
        
        if (msg->seq > seq){
          flag += (msg->seq - seq);
        }
        else{
          flag += (0x0FFFFFFF - seq + msg->seq);
        }
        */
        seq = msg->seq;
        //exit(-1);
      }

      INCREASE_NUM(seq);
     /*  
      for(i=0; i<1522-sizeof(msg_t); i++){
          if(msg->data[i] != 0xFF){
              if (counter == 0){
                printf("chk_sum erron seq = %u, data:%d\n", msg->seq, msg->data[i]);
              }
              counter = ++counter % 100;
          }
      }
      */
      /*
      for(i = 0; i < 1510; i += 64){
        if (!(0xFF & msg->data[i])){
          printf("chk_sum erron seq = %u\n", msg->seq);
          break;
        }
      }
      */

      pos += TRANS_CMSGSIZE;
      rlen -= TRANS_CMSGSIZE;

      //usleep(200);
    }
    /*
       if (flag){

       printf("seq error recv:%u local:%u drop=%d rlen=%d\n", msg->seq, seq, flag, rlen);
       system("cat /proc/sys/asic/ipq_len");
       system("dmesg -c");
    //system("cat /proc/meminfo");
    system("date");
    flag = 0;
    }*/

    /*
    time(&t2); 
    if ((t2 - t1) > 3){
      t1 = t2; 
      if (flag){

        system("date");
        printf("seq error drop=%d\n", flag);
        system("cat /proc/sys/asic/ipq_len");
        system("dmesg -c");
        //system("cat /proc/meminfo");
        flag = 0;
      }
    }
    */

  }
}

void usage()
{
  printf("trans -d (1 charset, 0 udp) -t S (server) -n pack number  -m net band(MB) -h listen ip -p listen port\n");
  printf("trans -t C (client) -n pack number  -m net band(MB) -h connect server ip -p connect port\n");
}

void parseoptions(int argc, char *argv[])
{
  if(argc < 2){
    usage();
    exit(-1);
  }
  char c;

  cfg.method = 1;
  cfg.type = NULL;
  cfg.bagnum = 0;
  cfg.delay = 0;

  cfg.port = SERVERPORT;
  strcpy(cfg.ip, SERVERIP);

  while ((c = getopt(argc, argv, "d:t:n:m:h:p:")) != -1)
  {
    switch(c)
    {
      case 'd':
        cfg.method = atoi(optarg);
        if(cfg.method == 0){
          printf("cfg.method = UDP\n");
        }
        else{
          printf("cfg.method = CHAR\n");
        }
        break;
      case 't':
        cfg.type = optarg;
        printf("cfg.type = %s\n", cfg.type);
        break;
      case 'n':
        cfg.bagnum = atoi(optarg);
        printf("cfg.bagnum = %d\n", cfg.bagnum);
        break;
      case 'm':
        cfg.delay = atoi(optarg);
        printf("cfg.delay = %d\n", cfg.delay);
        break;
      case 'h':
        strcpy(cfg.ip, optarg);
        printf("cfg.ip = %s\n", cfg.ip);
        break;
      case 'p':
        cfg.port = atoi(optarg);
        printf("cfg.port = %d\n", cfg.port);
        break;
      default:
        break;
    }
  }
}
void do_sigquit(int signr)
{
  printf("[%s]  signal\n", __func__);
  //exit (0);
}
void do_sigterm(int signr)
{
  printf("[%s]  signal\n", __func__);
  //exit (0);
}

void do_sigint(int signr)
{
  printf("[%s]  signal\n", __func__);
  //exit (0);
}


int main(int argc, char *argv[])
{
  int fd = -1;
  /*
  cpu_set_t mask;

  CPU_ZERO(&mask);
  CPU_SET(0, &mask);

  if (sched_setaffinity(0, sizeof(mask), &mask) == -1){
    printf("bind cpu 3 error \n");
  }
  */

  if(signal( SIGPIPE, SIG_IGN ) == SIG_ERR \
      ||signal( SIGTERM, do_sigterm ) == SIG_ERR \
      ||signal( SIGINT, do_sigint ) == SIG_ERR \
      ||signal( SIGHUP, do_sigint ) == SIG_ERR \
      ||signal( SIGQUIT, do_sigquit ) == SIG_ERR){
    perror("signal error");
    return -1;
  }

  trans_t pt[] = {{opensd, closesd, sendsd_message, recvsd_message}, \
    {openchar, closechar, sendchar_message, recvchar_message}};
  trans_t *ps = NULL;

  parseoptions(argc, argv);

  if(cfg.method == 0){
    ps = &pt[0];
  }
  else{
    ps = &pt[1];
  }

  ps->openfd(&fd);
  if(!strcmp(cfg.type, "C")){
    ps->send_message(fd);
  }
  else{
    ps->recv_message(fd);
  }
  ps->closefd(&fd);

}
