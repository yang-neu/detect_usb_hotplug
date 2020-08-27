#include <unistd.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <stddef.h>  
#include <string.h>  
#include <errno.h>  
#include <getopt.h>  
#include <fcntl.h>  
#include <errno.h>  
#include <signal.h>  
#include <getopt.h>  
#include <sys/time.h>  
#include <sys/socket.h>  
#include <sys/un.h>  
#include <sys/select.h>  
#include <linux/types.h>  
#include <linux/netlink.h>  
#include <sys/types.h>
#include <libudev.h>
#include <iostream>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/stat.h>
using namespace std;
static std::string record_path="";

static bool FindRecord=false;
void File_Opreation();
void GetUsbFolderLocation(char *basePath);
void MountTheSystem(char *basePath);
static bool NeedToCheck=false;
void Udev_Enumrate()
{
    struct udev* udev_ancestor=NULL;
    struct udev_enumerate* udev_enum=NULL;
    struct udev_list_entry* device_fistentry=NULL;
    struct udev_list_entry *dev_list_entry=NULL; //entry to store the current position
    struct udev_device *dev=NULL;
    udev_ancestor=udev_new();
    udev_enum=udev_enumerate_new(udev_ancestor);
    if(udev_enumerate_add_match_subsystem (udev_enum, "block")==0)
    {
        cout<<"add block device to match subsystem successful"<<endl;
    }
    
    if(udev_enumerate_add_match_subsystem (udev_enum, "usb")==0)
    {
        cout<<"add usb device to match subsystem successful"<<endl;
    }

    if(udev_enumerate_add_match_subsystem (udev_enum, "scsi")==0)
    {
        cout<<"add scsi device to match subsystem successful"<<endl;
    }

    //Scan the system under /sys/
    udev_enumerate_scan_devices(udev_enum);
    
    //get the first entry of the device list
    device_fistentry=udev_enumerate_get_list_entry(udev_enum);
    
    /* For each item enumerated, print out its information.
       udev_list_entry_foreach is a macro which expands to
       a loop. The loop will be executed for each member in
       devices, setting dev_list_entry to a list entry
       which contains the device's path in /sys. */
    udev_list_entry_foreach(dev_list_entry, device_fistentry)
    {
        const char *path;
        
        /* Get the filename of the /sys entry for the device
           and create a udev_device object (dev) representing it */
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev_ancestor, path);

        /* usb_device_get_devnode() returns the path to the device node
           itself in /dev. */
        printf("Test Device Node Path: %s\n", udev_device_get_devnode(dev));    
        

        /* The device pointed to by dev contains information about
        the hidraw device. In order to get information about the
        USB device, get the parent device with the
        subsystem/devtype pair of "usb"/"usb_device". This will
        be several levels up the tree, but the function will find
        it.*/
        dev = udev_device_get_parent_with_subsystem_devtype(
                                dev,
                                "usb",
                                "usb_device");
        if (!dev) 
        {
            cout<<"Test Unable to find parent usb device"<<endl;
            //exit(1);
        }
        else
        {        
            printf("  VID/PID: %s %s\n",udev_device_get_sysattr_value(dev,"idVendor"), udev_device_get_sysattr_value(dev, "idProduct"));
            printf("  %s\n  %s\n",udev_device_get_sysattr_value(dev,"manufacturer"), udev_device_get_sysattr_value(dev,"product"));
            printf("  serial: %s\n",udev_device_get_sysattr_value(dev, "serial"));
        }

        udev_device_unref(dev);
    }
    udev_enumerate_unref(udev_enum);
    udev_unref(udev_ancestor);

}

void* udev_Monitor(void*)
{
    struct udev* udev=NULL;
    struct udev_monitor * mon=NULL;
    struct udev_device *dev;
    int fd;
    fd_set fds;
    struct timeval tv;
    static int flag=0;
    
    udev=udev_new();
    mon=udev_monitor_new_from_netlink(udev,"udev");
    
    //udev_monitor_filter_add_match_subsystem_devtype(mon, "sound", "usb_device");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", "usb_device");
    //udev_monitor_filter_add_match_subsystem_devtype(mon, "block", "disk");
    //udev_monitor_filter_add_match_subsystem_devtype(mon, "block", "partition");
    //udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", "usb_interface");
    udev_monitor_enable_receiving(mon);
    fd = udev_monitor_get_fd(mon);
    while(1)
    {
                
        fd_set fds;
        struct timeval tv;
        int ret;
        
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        
        ret = select(fd+1, &fds, NULL, NULL, &tv);
        //ret means there's an event fd_isset means fd is readable
        if(ret>0 & FD_ISSET(fd,&fds))
        {
            //cout<<"There's a change with Num="<<flag<<endl;
            //flag++;
            /* Make the call to receive the device.
               select() ensured that this will not block. */
            dev = udev_monitor_receive_device(mon);
            if (dev) 
            {                    
                printf("Got Device\n");
                printf("   Node: %s\n", udev_device_get_devnode(dev));
                printf("   Subsystem: %s\n", udev_device_get_subsystem(dev));
                printf("   Devtype: %s\n", udev_device_get_devtype(dev));
                printf("   Action: %s\n",udev_device_get_action(dev));
                printf("   Path: %s\n",udev_device_get_syspath(dev));
                printf("   idVendor: %s\n",udev_device_get_sysattr_value(dev, "idVendor"));
                printf("   idProduct: %s\n",udev_device_get_sysattr_value(dev, "idProduct"));
                
                udev_device_unref(dev);
            }
            else 
            {
                printf("No Device from receive_device(). An error occured.\n");
            }        
        }
    }
}



//int main(int argc, char *argv[])  
int main()  
{      
    pthread_t monitor_thread=0;

    Udev_Enumrate();
    int err=0;
    err=pthread_create(&monitor_thread, NULL,udev_Monitor, NULL);
    if(err!=0)
    {
        cout<<"create thread error"<<endl;
    }    
    else
    {
        cout<<"create thread monitor success "<<endl;
    }
    
    
    if(monitor_thread!=0)
    {
        pthread_join(monitor_thread,NULL);
    }
    
    return 0;  
}
