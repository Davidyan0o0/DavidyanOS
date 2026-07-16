#include "kernel/shell.h"

#include "kernel/console.h"
#include "kernel/system.h"
#include "kernel/lib/string.h"
#include "kernel/arch/x86/timer.h"
#include "kernel/arch/x86/rtc.h"
#include "kernel/arch/x86/power.h"
#include "kernel/memory/heap.h"
#include "kernel/fs/ramfs.h"
#include "kernel/net/net.h"
#include "kernel/device/device.h"
#include "drivers/video/vga.h"
#include "drivers/storage/ramdisk.h"
#include "kernel/graphics/image.h"
#include "drivers/storage/ata.h"
#include "kernel/fs/diskfs.h"
#include "drivers/net/ne2k.h"


#define SHELL_INPUT_MAX 128


static char input_buffer[SHELL_INPUT_MAX];
static int input_length = 0;
static char disk_edit_name[DISKFS_NAME_MAX];
static char disk_edit_buffer[DISKFS_DATA_MAX];
static unsigned int disk_edit_open = 0;
static unsigned int disk_edit_dirty = 0;


static void shell_prompt()
{

    print("DavidyanOS> ");

}



static void shell_reset_input()
{

    input_length = 0;
    input_buffer[0] = 0;

}



static unsigned int shell_parse_uint(char* text)
{

    unsigned int value = 0;


    while(*text>='0' && *text<='9')
    {

        value = value*10 + (*text-'0');
        text++;

    }


    return value;

}



static char* shell_skip_word(char* text)
{

    while(*text && *text!=' ')
    {

        text++;

    }

    while(*text==' ')
    {

        text++;

    }

    return text;

}



static void shell_copy_text(char* dest,char* src,unsigned int capacity)
{

    unsigned int i;

    if(capacity==0)
    {

        return;

    }

    for(i=0;i<capacity-1 && src[i];i++)
    {

        dest[i] = src[i];

    }

    dest[i] = 0;

}



static unsigned int shell_append_text(char* dest,char* src,unsigned int capacity)
{

    unsigned int length = (unsigned int)strlen(dest);
    unsigned int i = 0;

    if(capacity==0)
    {

        return 0;

    }

    while(length<capacity-1 && src[i])
    {

        dest[length] = src[i];
        length++;
        i++;

    }

    dest[length] = 0;
    return i;

}



static int shell_split_two(char* text,char** first,char** second)
{

    char* separator = text;

    while(*separator && *separator!=' ')
    {

        separator++;

    }

    if(*separator!=' ')
    {

        return 0;

    }

    *separator = 0;
    *first = text;
    *second = separator+1;

    while(**second==' ')
    {

        (*second)++;

    }

    return **second!=0;

}



static void print_two_digits(unsigned int value)
{

    vga_put_char('0'+((value/10)%10));
    vga_put_char('0'+(value%10));

}



static void shell_print_help()
{

    println("Commands:");
    println("  help, clear, info, uptime, ticks");
    println("  time, date, mem, alloc <bytes>");
    println("  devs, disk, disk read <n>, disk write <n> <text>");
    println("  ata, dfs, dfs write/cat/rm/format <file>");
    println("  dfs open/view/edit/append/save/status for disk files");
    println("  ls, mkdir <dir>, touch <file>, cat/write/append/rm/mv");
    println("  color <fg> <bg>, box, rect, theme");
    println("  img <file> [scale], imginfo <file>, demoimg");
    println("  net, ip <addr>, route <gw>, arp, arp add <ip> <mac>");
    println("  dns <name>, dhcp, sockets, listen/close <port>");
    println("  link <up|down>, ping <ip>, send <ip> <text>, rx <text>");
    println("  netsend <text> sends a real NE2000 ethernet frame");
    println("  arpreq <ip>, udp <ip> <port> <text>");
    println("  echo <text>, reboot, halt");

}



static void shell_print_fsinfo()
{

    print("Files: ");
    print_uint(ramfs_count()-ramfs_dir_count());
    print(" dirs: ");
    print_uint(ramfs_dir_count());
    print("/");
    print_uint(RAMFS_FILE_MAX);
    print(" bytes used: ");
    print_uint(ramfs_used_bytes());
    print(" free: ");
    print_uint(ramfs_free_bytes());
    println("");

}



static void shell_list_files()
{

    unsigned int i;
    RAMFS_NODE* node;


    for(i=0;i<ramfs_count();i++)
    {

        node = ramfs_get(i);
        if(node)
        {

            print(node->name);
            print(node->directory ? "  <DIR>  " : "  <FILE> ");
            print("  ");
            print_uint(node->size);
            println(" bytes");

        }

    }

}



static void shell_print_devices()
{

    unsigned int i;
    DEVICE* dev;

    for(i=0;i<device_count();i++)
    {

        dev = device_get(i);
        if(dev)
        {

            print_uint(dev->id);
            print("  ");
            print(dev->name);
            print("  ");
            println(dev->type);

        }

    }

}



static void shell_print_disk()
{

    print("rd0 sectors=");
    print_uint(ramdisk_sector_count());
    print(" sector_size=");
    print_uint(RAMDISK_SECTOR_SIZE);
    print(" bytes total=");
    print_uint(ramdisk_size_bytes());
    println("");

}



static void shell_print_diskfs()
{

    unsigned int i;
    DISKFS_ENTRY* entry;

    print("diskfs mounted=");
    println(diskfs_mounted() ? "yes" : "no");
    for(i=0;i<diskfs_count();i++)
    {
        entry = diskfs_get(i);
        if(entry)
        {
            print(entry->name);
            print("  ");
            print_uint(entry->size);
            println(" bytes");
        }
    }

}



static void shell_print_diskfs_error()
{

    unsigned int error = diskfs_last_error();

    if(error==DISKFS_ERROR_NOT_MOUNTED)
    {

        print("diskfs not mounted");

    }
    else if(error==DISKFS_ERROR_BAD_NAME)
    {

        print("bad file name");

    }
    else if(error==DISKFS_ERROR_NO_SPACE)
    {

        print("no free disk file slots");

    }
    else if(error==DISKFS_ERROR_DATA_WRITE)
    {

        print("data sector write failed");

    }
    else if(error==DISKFS_ERROR_SUPER_WRITE)
    {

        print("directory sector write failed");

    }
    else if(error==DISKFS_ERROR_DATA_READ)
    {

        print("data sector read failed");

    }
    else
    {

        print("unknown diskfs error");

    }

    print(" ata=");
    print_hex(ata_last_status());
    println("");

}



static void shell_disk_edit_status()
{

    if(!disk_edit_open)
    {

        println("No disk file open");
        return;

    }

    print("Open: ");
    print(disk_edit_name);
    print(" size=");
    print_uint((unsigned int)strlen(disk_edit_buffer));
    print(" dirty=");
    println(disk_edit_dirty ? "yes" : "no");

}



static void shell_disk_edit_open(char* name)
{

    if(name[0]==0)
    {

        println("Usage: dfs open <file>");
        return;

    }

    shell_copy_text(disk_edit_name,name,DISKFS_NAME_MAX);
    if(!diskfs_read(name,disk_edit_buffer,DISKFS_DATA_MAX))
    {

        disk_edit_buffer[0] = 0;
        println("New disk edit buffer");

    }
    else
    {

        println("Disk file loaded");

    }

    disk_edit_open = 1;
    disk_edit_dirty = 0;

}



static void shell_disk_edit_view(char* name)
{

    char data[DISKFS_DATA_MAX];

    if(name[0]!=0)
    {

        if(!diskfs_read(name,data,DISKFS_DATA_MAX))
        {

            println("disk file not found");
            return;

        }

        println(data);
        return;

    }

    if(!disk_edit_open)
    {

        println("Usage: dfs view [file]");
        return;

    }

    println(disk_edit_buffer);

}



static void shell_disk_edit_replace(char* data)
{

    if(!disk_edit_open)
    {

        println("Use dfs open <file> first");
        return;

    }

    shell_copy_text(disk_edit_buffer,data,DISKFS_DATA_MAX);
    disk_edit_dirty = 1;
    println("Disk edit buffer updated");

}



static void shell_disk_edit_append(char* data)
{

    if(!disk_edit_open)
    {

        println("Use dfs open <file> first");
        return;

    }

    shell_append_text(disk_edit_buffer,data,DISKFS_DATA_MAX);
    disk_edit_dirty = 1;
    println("Disk edit buffer appended");

}



static void shell_disk_edit_save()
{

    if(!disk_edit_open)
    {

        println("Use dfs open <file> first");
        return;

    }

    if(!diskfs_write(disk_edit_name,disk_edit_buffer))
    {

        print("disk save failed: ");
        shell_print_diskfs_error();
        return;

    }

    disk_edit_dirty = 0;
    println("Disk file saved");

}



static void shell_print_arp()
{

    unsigned int i;
    ARP_ENTRY* entry;

    for(i=0;i<net_arp_count();i++)
    {

        entry = net_get_arp(i);
        if(entry)
        {

            net_print_ip(entry->ip);
            print("  ");
            println(entry->mac);

        }

    }

}



static void shell_print_net()
{

    NET_DEVICE* dev = net_get_device();

    print(dev->name);
    print(" link=");
    println(dev->link_up ? "up" : "down");
    print("hardware=");
    println(dev->hardware ? "ne2k" : "none");
    if(dev->hardware)
    {
        unsigned int i;
        print("mac=");
        for(i=0;i<6;i++)
        {
            if(i>0)
            {
                print(":");
            }
            print_hex(dev->mac[i]);
        }
        println("");
    }
    print("ip=");
    net_print_ip(dev->ip);
    print(" gateway=");
    net_print_ip(dev->gateway);
    print(" dns=");
    net_print_ip(dev->dns);
    println("");
    print("tx=");
    print_uint(dev->tx_packets);
    print(" hwtx=");
    print_uint(ne2k_tx_count());
    print(" rx=");
    print_uint(dev->rx_packets);
    print(" drop=");
    print_uint(dev->dropped_packets);
    println("");

}



static void shell_draw_theme()
{

    unsigned char old_color = vga_get_color();

    vga_clear();
    vga_draw_box(1,1,78,23,0x0A);
    vga_draw_box(4,3,32,8,0x0B);
    vga_draw_box(40,3,35,8,0x0E);
    vga_fill_rect(5,13,70,5,' ',0x1F);
    vga_set_color(15,0);
    print("DavidyanOS graphics surface");
    vga_put_at(6,5,'F',0x0B);
    vga_put_at(8,5,'S',0x0B);
    vga_put_at(42,5,'N',0x0E);
    vga_put_at(44,5,'E',0x0E);
    vga_put_at(46,5,'T',0x0E);
    vga_set_color(old_color&0x0F,old_color>>4);

}



static void shell_print_time()
{

    RTC_TIME time;


    rtc_read_time(&time);
    print_two_digits(time.hour);
    vga_put_char(':');
    print_two_digits(time.minute);
    vga_put_char(':');
    print_two_digits(time.second);
    println("");

}



static void shell_print_date()
{

    RTC_TIME time;


    rtc_read_time(&time);
    print_uint(time.year);
    vga_put_char('-');
    print_two_digits(time.month);
    vga_put_char('-');
    print_two_digits(time.day);
    println("");

}



static void shell_execute(char* command)
{

    if(strcmp(command,"")==0)
    {

        return;

    }


    if(strcmp(command,"help")==0)
    {

        shell_print_help();
        return;

    }


    if(strcmp(command,"clear")==0)
    {

        vga_clear();
        return;

    }


    if(strcmp(command,"info")==0)
    {

        system_print_info();
        return;

    }


    if(strcmp(command,"uptime")==0)
    {

        print("Uptime: ");
        print_uint(timer_get_seconds());
        print("s (");
        print_uint(timer_get_ticks());
        println(" ticks)");
        return;

    }


    if(strcmp(command,"ticks")==0)
    {

        print("Ticks: ");
        print_uint(timer_get_ticks());
        println("");
        return;

    }


    if(strcmp(command,"time")==0)
    {

        shell_print_time();
        return;

    }


    if(strcmp(command,"date")==0)
    {

        shell_print_date();
        return;

    }


    if(strcmp(command,"mem")==0)
    {

        print("Heap start: ");
        print_hex(heap_get_start());
        print(" end: ");
        print_hex(heap_get_end());
        print(" used: ");
        print_uint(heap_get_used());
        print(" free: ");
        print_uint(heap_get_free());
        println(" bytes");
        return;

    }


    if(strcmp(command,"devs")==0)
    {

        shell_print_devices();
        return;

    }


    if(strcmp(command,"disk")==0)
    {

        shell_print_disk();
        return;

    }


    if(strcmp(command,"ata")==0)
    {

        print("ATA status=");
        print_hex(ata_last_status());
        print(" identify=");
        println(ata_identify() ? "ok" : "fail");
        return;

    }


    if(strcmp(command,"dfs")==0)
    {

        shell_print_diskfs();
        return;

    }


    if(strcmp(command,"dfs format")==0)
    {

        if(!diskfs_format())
        {

            print("format failed: ");
            shell_print_diskfs_error();
            return;

        }
        println("diskfs formatted");
        disk_edit_open = 0;
        disk_edit_dirty = 0;
        disk_edit_name[0] = 0;
        disk_edit_buffer[0] = 0;
        return;

    }


    if(strcmp(command,"dfs status")==0)
    {

        shell_disk_edit_status();
        return;

    }


    if(strncmp(command,"dfs open ",9)==0)
    {

        shell_disk_edit_open(command+9);
        return;

    }


    if(strcmp(command,"dfs view")==0)
    {

        shell_disk_edit_view("");
        return;

    }


    if(strncmp(command,"dfs view ",9)==0)
    {

        shell_disk_edit_view(command+9);
        return;

    }


    if(strncmp(command,"dfs edit ",9)==0)
    {

        shell_disk_edit_replace(command+9);
        return;

    }


    if(strncmp(command,"dfs append ",11)==0)
    {

        shell_disk_edit_append(command+11);
        return;

    }


    if(strcmp(command,"dfs save")==0)
    {

        shell_disk_edit_save();
        return;

    }


    if(strncmp(command,"dfs write ",10)==0)
    {

        char* name = command+10;
        char* data = shell_skip_word(name);

        if(data[0]==0)
        {
            println("Usage: dfs write <file> <text>");
            return;
        }
        *(data-1) = 0;
        println(diskfs_write(name,data) ? "disk file written" : "disk write failed");
        return;

    }


    if(strncmp(command,"dfs cat ",8)==0)
    {

        char data[DISKFS_DATA_MAX];
        if(!diskfs_read(command+8,data,DISKFS_DATA_MAX))
        {
            println("disk file not found");
            return;
        }
        println(data);
        return;

    }


    if(strncmp(command,"dfs rm ",7)==0)
    {

        println(diskfs_delete(command+7) ? "disk file deleted" : "disk file not found");
        return;

    }


    if(strncmp(command,"disk read ",10)==0)
    {

        char sector[RAMDISK_SECTOR_SIZE];
        unsigned int index = shell_parse_uint(command+10);
        if(!ramdisk_read(index,sector))
        {

            println("Bad sector");
            return;

        }

        println(sector);
        return;

    }


    if(strncmp(command,"disk write ",11)==0)
    {

        unsigned int index = shell_parse_uint(command+11);
        char* data = shell_skip_word(command+11);

        if(data[0]==0 || !ramdisk_write(index,data))
        {

            println("Usage: disk write <sector> <text>");
            return;

        }

        println("Sector written");
        return;

    }


    if(strcmp(command,"fsinfo")==0)
    {

        shell_print_fsinfo();
        return;

    }


    if(strcmp(command,"ls")==0)
    {

        shell_list_files();
        return;

    }


    if(strncmp(command,"cat ",4)==0)
    {

        RAMFS_NODE* node = ramfs_find(command+4);
        if(node==0)
        {

            println("File not found");
            return;

        }

        if(node->directory)
        {

            println("Is a directory");
            return;

        }

        println(node->data);
        return;

    }


    if(strncmp(command,"mkdir ",6)==0)
    {

        println(ramfs_mkdir(command+6) ? "Directory created" : "mkdir failed");
        return;

    }


    if(strncmp(command,"touch ",6)==0)
    {

        println(ramfs_touch(command+6) ? "File ready" : "touch failed");
        return;

    }


    if(strncmp(command,"write ",6)==0)
    {

        char* name = command+6;
        char* data = shell_skip_word(name);
        if(data[0]==0)
        {

            println("Usage: write <file> <text>");
            return;

        }

        *(data-1) = 0;

        if(!ramfs_write(name,data))
        {

            println("Usage: write <file> <text>");
            return;

        }

        println("File written");
        return;

    }


    if(strncmp(command,"append ",7)==0)
    {

        char* name = command+7;
        char* data = shell_skip_word(name);

        if(data[0]==0)
        {

            println("Usage: append <file> <text>");
            return;

        }

        *(data-1) = 0;

        println(ramfs_append(name,data) ? "File appended" : "append failed");
        return;

    }


    if(strncmp(command,"mv ",3)==0)
    {

        char* first;
        char* second;

        if(!shell_split_two(command+3,&first,&second) || !ramfs_rename(first,second))
        {

            println("Usage: mv <old> <new>");
            return;

        }

        println("Renamed");
        return;

    }


    if(strncmp(command,"rm ",3)==0)
    {

        println(ramfs_delete(command+3) ? "File deleted" : "File not found");
        return;

    }


    if(strncmp(command,"color ",6)==0)
    {

        unsigned int fg = shell_parse_uint(command+6);
        unsigned int bg = shell_parse_uint(shell_skip_word(command+6));
        vga_set_color((unsigned char)fg,(unsigned char)bg);
        println("Color changed");
        return;

    }


    if(strcmp(command,"box")==0)
    {

        vga_draw_box(10,5,60,12,0x0B);
        return;

    }


    if(strcmp(command,"rect")==0)
    {

        vga_fill_rect(18,8,44,7,' ',0x2F);
        return;

    }


    if(strcmp(command,"theme")==0)
    {

        shell_draw_theme();
        return;

    }


    if(strncmp(command,"imginfo ",8)==0)
    {

        IMAGE image;
        if(!image_load_file(command+8,&image))
        {
            println("Image load failed (use P3 PPM)");
            return;
        }

        image_print_info(&image);
        return;

    }


    if(strncmp(command,"img ",4)==0)
    {

        IMAGE image;
        char* file = command+4;
        char* scale_text = shell_skip_word(file);
        int scale = 1;

        if(scale_text[0]!=0)
        {
            *(scale_text-1) = 0;
            scale = (int)shell_parse_uint(scale_text);
        }

        if(!image_load_file(file,&image))
        {
            println("Image load failed (use P3 PPM)");
            return;
        }

        vga_clear();
        image_display(&image,2,2,scale);
        return;

    }


    if(strcmp(command,"demoimg")==0)
    {

        IMAGE image;
        image_make_demo(&image);
        vga_clear();
        image_display(&image,2,2,2);
        return;

    }


    if(strcmp(command,"net")==0)
    {

        shell_print_net();
        return;

    }


    if(strcmp(command,"sockets")==0)
    {

        unsigned int i;
        NET_SOCKET* socket;

        for(i=0;i<net_socket_count();i++)
        {
            socket = net_get_socket(i);
            if(socket)
            {
                print_uint(socket->port);
                print("  ");
                print(socket->protocol);
                print("  ");
                println(socket->state);
            }
        }

        return;

    }


    if(strncmp(command,"listen ",7)==0)
    {

        unsigned int port = shell_parse_uint(command+7);
        println(net_socket_open((unsigned short)port,"tcp") ? "Socket listening" : "Listen failed");
        return;

    }


    if(strncmp(command,"close ",6)==0)
    {

        unsigned int port = shell_parse_uint(command+6);
        println(net_socket_close((unsigned short)port) ? "Socket closed" : "Socket not found");
        return;

    }


    if(strncmp(command,"dns ",4)==0)
    {

        IPV4_ADDR ip;
        if(!net_dns_resolve(command+4,&ip))
        {
            println("DNS name not found");
            return;
        }

        net_print_ip(ip);
        println("");
        return;

    }


    if(strcmp(command,"dhcp")==0)
    {

        net_dhcp_refresh();
        println("DHCP lease refreshed");
        return;

    }


    if(strncmp(command,"ip ",3)==0)
    {

        IPV4_ADDR ip;
        if(!net_parse_ipv4(command+3,&ip))
        {

            println("Bad IPv4 address");
            return;

        }

        net_set_ip(ip);
        println("IP updated");
        return;

    }


    if(strncmp(command,"route ",6)==0)
    {

        IPV4_ADDR ip;
        if(!net_parse_ipv4(command+6,&ip))
        {

            println("Bad gateway address");
            return;

        }

        net_set_gateway(ip);
        println("Gateway updated");
        return;

    }


    if(strcmp(command,"arp")==0)
    {

        shell_print_arp();
        return;

    }


    if(strncmp(command,"arp add ",8)==0)
    {

        IPV4_ADDR ip;
        char* first;
        char* second;

        if(!shell_split_two(command+8,&first,&second) || !net_parse_ipv4(first,&ip) || !net_arp_add(ip,second))
        {

            println("Usage: arp add <ip> <mac>");
            return;

        }

        println("ARP entry saved");
        return;

    }


    if(strcmp(command,"link up")==0)
    {

        net_set_link(1);
        println("net0 link up");
        return;

    }


    if(strcmp(command,"link down")==0)
    {

        net_set_link(0);
        println("net0 link down");
        return;

    }


    if(strncmp(command,"ping ",5)==0)
    {

        IPV4_ADDR ip;
        if(!net_parse_ipv4(command+5,&ip))
        {

            println("Bad IPv4 address");
            return;

        }

        if(!net_ping(ip))
        {

            println("Network is down");

        }

        return;

    }


    if(strncmp(command,"send ",5)==0)
    {

        IPV4_ADDR ip;
        char* address = command+5;
        char* payload = shell_skip_word(address);
        char* separator = address;

        while(*separator && *separator!=' ')
        {

            separator++;

        }

        if(*separator!=' ')
        {

            println("Usage: send <ip> <text>");
            return;

        }

        *separator = 0;
        if(!net_parse_ipv4(address,&ip))
        {

            println("Bad IPv4 address");
            return;

        }

        if(!net_send(ip,payload))
        {

            println("Send failed");

        }

        return;

    }


    if(strncmp(command,"netsend ",8)==0)
    {

        println(net_send_raw_demo(command+8) ? "ethernet frame sent" : "hardware send failed");
        return;

    }


    if(strncmp(command,"arpreq ",7)==0)
    {

        IPV4_ADDR ip;
        if(!net_parse_ipv4(command+7,&ip))
        {
            println("Bad IPv4 address");
            return;
        }

        println(net_send_arp_request(ip) ? "arp request sent" : "arp send failed");
        return;

    }


    if(strncmp(command,"udp ",4)==0)
    {

        IPV4_ADDR ip;
        char* address = command+4;
        char* port_text = shell_skip_word(address);
        char* payload = shell_skip_word(port_text);
        unsigned int port;

        if(port_text[0]==0 || payload[0]==0)
        {
            println("Usage: udp <ip> <port> <text>");
            return;
        }

        *(port_text-1) = 0;
        *(payload-1) = 0;
        port = shell_parse_uint(port_text);
        if(!net_parse_ipv4(address,&ip))
        {
            println("Bad IPv4 address");
            return;
        }

        println(net_send_udp(ip,(unsigned short)port,payload) ? "udp packet sent" : "udp send failed");
        return;

    }


    if(strncmp(command,"rx ",3)==0)
    {

        net_receive_demo(command+3);
        return;

    }


    if(strncmp(command,"alloc ",6)==0)
    {
        unsigned int size = shell_parse_uint(command+6);
        void* address = kmalloc(size);


        if(address==0)
        {

            println("Allocation failed");
            return;

        }


        print("Allocated ");
        print_uint(size);
        print(" bytes at ");
        print_hex((unsigned int)address);
        println("");
        return;

    }


    if(strncmp(command,"echo ",5)==0)
    {

        println(command+5);
        return;

    }


    if(strcmp(command,"reboot")==0)
    {

        println("Rebooting...");
        system_reboot();
        return;

    }


    if(strcmp(command,"halt")==0)
    {

        println("System halted");
        system_halt();
        return;

    }


    print("Unknown command: ");
    println(command);

}



void shell_init()
{

    shell_reset_input();
    shell_prompt();

}



void shell_handle_key(char c)
{

    if(c=='\n')
    {

        vga_put_char('\n');
        input_buffer[input_length] = 0;
        shell_execute(input_buffer);
        shell_reset_input();
        shell_prompt();
        return;

    }


    if(c=='\b')
    {

        if(input_length>0)
        {

            input_length--;
            input_buffer[input_length] = 0;
            vga_backspace();

        }


        return;

    }


    if(input_length>=SHELL_INPUT_MAX-1)
    {

        return;

    }


    input_buffer[input_length] = c;
    input_length++;
    input_buffer[input_length] = 0;
    vga_put_char(c);

}
