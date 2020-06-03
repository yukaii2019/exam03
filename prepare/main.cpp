#include "mbed.h"
#include "mbed_rpc.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "math.h"

#define PI acos(-1)
#define UINT14_MAX 16383
// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR0 (0x1E << 1) // with pins SA0=0, SA1=0
#define FXOS8700CQ_SLAVE_ADDR1 (0x1D << 1) // with pins SA0=1, SA1=0
#define FXOS8700CQ_SLAVE_ADDR2 (0x1C << 1) // with pins SA0=0, SA1=1
#define FXOS8700CQ_SLAVE_ADDR3 (0x1F << 1) // with pins SA0=1, SA1=1
// FXOS8700CQ internal register addresses
#define FXOS8700Q_STATUS 0x00
#define FXOS8700Q_OUT_X_MSB 0x01
#define FXOS8700Q_OUT_Y_MSB 0x03
#define FXOS8700Q_OUT_Z_MSB 0x05
#define FXOS8700Q_M_OUT_X_MSB 0x33
#define FXOS8700Q_M_OUT_Y_MSB 0x35
#define FXOS8700Q_M_OUT_Z_MSB 0x37
#define FXOS8700Q_WHOAMI 0x0D
#define FXOS8700Q_XYZ_DATA_CFG 0x0E
#define FXOS8700Q_CTRL_REG1 0x2A
#define FXOS8700Q_M_CTRL_REG1 0x5B
#define FXOS8700Q_M_CTRL_REG2 0x5C
#define FXOS8700Q_WHOAMI_VAL 0xC7

I2C i2c(PTD9, PTD8);
Serial pc(USBTX, USBRX);
Serial xbee(D12, D11);

EventQueue collectData_queue(32*EVENTS_EVENT_SIZE);
EventQueue RPC_queue(32*EVENTS_EVENT_SIZE);
Thread collectData_thread(osPriorityNormal);
Thread RPC_thread(osPriorityNormal);
DigitalOut led(LED1);
int m_addr = FXOS8700CQ_SLAVE_ADDR1;
void FXOS8700CQ_readRegs(int addr, uint8_t *data, int len);
void FXOS8700CQ_writeRegs(uint8_t *data, int len);
void get_volocity_XBEE(Arguments *in,Reply *out);
void get_volocity(float *v,float *vx,float *vy);
void collectData();
void RPC_function();
RPCFunction rpcGET_VOLOCITY(&get_volocity_XBEE,"GET_VOLOCITY");

float volocity[150];
float volocity_x[150];
float volocity_y[150];

void reply_messange(char *xbee_reply, char *messange)
{
    xbee_reply[0] = xbee.getc();
    xbee_reply[1] = xbee.getc();
    xbee_reply[2] = xbee.getc();
    if (xbee_reply[1] == 'O' && xbee_reply[2] == 'K')
    {
        pc.printf("%s\r\n", messange);
        xbee_reply[0] = '\0';
        xbee_reply[1] = '\0';
        xbee_reply[2] = '\0';
    }
}
void check_addr(char *xbee_reply, char *messenger)
{
    xbee_reply[0] = xbee.getc();
    xbee_reply[1] = xbee.getc();
    xbee_reply[2] = xbee.getc();
    xbee_reply[3] = xbee.getc();
    pc.printf("%s = %c%c%c\r\n", messenger, xbee_reply[1], xbee_reply[2], xbee_reply[3]);
    xbee_reply[0] = '\0';
    xbee_reply[1] = '\0';
    xbee_reply[2] = '\0';
    xbee_reply[3] = '\0';
}

void get_volocity(float *v,float *vx,float *vy)
{
    int16_t acc16;
    float t[3];
    uint8_t res[6];
    FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);
    acc16 = (res[0] << 6) | (res[1] >> 2);
    if (acc16 > UINT14_MAX / 2)
        acc16 -= UINT14_MAX;
    t[0] = ((float)acc16) / 4096.0f;
    acc16 = (res[2] << 6) | (res[3] >> 2);
    if (acc16 > UINT14_MAX / 2)
        acc16 -= UINT14_MAX;
    t[1] = ((float)acc16) / 4096.0f;
    acc16 = (res[4] << 6) | (res[5] >> 2);
    if (acc16 > UINT14_MAX / 2)
        acc16 -= UINT14_MAX;
    t[2] = ((float)acc16) / 4096.0f;
    
    float v_x,v_y;
    float v_horizontal;
    v_x = t[0]*0.1;
    v_y = t[1]*0.1;
    v_horizontal = sqrt(v_x*v_x+v_y*v_y);
    *v = v_horizontal;
    *vx = v_x;
    *vy = v_y;
}

void collectData(){
    wait(1);
    led = 1;
    pc.printf("start collect");
    for(int i=0;i<100;i++){
        collectData_queue.call(get_volocity,&volocity[i],&volocity_x[i],&volocity_y[i]);
        wait(0.1);
    }
    pc.printf("collect finish");
    led = 0 ;// calculate finish
}

void get_volocity_XBEE(Arguments *in,Reply *out){
    for(int i=0;i<100;i++){
        char outbuf[6];
        int b=sprintf(outbuf,"%+1.6f",volocity[i]);
        xbee.printf("%s",outbuf);
        wait(0.2);       
        b=sprintf(outbuf,"%+1.6f",volocity_x[i]);
        xbee.printf("%s",outbuf);
        wait(0.2); 
        b=sprintf(outbuf,"%+1.6f",volocity_y[i]);
        xbee.printf("%s",outbuf);
        wait(0.2);  
    }
}

void FXOS8700CQ_readRegs(int addr, uint8_t *data, int len)
{
    char t = addr;
    i2c.write(m_addr, &t, 1, true);
    i2c.read(m_addr, (char *)data, len);
}

void FXOS8700CQ_writeRegs(uint8_t *data, int len)
{
    i2c.write(m_addr, (char *)data, len);
}

void RPC_function(){
    char buf[256], outbuf[256];
    while (true)
    {
        memset(buf, 0, 256); // clear buffer
        for (int i = 0; i < 255; i++)
        {
            char recv = xbee.getc();
            if (recv == '\r' || recv == '\n')
            {
                pc.printf("\r\n");
                break;
            }
            buf[i] = pc.putc(recv);
            //buf[i] = recv;
        }
        RPC::call(buf, outbuf);
        pc.printf("%s\r\n", outbuf);
    }
}

int main()
{

    uint8_t data_acc[2];
    // Enable the FXOS8700Q
    FXOS8700CQ_readRegs(FXOS8700Q_CTRL_REG1, &data_acc[1], 1);
    data_acc[1] |= 0x01;
    data_acc[0] = FXOS8700Q_CTRL_REG1;
    FXOS8700CQ_writeRegs(data_acc, 2);
    
    pc.baud(9600);
    char xbee_reply[4];
    // XBee setting
    xbee.baud(9600);
    xbee.printf("+++");
    xbee_reply[0] = xbee.getc();
    xbee_reply[1] = xbee.getc();
    if (xbee_reply[0] == 'O' && xbee_reply[1] == 'K')
    {
        pc.printf("enter AT mode.\r\n");
        xbee_reply[0] = '\0';
        xbee_reply[1] = '\0';
    }
    xbee.printf("ATMY 0x265\r\n");
    reply_messange(xbee_reply, "setting MY : 0x265");
    xbee.printf("ATDL 0x165\r\n");
    reply_messange(xbee_reply, "setting DL : 0x165");
    xbee.printf("ATWR\r\n");
    reply_messange(xbee_reply, "write config");
    xbee.printf("ATMY\r\n");
    check_addr(xbee_reply, "MY");
    xbee.printf("ATDL\r\n");
    check_addr(xbee_reply, "DL");
    xbee.printf("ATCN\r\n");
    reply_messange(xbee_reply, "exit AT mode");
    pc.printf("start\r\n");


    collectData_thread.start(callback(&collectData_queue, &EventQueue::dispatch_forever));
    RPC_thread.start(callback(&RPC_queue,&EventQueue::dispatch_forever));
    RPC_queue.call(RPC_function);
    collectData();
    return 0;    
}