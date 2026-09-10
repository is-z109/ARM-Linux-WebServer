#include<stdio.h>
#include<string>
#include<fcntl.h>
#include<termios.h>
#include<unistd.h>
#include<stdint.h>
using namespace std;
class USART{
    string addr;
    int fd;
public:
    unsigned int readptr;
    unsigned int writeptr;
    uint8_t buffer[128];
    USART(string a="/dev/ttyS1"):addr(a),readptr(0),writeptr(0){
        fd = open(addr.data(), O_RDWR | O_NOCTTY | O_SYNC);
        if (fd < 0) {
        perror("Error opening serial port");
        }
    }
    //参数：波特率
    void USART_Init(speed_t baudrate)
    {
        struct termios tty;
    
    if (tcgetattr(fd, &tty) != 0) {
        perror("Error getting tty attributes");
        return ;
    }

    cfsetospeed(&tty, baudrate); // 设置输出波特率
    cfsetispeed(&tty, baudrate); // 设置输入波特率

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8位数据位
    tty.c_cflag |= (CLOCAL | CREAD);            // 启用接收器，忽略调制解调器线状态
    tty.c_cflag &= ~(PARENB | PARODD);          // 无奇偶校验
    tty.c_cflag &= ~CSTOPB;                     // 1个停止位
    tty.c_cflag &= ~CRTSCTS;                    // 不使用RTS/CTS硬件流控

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);     // 关闭软件流控
    tty.c_iflag &= ~(ICRNL | INLCR);            // 关闭CR-LF转换

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // 关闭规范模式，关闭回显
    tty.c_oflag &= ~OPOST;                     // 关闭输出处理

    tty.c_cc[VMIN] = 0;  // 非阻塞模式
    tty.c_cc[VTIME] = 10; // 1秒超时

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("Error setting tty attributes");
        return ;
    }

    return ;
    }
    
    int Recv_Message()
    {
        int bytes_read = read(fd, (char*)buffer+(writeptr%128),128-(writeptr%128));
       if (bytes_read < 0) {
        perror("Error reading from serial port");
        return -1;
    }
    writeptr += bytes_read;
      return bytes_read;
    }
    ~USART()
    {
        if (close(fd) != 0) {
        perror("Error closing serial port");
    }
    }
};