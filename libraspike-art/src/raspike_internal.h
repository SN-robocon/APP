#ifndef __RASPIKE_INTERNAL_H_
#define __RASPIKE_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <pthread.h>
#include <errno.h>  
#include <stdlib.h>
  
  /* TODO: implement */
#define ENSURE_INIT()
#define ENSURE_VALID_PORT(port)
#define ENSURE_VALID_DEVICE(device)
#define DECLARE_DEVICE_TYPE_IN_FILE(device_type) \
  static unsigned char fgDeviceType = device_type;

  
#define GET_DEVICE_COMMON(device_t)			\
  ENSURE_INIT(); \
  RasPikePort r_port = PORT_TO_RASPIKE(port); \
  ENSURE_VALID_PORT(r_port); \
  RPPortDevice *dev = getDevice(r_port);   \
  if ( dev->device.device_type != 0 ) {    \
    printf("Port %c is already initialized as %d\n", \
	   port,dev->device.device_type); \
    return 0; \
  } \
  /* MAKE_CMD(type,0) means config*/ \
  char config_cmd = MAKE_CMD(fgDeviceType,0); \
  raspike_prot_send(r_port,config_cmd,0,0);	\
  int ret = raspike_timedwait_ack(r_port,config_cmd,3);	\
  if ( ret == 1 ) { \
    dev->device.device_type = fgDeviceType; \
    dev->device.port_id = r_port; \
    return (device_t*)&dev->device;		\
  } else if ( ret == ETIMEDOUT ) {					\
       printf("port =%c getdevice time out. check port connection\n",port);	\
       exit(-1); \
    return (device_t*)0;			\
  } else { \
       printf("port =%c getdevice Error. \n",port);	\
    return (device_t*)0;			\
  } 
  
  
#define DELAYED_SENSOR_COMMON(ret_type,cmd) \
  ENSURE_VALID_DEVICE(pdev); \
  RasPikePort port = pdev->port_id; \
  raspike_port_com_change_if_needed(port,cmd); \
  RPProtocolSpikeStatus *p = raspike_prot_get_saved_status(); \
  return *(ret_type*)p->ports[port].data;
  

#define GET_AND_RET_SENSOR_COMMON(ret_type,index) \
  ENSURE_VALID_DEVICE(pdev); \
  RasPikePort port = pdev->port_id; \
  RPProtocolSpikeStatus *p = raspike_prot_get_saved_status(); \
  RPProtocolPortStatus *pp = p->ports+pdev->port_id; \
  return *(ret_type *)(pp->data+index);		     \


#define WAITACK_AND_RET_COMMON(ret_type,wait_cmd) \
  ENSURE_VALID_DEVICE(pdev); \
  RasPikePort port = pdev->port_id;		\
  char cmd = wait_cmd; \
  raspike_prot_send(port,cmd,0,0); \
  int ret = raspike_wait_ack(port,cmd); \
  return (ret_type)ret;

#define WAITACK_AND_RET_COMMON_WITH_DATA(ret_type,wait_cmd,data,size)	\
  ENSURE_VALID_DEVICE(pdev); \
  RasPikePort port = pdev->port_id;		\
  char cmd = wait_cmd; \
  raspike_prot_send(port,cmd,(unsigned char*)data,size);	\
  int ret = raspike_wait_ack(port,cmd); \
  return (ret_type)ret;


  
struct _pup_device_t
{
  RasPikePort port_id;
  unsigned char device_type;
  unsigned char cmd;
  int32_t power; // for motor
};

  /* Port Structure */
typedef struct {
  struct _pup_device_t device;
  pthread_cond_t cond;
  pthread_mutex_t mutex;
  int32_t ack_cmd;
  int32_t ack_data;
} RPPortDevice;


#define RP_PORT_NUMBER (7)
  //  extern RPPortDevice  gPortDevices[RP_PORT_NUMBER];

  extern RPPortDevice* getDevice(RasPikePort port);

  extern int raspike_prot_lock_status(void);
  extern int raspike_prot_unlock_status(void);
  extern int raspike_prot_send(RasPikePort port, unsigned char cmdid, const unsigned char *buf, int size);

  extern int raspike_wait_ack(unsigned char port, unsigned char cmd);
  extern int raspike_timedwait_ack(unsigned char port, unsigned char cmd, int waitsec);

  extern int raspike_wait_port_cmd_change(RasPikePort port, unsigned char wait_cmd);
  extern int raspike_port_com_change_if_needed(RasPikePort port, unsigned char wait_cmd); 
  
  
#ifdef __cplusplus
}
#endif

  
#endif /* !__RASPIKE_INTERNAL_H_ */
