#include"GPIO.h"
string GPIO::direction_path{"/sys/class/gpio/gpio"};
string GPIO::value_path{"/sys/class/gpio/gpio"};
void GPIO::GPIO_Init()
{
    //unique_lock<mutex>lock(gpio_mutex);
    ofstream infile("/sys/class/gpio/export",std::ios::trunc);
    if(!infile.is_open())
    {
        cout<<"Failed to open GPIO export file"<<endl<<std::flush;
        return;
    }
    infile<<pin<<std::flush;
    infile.close();
}
void GPIO::GPIO_Write(char value)
{
    ofstream infile;
    infile.open(value_path+pin+"/value",std::ios::trunc);
    if(!infile.is_open())
    {
        cout<<"Failed to open GPIO value file"<<endl<<std::flush;
        return;
    }
    infile<<value<<std::flush;
    infile.close();
}
void GPIO:: GPIO_direction(string de)
{
    //unique_lock<mutex>lock(gpio_mutex);
    ofstream infile(direction_path+pin+"/direction",std::ios::trunc);
    if(!infile.is_open())
    {
        cout<<"Failed to open GPIO direction file"<<endl<<std::flush;
        return;
    }
    infile<<de<<std::flush;
    infile.close();
}
char GPIO:: GPIO_GetInput()
{
    //unique_lock<mutex>lock(gpio_mutex);
    ifstream outfile(value_path+pin+"/value");
    if(!outfile.is_open())
    {
        cout<<"Failed to open GPIO value file"<<endl<<std::flush;
        return 'e';
    }
    char value='n';
    outfile>>value;
    if(value!='n')
    return value;
    else
    return 'e';
}
GPIO::~GPIO()
{
    ofstream infile("/sys/class/gpio/unexport",std::ios::trunc);
    if(!infile.is_open())
    {
        cout<<"Failed to open GPIO unexport file"<<endl<<std::flush;
        return;
    }
    infile<<pin;
    infile.close();
}
//cmake .. -DCMAKE_C_COMPILER=arm-none-linux-gnueabihf-gcc -DCMAKE_CXX_COMPILER=arm-none-linux-gnueabihf-g++