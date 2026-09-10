#include"recv.h"
using namespace std;
/*enum MsgType {
    MSG_SENSOR_DATA = 0x01, // 传感器数据回传
    MSG_MOTOR_CTRL = 0x02   // 电机控制指令
};*/
/*typedef struct __attribute__((packed)){
        uint8_t header1;  // 固定为 0xAA
        uint8_t header2;  // 固定为 0x55
    
    /* --- 2. 帧信息 (Frame Info) --- */
        //uint8_t msg_type; // 指令/数据类型，见上面的枚举
    
    /* --- 3. 有效数据载荷 (Payload) --- */
    // 下面放你真正要传的数据，必须使用定长类型
        //float    temperature;   // 4字节 (例如: 26.5)
        //float    humidity;      // 4字节 (例如: 60.2)
        //int32_t  motor_speed;   // 4字节 (例如: 1500 rpm)
        //uint8_t  switch_status; // 1字节 (例如: 0关闭, 1打开)
    
    /* --- 4. 校验位 (Checksum) --- */
        //uint8_t checksum; // 校验和 (通常是前面所有字节的累加和，或异或值)
    //}
//extern data_b stm32data;
r_data stm32;
void updata(data_b &data)
{
    bool state;
    while(1)
    {
    state=stm32.drecv(data);
    }
    //cout<<data.temperature<<endl;
}