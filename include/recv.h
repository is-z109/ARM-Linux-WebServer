#include<USART.h>
#include <cstring>
using namespace std;
#define PACKET_HEADER_1 0xAA
#define PACKET_HEADER_2 0x55
extern mutex json_lock;
enum MsgType {
    MSG_SENSOR_DATA = 0x01, // 传感器数据回传
    MSG_MOTOR_CTRL = 0x02   // 电机控制指令
};
typedef struct __attribute__((packed)){
        uint8_t header1;  // 固定为 0xAA
        uint8_t header2;  // 固定为 0x55
    
    /* --- 2. 帧信息 (Frame Info) --- */
        uint8_t msg_type; // 指令/数据类型，见上面的枚举
    
    /* --- 3. 有效数据载荷 (Payload) --- */
    // 下面放你真正要传的数据，必须使用定长类型
        float    temperature;   // 4字节 (例如: 26.5)
        float    humidity;      // 4字节 (例如: 60.2)
        int32_t  motor_speed;   // 4字节 (例如: 1500 rpm)
        uint8_t  switch_status; // 1字节 (例如: 0关闭, 1打开)
    
    /* --- 4. 校验位 (Checksum) --- */
        uint8_t checksum; // 校验和 (通常是前面所有字节的累加和，或异或值)
    }data_b;
class r_data{
    USART usart;
    int header;
public:
    r_data():header(0)
    {
        usart.USART_Init(B9600);
    }
    /*bool drecv(data_b &recvdata)
    {
        int size=0;
        int n=0;
        while(1)
        {
            n=usart.Recv_Message(sizeof(data_b),size);
            if (n < 0) return false; // 发生底层错误，退出
            if (n == 0) continue;    // 1秒超时，继续等
            size+=n;
            if(size==sizeof(data_b))
            {
                if(usart.buffer[0]==PACKET_HEADER_1&&usart.buffer[1]==PACKET_HEADER_2)
                {   
                     memcpy(&recvdata,usart.buffer,sizeof(data_b));
                     return true;
                }
                else
                {
                    size=0;
                    return false;
                }
            }
        }
    }*/
    bool drecv(data_b &recvdata)
    {
        int lastread=0,n;
        uint8_t temp[17];
        while(1)
      {
         n=usart.Recv_Message();
            if (n < 0) return false; // 发生底层错误，退出
        if(header==0)
        {
            for(;usart.readptr+1<usart.writeptr;usart.readptr++)
            {
                if(usart.buffer[(usart.readptr%128)]==PACKET_HEADER_1&&usart.buffer[(usart.readptr+1)%128]==PACKET_HEADER_2)
                {
                    header=1;
                    usart.readptr+=2;
                    temp[0]=0xAA;
                    temp[1]=0x55;
                    lastread+=2;
                    break;
                }
            }
        }
        if(header==1)
        {
            for(;usart.readptr<=usart.writeptr&&lastread<17;usart.readptr++,lastread++)
            {
                temp[lastread]=usart.buffer[usart.readptr%128];
            }
            if(lastread==17)
            {
                unique_lock<mutex>lock(json_lock);
                memcpy(&recvdata,temp,sizeof(data_b));
                header=0;
                return true;
            }
            
        }
        if(n==0)
        {
            usleep(10000);
        }
     }
    }
};