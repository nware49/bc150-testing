/* =======================================================
 ** Optical Modem multi-threaded Optical Application (OA)
 **
 ** Description:
 **
 ** History:
 **     Date      Who    Description
 **    -------    ---    ----------------------------------
 **   01/01/2010  SL     Create stub
 **   03/01/2010  JW     Add multiple processes to run AGC
 **                      sample comblock error rate
 **   04/02/2010  JW     Copied from thread_SOA.c, disable AGC
 **   05/27/2010  SL     Added last_update to OM_SOADAT
 **   04/13/2018  JW     Add new commands from hal 2018
 **   12/26/2018  JW     Conversion to Zynq based OM4
 **   11/06/2020  JW     Converted to Ultrascale based OM41
 **   05/10/2020  CM     Unified moa/soa_ultrascale to single
 **                      code; id's are assigne at compile time
 ** =======================================================
 */
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/stat.h>
#include <algorithm>
#include <chrono>

#define _USE_MATH_DEFINES
#include <math.h>

#include "om_util.h"
#include "net_comms.h"
#include "comutils.h"
// #include "adctest.h"

#include "main_common.h"
#include "om_systypes.h"
#include "fpga_mesg_funct.h"
#include "fpga_uio_interrupt.h"
#include "om4u_config.hpp"
#include "om4u_dma_capture.hpp"
#include "om4u_fpga_monitor.hpp"
#include "om4u_bearing.h"
#include "om4u_navigation.h"
#include "om4u_emitter.h"
#include "om4u_environment.h"
#include "om4u_message_receiver.h"
#include "om4u_orientation.h"
#include "om4u_pairing.h"
#include "om4u_wake.h"
#include "serial.h"
#include "version.h"

#include "om_shmem.h"

#define subtract32(a, b) (a < b) ? (unsigned int)(4294967296LL + (uint64_t)(a - b)) : a - b;

#if OM_NAV
#pragma message "  NAV capability"
#endif

#if OM_WAKE
#pragma message "  WAKE capability"
#endif

#if OM_DMA
#pragma message "  DMA capability"
#endif

pthread_mutex_t om4u_reg_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t thread_sync_10Hz = PTHREAD_COND_INITIALIZER;
pthread_cond_t thread_sync_100Hz = PTHREAD_COND_INITIALIZER;

// TEST GLOBAL variable
int loglevel;
int stat1hz_sel;

int main(int argc, char *argv[])
{
    int DEBUG_LEVEL = MSG; // ERR, WNG, MSG, DBG1, DBG2
    int my_id = SOA;
    int my_id2 = SOAA;
    const char *log_id = "OM4U_oa";

    int ret, cnt;
    //char msg_in[MAX_MESSAGE_SIZE];
    //char tmsg_in[MAX_MESSAGE_SIZE];
    char msg_out[MAX_MESSAGE_SIZE];
    char timestamp[TSTRING_SIZE];
    char settings_str[1024];
    //    char line_str[128];
    //    char * line_ptr;
    char *token;
    char *msg_ptr;
    const char space[2] = " ";
    //message_hdr_t msg_hdr;
    message_t msg;
    int done = 0;
    //u32 phy_tx_symrate;
    //u32 phy_rx_symrate;
    //	unsigned short env_error;
    //	int txercnum,rxercnum;
    int retval;
    //     int empty=0;
    //FILE *fptr_set;
    //FILE *fptr_tmp;

    //    u32 omtest_error;
    //    unsigned int omtest_berstat;

    //float emit_data[4];
    //    u16 err_data[10];
    //io_cfg_t scfg;
    //io_cfg_t scfg_emit;
    // for pthread
    pthread_t omfpga_pthread;
    pthread_t receiver_pthread_array[MAX_PIDS];
    //pthread_t ipdet_pthread;
    pthread_t emitter_control_pthread;
    pthread_t pairing_state_tracking_pthread;
#if OM_NAV
    pthread_t orientation_pthread;
    pthread_t navigation_listen_pthread;
#endif
#if OM_WAKE
    pthread_t wake_mission_pthread;
#endif
#if OM_DMA
    pthread_t dma_capture_pthread;
#endif
    pthread_argument_vector_t pthread_args = {};
    size_t pthread_stack_size;

    om_status_t om_status = {};
    //om_status_t modes_arr[100];
    //om_status_t *mode_ptr;

    nav_info_t nav_info;
#ifdef OM_NAV
    nav_status_t nav_status = {};

    struct timespec navdata_tic = {};
    struct timespec navdata_tic_last = {};
    struct timespec navdata_msg = {};
    struct timespec navdata_acq = {};
#endif

    om4u::modeListElement_t* mode_el;
    //int mode_cnt = 0;
    int new_mode = -1;
    //int confirm_mode = -2;
    //int selftest_mode = -2;
    //emit_settings_t emit_val;
    //int emitsetok = 0;
    //int pval;
    //char paramstr[64];
    //double modereq_time = 0.0;

    //    om_tstatus_t omfpga_settings;
    int status;
    //    int tval;
    //int xfrtxflag;
    //    float batvolt;
    //int testmode;
    //unsigned char tval;
    //float symbols_usec;
    //unsigned int synctx_sched[5];
    //unsigned int synclisten_sched[5];

    //	struct tm       * tm;
    //	time_t		current_time;
    static char filename[512];
    //static char keyname[256];
    //static char cmdstr[512];
    static char filestr[64];
    //static char keystr[64];
    char software_version[128];

    int new_pid;
    char idstr[16];
    char ipaddrstr[32];
    //bool mode_load_confirm[2] = {false, false};

    //struct stat stat_buf; /* argument to fstat */
    //	static char syscmd[512];
    //  	 int status = 1
    //	FILE * fdout;
    //	int k;
    //	unsigned char rawdata[512];
    //	unsigned char rawdataarr[10][512];

    //	int subsamp;
    //	double sampper;

    sprintf(software_version, "%s", GIT_VERSION);
#if OM_NAV
    strncat(software_version, "+nav", 4);
#endif

#if OM_NO_EMITTER_POLLING
    strncat(software_version, "+noemit", 7);
#endif

#if OM_WAKE
    strncat(software_version, "+wake", 5);
#endif

#if OM_DMA
    strncat(software_version, "+dma", 5);
#endif

    printf("\n%s, Built %s %s, Version %s\n\n", log_id, __DATE__, __TIME__, software_version);

    /******************************************************************************************
     * Read configuration files
     ******************************************************************************************/
    const std::string rootConfigFile = "om4u.conf";

    om4u::om4uConfiguration om4uConfig;
    om4u::om4uConfigStruct_t *conf = om4uConfig.currentConf;

    om4uConfig.loadDefaultConfigFile(rootConfigFile);
    om4uConfig.loadUserConfigFile(conf->user_config_file);
    printf("\r\n*** End Configuration ***\r\n\n");

    /******************************************************************************************
     * Parse configuration
     ******************************************************************************************/
    DEBUG_LEVEL = conf->debug_level;
    loglevel = conf->debug_level;
    pthread_args.om4u_conf = &om4uConfig;

    // MOA/SOA is used for external messaging
    // MOAA/SOAA is used for oa to oa messaging
    if (conf->network.main_role)
    {
        my_id = MOA;
        my_id2 = MOAA;
        conf->fpga.tdma.master_slaven = 1; // Force for now
    }
    else
    {
        my_id = SOA;
        my_id2 = SOAA;
        conf->fpga.tdma.master_slaven = 0; // Force for now
    }

    om_status.my_id_array[0] = my_id;
    om_status.my_id_array[1] = my_id2;
    om_status.my_id_array_size = 2;
    SHM_WRITE(om_status, SHM_om_status);

    /******************************************************************************************
     * Sensor initialization
     ******************************************************************************************/
    om4uIIOInitialize();

    /******************************************************************************************
     * Network configuration
     ******************************************************************************************/
    initialize_network_comms(conf);

    // initialize_network_comms() modifies SHM_om_status, so re-read it
    SHM_READ(om_status, SHM_om_status);

    stat1hz_sel = conf->network.status_1Hz;

    TERMCOLOR(TERM_MAGENTA);
    DPRINTF(MSG, DEBUG_LEVEL,
            printf("MAIN: Start up role is %s, log level %d\n",
                   ID2PIDstr(my_id), DEBUG_LEVEL));
    TERMCOLOR(TERM_BLACK);

    /******************************************************************************************
     * Initialize tx/rx bit rate and update shared memory
     ******************************************************************************************/
    TERMCOLOR(TERM_MAGENTA);
    DPRINTF(MSG, DEBUG_LEVEL,
            printf("MAIN: Loaded %ld modes\n", conf->fpga.tdma.mode_list.size()));
    TERMCOLOR(TERM_BLACK);

    strcpy(om_status.software_ver, software_version);
    om_status.testmode = 0; // 0=normal, 1=test mode on
    om_status.state = -1;
    om_status.loglevel = conf->debug_level;
    om_status.status1hz_enable = conf->network.status_1Hz;

#ifdef OM_NAV
    if(conf->fpga.bearing.enable)
    {
        conf->fpga.bearing.enable = initializeBearing(conf->navigation.bearing.number_of_strings, conf->navigation.bearing.angle_of_first_string);
        if(!conf->fpga.bearing.enable)
        {
            TERMCOLOR(TERM_RED);
            DPRINTF(ERR, loglevel,
                    printf("BEARING: No emitter count or invalid number provided. Bearing has been disabled.\n"));
            TERMCOLOR(TERM_RED);
        }
    }
#endif

    push_om4u_config_to_om_status(conf, &om_status);

    //symbols_usec = (float)(100.0 / pow(2.0, om_status.phy_tx_symrate));
    SHM_WRITE(om_status, SHM_om_status);

    /******************************************************************************************
     * Send configuration to FPGA
     ******************************************************************************************/

    /*****************************************************************
     * Pointer to the OM4U FPGA register in virtual memory           *
     * Opened here by main()                                         *
     * Accessed by fpga_monitor_thread() and wake_mission_thread() *
     * using om4u_reg_vptr                                           *
     * Reading and writing is protected by om4u_reg_mutex            *
     *****************************************************************/
#ifdef ARM
    // initialize mutex for om4u register reading/writing
    pthread_mutex_init(&om4u_reg_mutex, NULL);
    pthread_args.om4u_reg_mutex_ptr = &om4u_reg_mutex;

    int fd;
    off_t bram_pbase = OMR_REG_ADDR; // physical base address
    if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) != -1)
    {
        if ((pthread_args.om4u_reg_vptr = mmap(NULL, 0x80, PROT_READ | PROT_WRITE, MAP_SHARED, fd, bram_pbase)) == NULL)
        {
            TERMCOLOR(TERM_RED);
            DPRINTF(MSG, DEBUG_LEVEL,
                    printf("MAIN: Failed to open ps/pl resigster\n"));
            TERMCOLOR(TERM_BLACK);
            return (EXIT_FAILURE);
        }
        else
        {
            TERMCOLOR(TERM_MAGENTA);
            DPRINTF(MSG, DEBUG_LEVEL,
                    printf("MAIN: Opened shared ps/pl register @%p\n", pthread_args.om4u_reg_vptr));
            TERMCOLOR(TERM_BLACK);
        }
    }
    else
    {
        TERMCOLOR(TERM_RED);
        DPRINTF(MSG, DEBUG_LEVEL,
                printf("MAIN: Failed to open /dev/mem\n"));
        TERMCOLOR(TERM_BLACK);
        return (EXIT_FAILURE);
    }

    TERMCOLOR(TERM_MAGENTA);
    DPRINTF(MSG, DEBUG_LEVEL,
            printf("MAIN: Send configuration to FPGA\n"));
    TERMCOLOR(TERM_BLACK);

    om_cfg_send(&pthread_args, &om_status);
#else
    TERMCOLOR(TERM_MAGENTA);
    printf("MAIN: Not starting threads. Exiting now\n");
    TERMCOLOR(TERM_BLACK);
    return(EXIT_SUCCESS);
#endif

    /******************************************************************************************
     * initialize the sensor thread attributes
     ******************************************************************************************/
    pthread_attr_init(&DEFAULT_OM_THREAD_ATTR);

    // set for detached threads
    pthread_attr_setdetachstate(&DEFAULT_OM_THREAD_ATTR, PTHREAD_CREATE_DETACHED);
    // read and set the default stack size
    pthread_attr_getstacksize(&DEFAULT_OM_THREAD_ATTR, &pthread_stack_size);
    TERMCOLOR(TERM_MAGENTA);
    DPRINTF(MSG, DEBUG_LEVEL,
            printf("MAIN: Default OM PTHREAD stack size is %ld (0x%08lX)\n", pthread_stack_size, pthread_stack_size));
    TERMCOLOR(TERM_BLACK);

    if (0) // Why do we need this?
    {
        pthread_attr_setstacksize(&DEFAULT_OM_THREAD_ATTR, DEFAULT_OM_THREAD_STACKSIZE);
        pthread_attr_getstacksize(&DEFAULT_OM_THREAD_ATTR, &pthread_stack_size);
        TERMCOLOR(TERM_MAGENTA);
        DPRINTF(MSG, DEBUG_LEVEL,
                printf("MAIN: Setting OM PTHREAD stack size to %ld (0x%08lX)\n", pthread_stack_size, pthread_stack_size));
        TERMCOLOR(TERM_BLACK);
    }

    // initialize condition for synchronization of om4u register reading/writing between threads
    pthread_cond_init(&thread_sync_10Hz, NULL);
    pthread_args.thread_sync_10Hz_ptr = &thread_sync_10Hz;

    pthread_cond_init(&thread_sync_100Hz, NULL);
    pthread_args.thread_sync_100Hz_ptr = &thread_sync_100Hz;

#if OM_NAV
    // initialize mutex for orientation sensor reading, provided by om4u_orientation.c
    pthread_mutex_init(&orientation_mutex, NULL);

    // initialize mutex for navigation shared resource reading/writing
    pthread_mutex_init(&navigation_mutex, NULL);
#endif

    /******************************************************************************************
     * Start threads
     ******************************************************************************************/
    TERMCOLOR(TERM_MAGENTA);
    DPRINTF(MSG, DEBUG_LEVEL,
            printf("MAIN: Start threads "));

    // Create fpga monitor thread; function provided below main()
    status = pthread_create(&omfpga_pthread, &DEFAULT_OM_THREAD_ATTR,
                            fpga_monitor_thread, &pthread_args);
    if (status != 0)
        logtextonly(1, 1, "***Error creating fpga thread\n");
    printf("F");

    // Create receiver threads; function provided by om4u_message_receiver.h
    for (cnt = 0; cnt < om_status.my_id_array_size; cnt++)
    {
        pthread_args.my_id_process_array[cnt] = 0;
        status = pthread_create(&receiver_pthread_array[cnt], &DEFAULT_OM_THREAD_ATTR,
                                message_receiver_thread, &pthread_args);
        if (status != 0)
            logtextonly(1, 1, "***Error creating receiver thread\n");
        printf("%d", cnt+1);
        usleep(10000);
    }

    // Create pairing state tracking thread; function provided by om4u_pairing.h
    status = pthread_create(&pairing_state_tracking_pthread, &DEFAULT_OM_THREAD_ATTR,
                            pairing_state_tracking_thread, &pthread_args);
    if (status != 0)
        logtextonly(1, 1, "***Error creating pairing thread\n");
    printf("P");

#if OM_NO_EMITTER_POLLING
#pragma message "Emitter polling disabled"
#else
    // Create emitter control thread; function provided by om4u_emitter.h
    status = pthread_create(&emitter_control_pthread, &DEFAULT_OM_THREAD_ATTR,
                            emitter_control_thread, &pthread_args);
    if (status != 0)
        logtextonly(1, 1, "***Error creating emitter thread\n");
    printf("E");
#endif

#if OM_NAV
#pragma message "Navigation functions enabled"
    // Create navigation thread; function provided below main()
    status = pthread_create(&navigation_listen_pthread, &DEFAULT_OM_THREAD_ATTR,
                            navigation_listen_thread, &pthread_args);
    if (status != 0)
        logtextonly(1, 1, "***Error creating navigation thread\n");
    printf("N");

    // Create orientation update thread; function provided by om4u_orientation.h
    status = pthread_create(&orientation_pthread, &DEFAULT_OM_THREAD_ATTR,
                            update_orientation_thread, &pthread_args);
    if (status != 0)
        logtextonly(1, 1, "***Error creating orientation thread\n");
    printf("O");
#endif

#if OM_WAKE
#pragma message "Wake enabled"
    // Create wake mission thread
    if (my_id == MOA) // OM_MASTER
    {
        status = pthread_create(&wake_mission_pthread, &DEFAULT_OM_THREAD_ATTR,
                                wake_mission_payload_thread, &pthread_args);
    }
    else
    {
        status = pthread_create(&wake_mission_pthread, &DEFAULT_OM_THREAD_ATTR,
                                wake_mission_cairn_thread, &pthread_args);
    }

    if (status != 0)
        logtextonly(1, 1, "***Error creating wake mission thread\n");
    printf("W");
#endif

#if OM_DMA
#pragma message "DMA capture enabled"

    status = pthread_create(&dma_capture_pthread, &DEFAULT_OM_THREAD_ATTR,
                            dma_capture_thread, &pthread_args);

    if (status != 0)
        logtextonly(1, 1, "***Error creating dma capture thread\n");
    printf("D");
#endif


    //	status = pthread_create (&ipdet_pthread, &DEFAULT_OM_THREAD_ATTR,
    //	                         ip_detect_thread, (void *)&net_address);
    //	if (status != 0) logtextonly(1,1,"***Error ip_detect thread\n");
    //-----------------------------------------------------------------
    printf("\n");
    TERMCOLOR(TERM_BLACK);

    load_navid(conf, &nav_info);

    SHM_LOCK(SHM_nav_info);
    sprintf(SHM_nav_info.data.identifier, "%s", nav_info.identifier);
    SHM_nav_info.data.latitude = nav_info.latitude;
    SHM_nav_info.data.longitude = nav_info.longitude;
    SHM_nav_info.data.depth = nav_info.depth;
    SHM_UNLOCK(SHM_nav_info);

    /******************************************************************************************
     * Command/control interface
     ******************************************************************************************/
    while (!done)
    {
        // TERMCOLOR(TERM_MAGENTA);
        // DPRINTF(MSG, DEBUG_LEVEL, printf("MAIN: my_id_array size %d\n", om_status.my_id_array_size));
        // TERMCOLOR(TERM_BLACK);

        SHM_LOCK(SHM_om_status)
        my_id = om_status.my_id_array[0];
        my_id2 = om_status.my_id_array[1];
        SHM_UNLOCK(SHM_om_status)

        // Blocking call to pop oldest message
        message_wait_and_pop(&msg);

        // forward the message to other modem, if we are not the intended receiver
        if (!(msg.hdr.dest == my_id || msg.hdr.dest == my_id2))
        {
            DPRINTF(DBG1, DEBUG_LEVEL, printf("main(): Forwarding message to %s\n", ID2PIDstr(msg.hdr.dest)));
            send_msg(msg.hdr.dest, my_id2, msg.hdr.msg_type, strlen(msg.body), msg.body);
        }
        else
        {
            switch (msg.hdr.msg_type)
            {
            case ACK:
                break;

            case NACK:
                break;

            case PING:
                get_timestamp(timestamp);
                sprintf(msg_out, "%s ping received from %s at %s",
                        ID2PIDstr(my_id), ID2PIDstr(msg.hdr.src), timestamp);
                send_msg(msg.hdr.src, my_id, PING_RET, strlen(msg_out), msg_out);
                break;

            case PING_RET:
                get_timestamp(timestamp);
                if (msg.hdr.src == my_id)
                {
                    TERMCOLOR(TERM_RED);
                    DPRINTF(MSG, DEBUG_LEVEL, printf("MAIN: PING Return from %s, OM STATE =4", log_id));
                    TERMCOLOR(TERM_BLACK);
                    SHM_LOCK(SHM_om_status);
                    SHM_om_status.data.state = 4;
                    SHM_UNLOCK(SHM_om_status);
                }
                break;

            case QUIT:
                done = 1;
                TERMCOLOR(TERM_RED);
                DPRINTF(DBG2, DEBUG_LEVEL, printf("MAIN: QUIT received\n"));
                TERMCOLOR(TERM_BLACK);
                break;

            case OM_TXRTREQ:
                get_timestamp(timestamp);

                TERMCOLOR(TERM_RED);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: %s not implemented\n", MsgID2Str(msg.hdr.msg_type)));
                TERMCOLOR(TERM_BLACK);
                sprintf(msg_out, "%s not implemented", MsgID2Str(msg.hdr.msg_type));
                send_msg(msg.hdr.src, my_id, ERROR_MSG, strlen(msg_out), msg_out);
                // phy_tx_symrate = (u32)atoi(msg.body);
                ////                 slogprint(1,1,msg_out,"%s txrtreq received at %s",ID2PIDstr(my_id),timestamp);
                // sprintf(msg_out, "%u", phy_tx_symrate);
                // send_msg(msg.hdr.src, my_id, OM_TXRTCHNG, strlen(msg_out), msg_out);

                //// update shared-mem
                // SHM_LOCK(SHM_om_status);
                // SHM_om_status.data.txbitrate = phy_tx_symrate;
                ////                SHM_om_status.data.txclkfreq = phy_tx_symrate;
                // SHM_UNLOCK(SHM_om_status);
                // SHM_READ(om_status, SHM_om_status);
                // printf("om_cfg_send from OM_TXRTREQ\n");

                // om_cfg_send(&om_status, &pthread_args);

                break;

            case OM_RXRTREQ:
                get_timestamp(timestamp);
                TERMCOLOR(TERM_RED);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: %s not implemented\n", MsgID2Str(msg.hdr.msg_type)));
                TERMCOLOR(TERM_BLACK);
                sprintf(msg_out, "%s not implemented", MsgID2Str(msg.hdr.msg_type));
                send_msg(msg.hdr.src, my_id, ERROR_MSG, strlen(msg_out), msg_out);

                ////                      sprintf(msg_out,"%s rxrtreq received at %s",ID2PIDstr(my_id),timestamp);
                // phy_rx_symrate = (u32)atoi(msg.body);

                // sprintf(msg_out, "%u", phy_rx_symrate);
                // send_msg(msg.hdr.src, my_id, OM_RXRTCHNG, strlen(msg_out), msg_out);

                //// update shared-mem
                // SHM_LOCK(SHM_om_status);
                // SHM_om_status.data.rxbitrate = phy_rx_symrate;
                // SHM_UNLOCK(SHM_om_status);
                // SHM_READ(om_status, SHM_om_status);
                // printf("From OM_RXRTREQ om_cfg_send1\n");

                ////om_cfg_send(&net_address, &om_status);

                break;

            case OM_TXCARREQ:
                get_timestamp(timestamp);

                TERMCOLOR(TERM_RED);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: %s not implemented\n", MsgID2Str(msg.hdr.msg_type)));
                TERMCOLOR(TERM_BLACK);
                sprintf(msg_out, "%s not implemented", MsgID2Str(msg.hdr.msg_type));
                send_msg(msg.hdr.src, my_id, ERROR_MSG, strlen(msg_out), msg_out);
                // phy_tx_symrate = (u32)atoi(msg.body);
                ////                 sprintf(msg_out,"%s txrtreq received at %s",ID2PIDstr(my_id),timestamp);
                // sprintf(msg_out, "%u", phy_tx_symrate);
                // send_msg(msg.hdr.src, my_id, OM_TXRTCHNG, strlen(msg_out), msg_out);

                // update shared-mem
                //                SHM_LOCK(SHM_om_status);
                //                SHM_om_status.data.txbitrate = phy_tx_symrate;
                //                SHM_UNLOCK(SHM_om_status);

                break;

            case OM_RXKICK:
                get_timestamp(timestamp);
                TERMCOLOR(TERM_RED);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: %s not implemented\n", MsgID2Str(msg.hdr.msg_type)));
                TERMCOLOR(TERM_BLACK);
                sprintf(msg_out, "%s not implemented", MsgID2Str(msg.hdr.msg_type));
                send_msg(msg.hdr.src, my_id, ERROR_MSG, strlen(msg_out), msg_out);

                ////                      sprintf(msg_out,"%s rxkick received at %s",ID2PIDstr(my_id),timestamp);

                ////                sprintf (msg_out, "kickme");
                ////                SHM_READ(om_status,SHM_om_status);
                ////                if (!om_status.testmode)
                ////                {
                ////                    com_rx_kick (&scfg);
                ////                }
                ////                else
                // logtextonly(1, 1, "KICK Received - No OP\n");
                // SHM_READ(om_status, SHM_om_status);
                // om_status.reset_bit = 1;
                ////					printf("From OM_RXKICK om_cfg_send1\n")
                // om_cfg_send(&om_status, &pthread_args);
                // om_status.reset_bit = 0;

                // logtextonly(1, 1, "Sending fpga reset signal\n");

                // update shared-mem
                //                        SHM_LOCK(SHM_om_status);
                //                        SHM_om_status.data.rxbitrate = phy_rx_symrate;
                //                        SHM_UNLOCK(SHM_om_status);

                break;

            case OM_TEST_MODE:
                get_timestamp(timestamp);
                TERMCOLOR(TERM_RED);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: %s not implemented\n", MsgID2Str(msg.hdr.msg_type)));
                TERMCOLOR(TERM_BLACK);
                sprintf(msg_out, "%s not implemented", MsgID2Str(msg.hdr.msg_type));
                send_msg(msg.hdr.src, my_id, ERROR_MSG, strlen(msg_out), msg_out);

                // testmode = atoi(msg.body);

                // if (testmode == 1)
                //{
                //     com_test_config(&scfg, 1);
                //     SHM_LOCK(SHM_om_status);
                //     SHM_om_status.data.testmode = 1;
                //     SHM_UNLOCK(SHM_om_status);
                //     printf("Test mode = 1\n");
                // }
                // else if (testmode == 0)
                //{
                //     com_test_config(&scfg, 0);
                //     SHM_LOCK(SHM_om_status);
                //     SHM_om_status.data.testmode = 0;
                //     SHM_UNLOCK(SHM_om_status);
                //     printf("Test mode = 0\n");
                // }
                // else
                //     printf("Received bad command\n");

                break;

            case OM_MODEREQ:
                get_timestamp(timestamp);
                new_mode = atoi(msg.body);

                TERMCOLOR(TERM_RED);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: OM_MODEREQ received, new mode %d\n", new_mode));
                TERMCOLOR(TERM_BLACK);

                mode_el = om4u::findModeById(conf->fpga.tdma.mode_list, new_mode, conf->fpga.tdma.master_slaven);

                if (mode_el != NULL)
                {
                    sprintf(msg_out, "%02d", new_mode);
                    send_msg(msg.hdr.src, my_id, OM_MODECHNG, strlen(msg_out), msg_out);

                    conf->fpga.tdma.default_mode = new_mode;
                    apply_om4u_config_to_fpga(&pthread_args);

                    //modereq_time = get_utime();

                    // Send the entire settings back to WEB to reflect current config state correctly
                    *settings_str = 0;
                    create_om_settings_mesg(&om4uConfig, settings_str);
                    send_msg(WEB, om_status.my_id_array[0], OM_SETTINGS, strlen(settings_str), settings_str);
                }
                else
                {
                    sprintf(msg_out, "Invalid mode %02d", new_mode);
                    send_msg(msg.hdr.src, my_id, ERROR_MSG, strlen(msg_out), msg_out);
                    new_mode = -1;
                }

                break;

            case OM_MODECONFIRM:
                get_timestamp(timestamp);
                TERMCOLOR(TERM_RED);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: %s not implemented\n", MsgID2Str(msg.hdr.msg_type)));
                TERMCOLOR(TERM_BLACK);
                sprintf(msg_out, "%s not implemented", MsgID2Str(msg.hdr.msg_type));
                send_msg(msg.hdr.src, my_id, ERROR_MSG, strlen(msg_out), msg_out);

                // confirm_mode = -2;
                // confirm_mode = atoi(msg.body);
                //                  sprintf(msg_out,"%s txrtreq received at %s",ID2PIDstr(my_id),timestamp);
                // sprintf(msg_out, "%d", confirm_mode);
                // if (confirm_mode == new_mode)
                //{
                //     if ((modereq_time + 15.0 - get_utime()) > 0)
                //     {

                //        mode_ptr = modes_arr;
                //        for (cnt = 0; cnt < mode_cnt; cnt++)
                //        {
                //            if ((mode_ptr->mode == confirm_mode) && (mode_ptr->master_slaven == net_address.master_slaven))
                //            {
                //                logprint(1, 1, "Set to mode %d  %s, mode_cnt=%d\n", confirm_mode, modes_arr[cnt].mode_label, cnt);
                //                om_status.rx_fec_mode = modes_arr[cnt].rx_fec_mode;
                //                om_status.tx_fec_mode = modes_arr[cnt].tx_fec_mode;
                //                om_status.rx_prbs11 = modes_arr[cnt].rx_prbs11;
                //                om_status.ber_en = modes_arr[cnt].ber_en;
                //                om_status.cont_burstn = modes_arr[cnt].cont_burstn;
                //                om_status.master_slaven = modes_arr[cnt].master_slaven;
                //                om_status.sf_period = modes_arr[cnt].sf_period;
                //                om_status.tx_start = modes_arr[cnt].tx_start;
                //                om_status.tx_end = modes_arr[cnt].tx_end;
                //                om_status.rx_start = modes_arr[cnt].rx_start;
                //                om_status.rx_end = modes_arr[cnt].rx_end;
                //                om_status.phy_agc_response = modes_arr[cnt].phy_agc_response;
                //                om_status.phy_preamble = modes_arr[cnt].phy_preamble;
                //                om_status.phy_tx_symrate = modes_arr[cnt].phy_tx_symrate;
                //                om_status.phy_rx_symrate = modes_arr[cnt].phy_rx_symrate;
                //                om_status.mode = confirm_mode;
                //                om_status.selftest_mode = 0;
                //                SHM_WRITE(om_status, SHM_om_status);
                //                break;
                //            }
                //            else
                //            {
                //                logprint(1, 1, "No match mode_cnt = %d\n", cnt);
                //            }
                //            mode_ptr = &modes_arr[cnt + 1];
                //        }

                //        printf("om_cfg_send from OM_MODECONFIRM\n");

                //        om_cfg_send(&om_status, &pthread_args);
                //    }
                //    else
                //    {
                //        logtextonly(1, 1, "Too long since om_modereq\n");
                //    }
                //}
                // else
                //    logtextonly(1, 1, "Confirmation does not match\n");
                break;

            case OM_MODELIST:
                get_timestamp(timestamp);
                TERMCOLOR(TERM_RED);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: OM_MODELIST requested\n"));
                TERMCOLOR(TERM_BLACK);

                create_mode_message(conf, msg_out);

                send_msg(msg.hdr.src, my_id, OM_MODES, strlen(msg_out), msg_out);

                break;

            case OM_MODES:
                // Re-broadcast modes info from other modem to the web interface
                send_msg(WEB, msg.hdr.src, msg.hdr.msg_type, strlen(msg.body), msg.body);
                break;

            case OM_POWER:
                TERMCOLOR(TERM_RED);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: %s not implemented\n", MsgID2Str(msg.hdr.msg_type)));
                TERMCOLOR(TERM_BLACK);
                sprintf(msg_out, "%s not implemented", MsgID2Str(msg.hdr.msg_type));
                send_msg(msg.hdr.src, my_id, ERROR_MSG, strlen(msg_out), msg_out);

                // printf("Received Power command %s\n", msg.body);

                ////				  printf("Line = %s\n",line);
                // pval = 0;
                // if ((ret = sscanf(msg.body, "%s %d",
                //                 paramstr, &pval) == 2))
                //{
                //     if (strcmp(paramstr, "EXTPWR") == 0)
                //     {
                //         if (pval)
                //         {
                //             printf("Turning external power on");
                //             system("/root/extpwr.sh on");
                //         }
                //         else
                //         {
                //             printf("Turning external power off");
                //             system("/root/extpwr.sh off");
                //         }
                //     }

                //    else if (strcmp(paramstr, "FPGAPWR") == 0)
                //    {
                //        if (pval)
                //        {
                //            printf("Turning fpga power on");
                //            system("/root/fpgapwr.sh on");
                //        }
                //        else
                //        {
                //            printf("Turning fpga power off");
                //            system("/root/fpgapwr.sh off");
                //        }
                //    }
                //}
                break;

            case OM_EMITSET:
                get_timestamp(timestamp);
                DPRINTF(MSG, DEBUG_LEVEL, printf("%s\n", msg.body));
                //emitsetok = 0;
                if (!(strncmp(msg.body, "#TR", 3)))
                {
                    emitter_command_push(msg.body);
                }
                break;

            case OM_EMITTMP: // FIXME so much kludge; we have to pass the src id through the command
                get_timestamp(timestamp);
                sprintf(msg_out, "%s", msg.body);
                send_msg(MXA, my_id, OM_EMITTMP, strlen(msg_out), msg_out);

                sprintf(msg_out, "#TR01T1?,%d", msg.hdr.src);
                emitter_command_push(msg_out);
                break;

            case OM_EMITSTAT:
                sprintf(msg_out, "RRRRRR");
                DPRINTF(MSG, DEBUG_LEVEL, printf("OM_EMITSTAT\n"));
                emitter_command_push(msg_out);
                break;

            case OM_SOAEMIT:
            case OM_MOAEMIT:
                // Re-broadcast emitter info from other modem to the web interface
                send_msg(WEB, my_id, msg.hdr.msg_type, strlen(msg.body), msg.body);
                break;

            case GUI_ADDRESS:
                get_timestamp(timestamp);
                logprint(1, 1, "Setting GUI address to %s\n", msg.body);
                updateNetworkMapIPAddress(conf->network.user_map, ID2PIDstr(GUI), msg.body);
                break;

            case APP_ADDRESS:
                get_timestamp(timestamp);
                new_mode = -1;
                ret = sscanf(msg.body, "%s %s %d", idstr, ipaddrstr, &new_mode);
                new_pid = PIDstr2ID(idstr);

                if ((ret == 2 || ret == 3) && (new_pid == GUI || new_pid == LIST1 || new_pid == LIST2))
                {
                    sprintf(msg_out, "Setting");
                    if (ret == 3)
                    {
                        new_mode = (new_mode > 0 ? 1 : 0);
                        switch (new_pid)
                        {
                        case GUI:
                            conf->network.status_1Hz = (conf->network.status_1Hz & 0x6) | new_mode;
                            break;
                        case LIST1:
                            conf->network.status_1Hz = (conf->network.status_1Hz & 0x5) | (new_mode << 1);
                            break;
                        case LIST2:
                            conf->network.status_1Hz = (conf->network.status_1Hz & 0x3) | (new_mode << 2);
                            break;
                        }
                        SHM_LOCK(SHM_om_status)
                        SHM_om_status.data.status1hz_enable = conf->network.status_1Hz;
                        SHM_UNLOCK(SHM_om_status)

                        if (new_mode)
                            sprintf(msg_out, "Enabling");
                        else
                            sprintf(msg_out, "Disabling");

                        // Send the entire settings back to WEB to reflect current config state correctly
                        *settings_str = 0;
                        create_om_settings_mesg(&om4uConfig, settings_str);
                        send_msg(WEB, om_status.my_id_array[0], OM_SETTINGS, strlen(settings_str), settings_str);
                    }

                    TERMCOLOR(TERM_MAGENTA);
                    DPRINTF(MSG, DEBUG_LEVEL,
                            printf("MAIN: %s app id string %s to address %s\n",
                                   msg_out, ID2PIDstr(new_pid), ipaddrstr));
                    TERMCOLOR(TERM_BLACK);

                    updateNetworkMapIPAddress(conf->network.user_map, idstr, ipaddrstr);
                }
                else
                {
                    TERMCOLOR(TERM_MAGENTA);
                    DPRINTF(ERR, DEBUG_LEVEL, printf("MAIN: Invalid app address id string %s\n", idstr));
                    TERMCOLOR(TERM_BLACK);
                }
                break;

            case OM_STATUSREQ:

                SHM_LOCK(SHM_om_status);
                strcpy(msg_out, SHM_link_status.data.link_msg);
                SHM_UNLOCK(SHM_om_status);

                send_msg(msg.hdr.src, my_id, OM_STATUS, strlen(msg_out), msg_out);
                break;

            case OM_CONFIG:
                get_timestamp(timestamp);
                TERMCOLOR(TERM_RED);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: %s not implemented\n", MsgID2Str(msg.hdr.msg_type)));
                TERMCOLOR(TERM_BLACK);
                sprintf(msg_out, "%s not implemented", MsgID2Str(msg.hdr.msg_type));
                send_msg(msg.hdr.src, my_id, ERROR_MSG, strlen(msg_out), msg_out);

                // printf("Read config packets\n");
                // load_om_param(msg.body, &om_status);
                // SHM_WRITE(om_status, SHM_om_status);
                ////					printf("From OM_CONFIG om_cfg_send1\n");
                // om_cfg_send(&om_status, &pthread_args);
                break;

            case OM_RESET:
                get_timestamp(timestamp);

                TERMCOLOR(TERM_MAGENTA);
                DPRINTF(MSG, DEBUG_LEVEL, printf("MAIN: Rebooting system in 5 seconds\n"));
                TERMCOLOR(TERM_BLACK);
                sleep(5);
                system("/sbin/reboot");
                // // FIXME Change this hijack back to reboot after Bluelight
                // logtextonly(1,1,"MAIN: Reseting FPGA\n");
                // //om_fpga_reset(NULL, &om_status);

                // com_pkt_read(&scfg, err_data)
                break;

            case OM_SETREQ:
                get_timestamp(timestamp);
                TERMCOLOR(TERM_MAGENTA);
                DPRINTF(MSG, DEBUG_LEVEL, printf("MAIN: Received OM_SETREQ\n"));
                TERMCOLOR(TERM_BLACK);

                *settings_str = 0;

                create_om_settings_mesg(&om4uConfig, settings_str);

                TERMCOLOR(TERM_MAGENTA);
                DPRINTF(MSG, DEBUG_LEVEL, printf("MAIN: Sending settings\n"));
                TERMCOLOR(TERM_BLACK);
                send_msg(msg.hdr.src, my_id, OM_SETTINGS, strlen(settings_str), settings_str);

                break;

            case OM_SETPARAMS:
                get_timestamp(timestamp);

                TERMCOLOR(TERM_MAGENTA);
                DPRINTF(MSG, DEBUG_LEVEL, printf("MAIN: Parse contents of OM_SETPARAMS\n"));
                TERMCOLOR(TERM_BLACK);

                if (parse_om_settings_mesg(&pthread_args, msg.body))
                {
                    TERMCOLOR(TERM_MAGENTA);
                    DPRINTF(MSG, DEBUG_LEVEL, printf("MAIN: Parse contents successful\n"));
                    TERMCOLOR(TERM_BLACK);

                    // Update local om_status after parse_om_settings_mesg()
                    // may have updated shared memory
                    SHM_READ(om_status, SHM_om_status);

                    *settings_str = 0;
                    create_om_settings_mesg(&om4uConfig, settings_str);

                    // Send the new settings back to the src and to WEB also
                    send_msg(msg.hdr.src, om_status.my_id_array[0], OM_SETTINGS, strlen(settings_str), settings_str);
                    send_msg(WEB, om_status.my_id_array[0], OM_SETTINGS, strlen(settings_str), settings_str);
                }
                break;

            case OM_SETTINGS:
                // If we have received an OM_SETTINGS message, forward it to the WEB interface
                send_msg(WEB, msg.hdr.src, msg.hdr.msg_type, strlen(msg.body), msg.body);
                break;

            case OM_SETTINGS_SAVE_REQ:
                TERMCOLOR(TERM_MAGENTA);
                DPRINTF(MSG, DEBUG_LEVEL, printf("MAIN: Received OM_SETTINGS_SAVE_REQ\n"));
                TERMCOLOR(TERM_BLACK);

                // Push changes to the user config file
                ret = om4uConfig.updateUserConfigFile(conf->user_config_file);
                if (ret)
                {
                    sprintf(msg_out, "1 Settings successfully saved");
                }
                else
                {
                    sprintf(msg_out, "0 ERROR Settings save unsuccessful");
                }
                TERMCOLOR(TERM_MAGENTA);
                DPRINTF(MSG, DEBUG_LEVEL, printf("MAIN: %s\n", msg_out));
                TERMCOLOR(TERM_BLACK);

                send_msg(msg.hdr.src, my_id, OM_SETTINGS_SAVE_RESP, strlen(msg_out), msg_out);

                if (ret)
                {
                    // Re-load the user config file
                    om4uConfig.loadUserConfigFile(conf->user_config_file);

                    // Send OM_SETTINGS
                    *settings_str = 0;
                    create_om_settings_mesg(&om4uConfig, settings_str);
                    send_msg(msg.hdr.src, my_id, OM_SETTINGS, strlen(settings_str), settings_str);
                }
                break;

            case OM_SETTINGS_SAVE_RESP:
                // If we have received an OM_SETTINGS_SAVE_RESP message, forward it to the WEB interface
                send_msg(WEB, msg.hdr.src, msg.hdr.msg_type, strlen(msg.body), msg.body);
                break;

            case SELFTEST:
                get_timestamp(timestamp);
                TERMCOLOR(TERM_RED);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: %s not implemented\n", MsgID2Str(msg.hdr.msg_type)));
                TERMCOLOR(TERM_BLACK);
                sprintf(msg_out, "%s not implemented", MsgID2Str(msg.hdr.msg_type));
                send_msg(msg.hdr.src, my_id, ERROR_MSG, strlen(msg_out), msg_out);
                // selftest_mode = -2;
                // selftest_mode = atoi(msg.body);
                ////                 sprintf(msg_out,"%s txrtreq received at %s",ID2PIDstr(my_id),timestamp);
                // sprintf(msg_out, "%d", selftest_mode);
                // printf("Selftest_mode = %d\n", selftest_mode);

                // mode_ptr = modes_arr;
                // for (cnt = 0; cnt < mode_cnt; cnt++)
                //{
                //     if ((mode_ptr->mode == selftest_mode) && (mode_ptr->master_slaven == 0))
                //     {
                //         logprint(1, 1, "Set selftest for mode %d  %s, mode_cnt=%d\n", selftest_mode, modes_arr[cnt].mode_label, cnt);
                //         om_status.rx_fec_mode = modes_arr[cnt].rx_fec_mode;
                //         om_status.tx_fec_mode = modes_arr[cnt].tx_fec_mode;
                //         om_status.rx_prbs11 = modes_arr[cnt].rx_prbs11;
                //         om_status.ber_en = modes_arr[cnt].ber_en;
                //         om_status.cont_burstn = modes_arr[cnt].cont_burstn;
                //         om_status.master_slaven = modes_arr[cnt].master_slaven;
                //         om_status.sf_period = modes_arr[cnt].sf_period;
                //         //								om_status.tx_start=modes_arr[cnt].tx_start;
                //         //								om_status.tx_end=modes_arr[cnt].tx_end;
                //         om_status.rx_start = modes_arr[cnt].rx_start;
                //         om_status.rx_end = modes_arr[cnt].rx_end;
                //         om_status.phy_agc_response = modes_arr[cnt].phy_agc_response;
                //         om_status.phy_preamble = modes_arr[cnt].phy_preamble;
                //         om_status.phy_tx_symrate = modes_arr[cnt].phy_tx_symrate;
                //         om_status.phy_rx_symrate = modes_arr[cnt].phy_rx_symrate;
                //         //						om_status.selftest_mode=1;
                //         om_status.selftest_en = 1;

                //        om_status.mode = selftest_mode;
                //        //								SHM_WRITE(om_status,SHM_om_status);
                //        //								break;
                //    }
                //    if ((mode_ptr->mode == selftest_mode) && (mode_ptr->master_slaven == 1))
                //    {
                //        logprint(1, 1, "Set selftest for mode %d  %s, mode_cnt=%d\n", selftest_mode, modes_arr[cnt].mode_label, cnt);
                //        //om_status.rx_fec_mode=modes_arr[cnt].rx_fec_mode;
                //        //om_status.tx_fec_mode=modes_arr[cnt].tx_fec_mode;
                //        //om_status.rx_prbs11=modes_arr[cnt].rx_prbs11;
                //        //om_status.ber_en=modes_arr[cnt].ber_en;
                //        //om_status.cont_burstn=modes_arr[cnt].cont_burstn;
                //        //om_status.master_slaven=modes_arr[cnt].master_slaven;
                //        //om_status.sf_period=modes_arr[cnt].sf_period;
                //        om_status.tx_start = modes_arr[cnt].tx_start;
                //        om_status.tx_end = modes_arr[cnt].tx_end;
                //        //om_status.rx_start=modes_arr[cnt].rx_start;
                //        //om_status.rx_end=modes_arr[cnt].rx_end;
                //        //om_status.phy_agc_response=modes_arr[cnt].phy_agc_response;
                //        //om_status.phy_preamble=modes_arr[cnt].phy_preamble;
                //        //om_status.phy_tx_symrate=modes_arr[cnt].phy_tx_symrate;
                //        //om_status.phy_rx_symrate=modes_arr[cnt].phy_rx_symrate;

                //        //						om_status.mode = selftest_mode;
                //        om_status.selftest_en = 1;
                //        //  do not write to permanent memory
                //        //			SHM_WRITE(om_status,SHM_om_status);
                //        break;
                //    }
                //    else
                //    {
                //        logprint(1, 1, "No match mode_cnt = %d\n", cnt);
                //    }
                //    mode_ptr = &modes_arr[cnt + 1];
                //}

                // printf("om_cfg_send from SELFTest\n");

                // om_cfg_send(&om_status, &pthread_args);

                break;


            case VERSIONREQ:
                get_timestamp(timestamp);
                sprintf(msg_out, "Arm software version - %s", software_version);
                TERMCOLOR(TERM_MAGENTA);
                DPRINTF(MSG, DEBUG_LEVEL, printf("MAIN: %s\n", msg_out));
                TERMCOLOR(TERM_BLACK);
                send_msg(msg.hdr.src, my_id, VERSION, strlen(msg_out), msg_out);
                break;

            case CMDEXEC:
                get_timestamp(timestamp);
                TERMCOLOR(TERM_RED);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: %s not implemented\n", MsgID2Str(msg.hdr.msg_type)));
                TERMCOLOR(TERM_BLACK);
                sprintf(msg_out, "%s not implemented", MsgID2Str(msg.hdr.msg_type));
                send_msg(msg.hdr.src, my_id, ERROR_MSG, strlen(msg_out), msg_out);

                // sprintf(filename, "/root/%s", msg.body);
                // if (!access(filename, F_OK))
                //{
                //     logprint(1, 1, "Running %s,\n", filename);
                //     system(filename);
                // }
                // else
                //     logprint(1, 1, "%s not found\n", filename);

                break;

            case CMDFWD:
                get_timestamp(timestamp);
                TERMCOLOR(TERM_RED);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: %s not implemented\n", MsgID2Str(msg.hdr.msg_type)));
                TERMCOLOR(TERM_BLACK);
                sprintf(msg_out, "%s not implemented", MsgID2Str(msg.hdr.msg_type));
                send_msg(msg.hdr.src, my_id, ERROR_MSG, strlen(msg_out), msg_out);

                // logprint(1, 1, "Forwarding %s,\n", msg.body);
                // send_msg(SOA, my_id, CMDEXEC, strlen(msg.body), msg.body);

                break;

            case REMUS_DATA:
                get_timestamp(timestamp);

                logprint(1, 1, "Remus Data =  %s,\n", msg.body);
                //				send_msg (SOA, my_id, CMDEXEC, strlen (msg.body), msg.body);

                break;

            case OM_BANDWIDTH_REQ:
                get_timestamp(timestamp);

                sprintf(msg_out, "%d %d", (int)(om_status.txratecalc / 1000), (int)(om_status.rxratecalc / 1000));

                logprint(1, 1, "Bandwidth Request %s\n", msg_out);
                send_msg(msg.hdr.src, my_id, OM_BANDWIDTH, strlen(msg_out), msg_out);

                break;

            case OM_UPDATE:
                get_timestamp(timestamp);
                TERMCOLOR(TERM_MAGENTA);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: OM_UPDATE msg.body %s\n", msg.body));
                TERMCOLOR(TERM_BLACK);
                if ((ret = sscanf(msg.body, "%s", filestr) == 1))
                {
                    sprintf(filename, "%s/%s", omuser_home_dir, filestr);
                    if (!access(filename, F_OK))
                    {
                        /* File exists, run update */
                        TERMCOLOR(TERM_MAGENTA);
                        DPRINTF(MSG, DEBUG_LEVEL,
                                printf("MAIN: OM_UPDATE Updating with %s\n", filestr));
                        TERMCOLOR(TERM_BLACK);

                        update_arm_code(msg.hdr.src, msg.hdr.dest, filename);
                    }
                    else
                    {
                        /* Update file does not exist */
                        TERMCOLOR(TERM_MAGENTA);
                        DPRINTF(ERR, DEBUG_LEVEL,
                                printf("MAIN: OM_UPDATE Update file \"%s\" not found\n", filestr));
                        TERMCOLOR(TERM_BLACK);
                    }
                }
                else
                {
                    /* Wrong argument/usage statement */
                    TERMCOLOR(TERM_MAGENTA);
                    DPRINTF(ERR, DEBUG_LEVEL,
                            printf("MAIN: OM_UPDATE Wrong number of arguments for OM_UPDATE [filename]\n"));
                    TERMCOLOR(TERM_BLACK);
                }
                break;

            case WEB_UPDATE_REQ:
                get_timestamp(timestamp);
                TERMCOLOR(TERM_MAGENTA);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: WEB_UPDATE msg.body %s\n", msg.body));
                TERMCOLOR(TERM_BLACK);
                if ((ret = sscanf(msg.body, "%s", filestr) == 1))
                {
                    sprintf(filename, "%s/%s", web_home_dir, filestr);
                    if (!access(filename, F_OK))
                    {
                        /* File exists, run update */
                        TERMCOLOR(TERM_MAGENTA);
                        DPRINTF(MSG, DEBUG_LEVEL,
                                printf("MAIN: WEB_UPDATE Updating with %s\n", filestr));
                        TERMCOLOR(TERM_BLACK);

                        sprintf(msg_out, "Updating with %s", filestr);
                        send_msg(msg.hdr.src, my_id, WEB_UPDATE_RSP, strlen(msg_out), msg_out);
                        sleep(1);

                        update_arm_code(msg.hdr.src, msg.hdr.dest, filename);
                    }
                    else
                    {
                        /* Update file does not exist */
                        TERMCOLOR(TERM_MAGENTA);
                        DPRINTF(ERR, DEBUG_LEVEL,
                                printf("MAIN: WEB_UPDATE Update file \"%s\" not found\n", filestr));
                        TERMCOLOR(TERM_BLACK);
                        sprintf(msg_out, "Updating file not found \"%s\" provided in WEB_UPDATE_REQ", filestr);
                        send_msg(msg.hdr.src, my_id, WEB_UPDATE_RSP, strlen(msg_out), msg_out);
                    }
                }
                else
                {
                    /* Wrong argument/usage statement */
                    TERMCOLOR(TERM_MAGENTA);
                    DPRINTF(ERR, DEBUG_LEVEL,
                            printf("MAIN: WEB_UPDATE Wrong number of arguments for WEB_UPDATE_REQ [filename]\n"));
                    TERMCOLOR(TERM_BLACK);
                    sprintf(msg_out, "Error: Wrong number of arguments; usage: WEB_UPDATE_REQ [filename]");
                    send_msg(msg.hdr.src, my_id, WEB_UPDATE_RSP, strlen(msg_out), msg_out);
                }
                break;

            case WEB_UPDATE_RSP:
            case UPDATE_STATUS:
                // If we have received an WEB_UPDATE_RSP or UPDATE_STATUS message, forward it to the WEB interface
                send_msg(WEB, msg.hdr.src, msg.hdr.msg_type, strlen(msg.body), msg.body);
                break;
                ;

            case OM_FACT_RESET:
                get_timestamp(timestamp);
                TERMCOLOR(TERM_RED);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: %s not implemented\n", MsgID2Str(msg.hdr.msg_type)));
                TERMCOLOR(TERM_BLACK);
                sprintf(msg_out, "%s not implemented", MsgID2Str(msg.hdr.msg_type));
                send_msg(msg.hdr.src, my_id, ERROR_MSG, strlen(msg_out), msg_out);

                // TODO Change this to simply delete om4u_user.conf
                // pval = 0;
                // if ((ret = sscanf(msg.body, "%s",
                //                paramstr) == 1))
                //{
                //    if (strcmp(paramstr, "MASTER") == 0)
                //    {

                //        logtextonly(1, 1, "\n***\n***\nResetting to master configuration, IP =192.168.2.50\n");
                //        //						retval =system("/root/transfer_10MB.sh &");
                //        retval = system("/root/master_config.sh");
                //        //						logprint(1,1,"Running nohup /root/transfer_10MB.sh &\n");
                //        //						retval =system("/bin/touch /data/t10");
                //        logprint(1, 1, "retval = %d\n***\n***\n", retval);
                //    }

                //    else if (strcmp(paramstr, "SLAVE") == 0)
                //    {

                //        logtextonly(1, 1, "\n***\n***\nResetting to slave configuration, IP =192.168.2.150\n");
                //        //						retval =system("/root/transfer_10MB.sh &");
                //        retval = system("/root/slave_config.sh");
                //        //						logprint(1,1,"Running nohup /root/transfer_10MB.sh &\n");
                //        //						retval =system("/bin/touch /data/t10");
                //        logprint(1, 1, "retval = %d\n***\n***\n", retval);
                //    }
                //}
                break;

            case OXFRSTART:
                get_timestamp(timestamp);
                TERMCOLOR(TERM_RED);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: %s not implemented\n", MsgID2Str(msg.hdr.msg_type)));
                TERMCOLOR(TERM_BLACK);
                sprintf(msg_out, "%s not implemented", MsgID2Str(msg.hdr.msg_type));
                send_msg(msg.hdr.src, my_id, ERROR_MSG, strlen(msg_out), msg_out);

                // pval = 0;
                // if ((ret = sscanf(msg.body, "%s",
                //                 paramstr) == 1))
                //{
                //     if (strcmp(paramstr, "10MB") == 0)
                //     {

                //        logtextonly(1, 1, "\n***\n***\nStarting 10MB transfer\n");
                //        //						retval =system("/root/transfer_10MB.sh &");
                //        retval = system("/usr/bin/nohup /root/transfer_10MB.sh &");
                //        //						logprint(1,1,"Running nohup /root/transfer_10MB.sh &\n");
                //        //						retval =system("/bin/touch /data/t10");
                //        logprint(1, 1, "retval = %d\n***\n***\n", retval);
                //    }
                //    else if (strcmp(paramstr, "100MB") == 0)
                //    {
                //        logtextonly(1, 1, "\n***\n***\nStarting 100MB transfer\n");

                //        //						retval= system("/root/transfer_100MB.sh &");
                //        retval = system("/usr/bin/nohup /root/transfer_100MB.sh &");
                //        //						retval =system("/bin/touch /data/t100");
                //        logprint(1, 1, "retval = %d\n***\n***\n", retval);
                //    }
                //    else if (strcmp(paramstr, "Hail") == 0)
                //    {
                //        logtextonly(1, 1, "\n***\n***\nStarting 100MB transfer\n");

                //        retval = system("/bin/rm /data/10MBdata");
                //        retval = system("/usr/bin/nohup /usr/bin/rsync -v -P -e "
                //                "ssh -i /root/.ssh/id_rsa"
                //                " root@213.123.1.106:/data/10MBdata /data &");
                //        logprint(1, 1, "retval = %d\n***\n***\n", retval);
                //    }
                //}
                break;

            case OM_NETPKT_RD:
                get_timestamp(timestamp);
                TERMCOLOR(TERM_RED);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: %s not implemented\n", MsgID2Str(msg.hdr.msg_type)));
                TERMCOLOR(TERM_BLACK);
                sprintf(msg_out, "%s not implemented", MsgID2Str(msg.hdr.msg_type));
                send_msg(msg.hdr.src, my_id, ERROR_MSG, strlen(msg_out), msg_out);

                // printf("Read net packets\n");
                ////                com_pkt_read(&scfg, err_data);
                break;

            case OM_XFRTXFLAG:
                TERMCOLOR(TERM_RED);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: %s not implemented\n", MsgID2Str(msg.hdr.msg_type)));
                TERMCOLOR(TERM_BLACK);
                sprintf(msg_out, "%s not implemented", MsgID2Str(msg.hdr.msg_type));
                send_msg(msg.hdr.src, my_id, ERROR_MSG, strlen(msg_out), msg_out);

                //// #(Note: Direction 0-MSC=Rcv,RSC=Tx, 1-MSC-Tx,Rsc=Rx)
                // if (sscanf(msg.body, "%d", &xfrtxflag) == 1)
                //{
                //     if (xfrtxflag == 1)
                //     {
                //         logtextonly(1, 1, "Call system -/root/mast_tx.sh\n");

                //        system("/root/mast_tx.sh");
                //        sleep(1);
                //    }
                //    else if (xfrtxflag == 0)
                //    {
                //        logtextonly(1, 1, "Call system -/root/mast_rcv.sh\n");
                //        system("/root/mast_rcv.sh");
                //        sleep(1);
                //    }
                //}
                break;

            case OM_NAVID_REQ:
                get_timestamp(timestamp);

                SHM_LOCK(SHM_nav_info);
                sprintf(nav_info.identifier, "%s", SHM_nav_info.data.identifier);
                nav_info.latitude = SHM_nav_info.data.latitude;
                nav_info.longitude = SHM_nav_info.data.longitude;
                nav_info.depth = SHM_nav_info.data.depth;
                SHM_UNLOCK(SHM_nav_info);
                sprintf(msg_out, "NAVID %s %10.5f %10.5f %.1f", nav_info.identifier, nav_info.latitude, nav_info.longitude, nav_info.depth);
                send_msg(msg.hdr.src, my_id, OM_NAVID_RESP, strlen(msg_out), msg_out);

                break;

            case OM_NAVID_SAVE_REQ:
                get_timestamp(timestamp);
                TERMCOLOR(TERM_RED);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: %s not implemented\n", MsgID2Str(msg.hdr.msg_type)));
                TERMCOLOR(TERM_BLACK);
                sprintf(msg_out, "%s not implemented", MsgID2Str(msg.hdr.msg_type));
                send_msg(msg.hdr.src, my_id, ERROR_MSG, strlen(msg_out), msg_out);

                // printf("Received msg %s", msg.body);

                // if (save_navid("/home/omuser/data/om_navid.cfg", msg.body, &nav_info) == 1) // returns when file is found
                //{
                //     if (stat("/home/omuser/data/om_navid.cfg", &stat_buf) == 0) // returns when file is found
                //     {
                //         sprintf(msg_out, "NAVID %s %10.5f %10.5f %.1f", nav_info.identifier, nav_info.latitude, nav_info.longitude, nav_info.depth);
                //         send_msg(msg.hdr.src, my_id, OM_NAVID_SAVE_RESP, strlen(msg_out), msg_out);
                //     }
                //     else
                //     {
                //         sprintf(msg_out, "Error: Internal error, OM_NAVID_SAVE_REQ not completed \n");
                //         send_msg(msg.hdr.src, my_id, OM_NAVID_SAVE_RESP, strlen(msg_out), msg_out);
                //     }
                // }
                break;

            case OM_NAVID_MSG:
                SHM_LOCK(SHM_cairn_nav_info)
                retval = sscanf(msg.body, "%s %f %f %f",
                                SHM_cairn_nav_info.data.identifier,
                                &SHM_cairn_nav_info.data.latitude,
                                &SHM_cairn_nav_info.data.longitude,
                                &SHM_cairn_nav_info.data.depth);
                SHM_UNLOCK(SHM_cairn_nav_info)
                // printf("MAIN: Received OM_NAVID_MSG, ret val = %d\n", retval);
                break;

            case OM_NAVDATA: // Navigation data from ONAV camera
#if OM_NAV
            { // Scope for 'auto' declarations below
                // Expected format:
                // onav_state  timestamp_tv_sec  timestamp_tv_nsec acq_timestamp_tv_sec acq_timestamp_tv_nsec camera_azimuth  camera_elevation  camera_azimuth_error  camera_elevation_error camera_pitch camera_roll camera_sequence_number
                // NOTE!!! sscanf format MUST MATCH variable type!!!
                retval = sscanf(msg.body, "%hu %lu %lu %lu %lu %f %f %f %f %f %f %hhu",
                                &nav_status.navStatus, &navdata_msg.tv_sec, &navdata_msg.tv_nsec, &navdata_acq.tv_sec, &navdata_acq.tv_nsec,
                                &nav_status.cameraAzimuth, &nav_status.cameraElevation, &nav_status.cameraAzimuthError, &nav_status.cameraElevationError,
                                &nav_status.cameraPitch, &nav_status.cameraRoll, &nav_status.cameraSequenceNumber);
                if (retval == 12)
                {
                    timespec_get(&navdata_tic, TIME_UTC);
                    // printf("MAIN: Received OM_NAVDATA, ret val = %d, navStatus = %d, sequence = %d\n", retval, nav_status.navStatus, nav_status.cameraSequenceNumber);
                    // printf("MAIN: Received OM_NAVDATA, length = %ld, message = \"%s\"\n", strlen(msg.body), msg.body);
                    auto last_nav_update_time = ((double)(navdata_tic.tv_sec - navdata_tic_last.tv_sec) + (double)(navdata_tic.tv_nsec - navdata_tic_last.tv_nsec) * 1.E-9);

                    if (last_nav_update_time > 2.)
                    {
                        TERMCOLOR(TERM_MAGENTA);
                        DPRINTF(WNG, DEBUG_LEVEL,
                                printf("main(): WARNING OM_NAVDATA timestamp difference larger than two seconds: %.1f\n", last_nav_update_time));
                        TERMCOLOR(TERM_BLACK);
                    }


                    // Prepare for calculation of camera latency.
                    // Assumptions:
                    // - System clocks of camera and modem are synchronized via NTP to better than 1 ms.
                    // - Time between sending of UDP packet from camera to receipt on modem is negligible (<1 ms)
                    // Convert camera's acquistion timespec timestamp to local steady clock time_point
                    // Get time difference between message timestamp and the acquisition timestamp
                    auto navdata_acq_delta = std::chrono::seconds{navdata_msg.tv_sec - navdata_acq.tv_sec} + std::chrono::nanoseconds{navdata_msg.tv_nsec - navdata_acq.tv_nsec};
                    // Get apparent difference between our receive time and the message time; this should be zero, but clock differences can make that not true.
                    auto navdata_msg_delta = std::chrono::seconds{navdata_tic.tv_sec - navdata_msg.tv_sec} + std::chrono::nanoseconds{navdata_tic.tv_nsec - navdata_msg.tv_nsec};
                    // Convert to a std::chrono::duration type
                    auto navdata_acq_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(navdata_acq_delta);
                    auto navdata_msg_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(navdata_msg_delta);
                    // Remove these durations from our steady_clock receive time to get the acquisition time in our steady_clock
                    auto tic_steady_clock = std::chrono::steady_clock::now();
                    auto acq_steady_clock = tic_steady_clock - navdata_msg_duration - navdata_acq_duration;

                    SHM_LOCK(SHM_nav_status);
                    SHM_nav_status.data.acq_steady_clock = acq_steady_clock;
                    SHM_nav_status.data.proc_steady_clock = tic_steady_clock;
                    SHM_nav_status.data.msgTimestamp.tv_sec = navdata_tic.tv_sec;
                    SHM_nav_status.data.msgTimestamp.tv_nsec = navdata_tic.tv_nsec;
                    SHM_nav_status.data.acqTimestamp.tv_sec = navdata_acq.tv_sec;
                    SHM_nav_status.data.acqTimestamp.tv_nsec = navdata_acq.tv_nsec;
                    SHM_nav_status.data.msgTimeDelta_ms = navdata_msg_delta.count();
                    SHM_nav_status.data.cameraAzimuth = nav_status.cameraAzimuth;
                    SHM_nav_status.data.cameraElevation = nav_status.cameraElevation;
                    SHM_nav_status.data.cameraAzimuthError = nav_status.cameraAzimuthError;
                    SHM_nav_status.data.cameraElevationError = nav_status.cameraElevationError;
                    SHM_nav_status.data.cameraPitch = nav_status.cameraPitch;
                    SHM_nav_status.data.cameraRoll = nav_status.cameraRoll;
                    SHM_nav_status.data.cameraSequenceNumber = nav_status.cameraSequenceNumber;
                    SHM_nav_status.data.navStatus = nav_status.navStatus;
                    SHM_UNLOCK(SHM_nav_status);

                    navdata_tic_last = navdata_tic;
                }
                else
                {
                    TERMCOLOR(TERM_MAGENTA);
                    DPRINTF(ERR, DEBUG_LEVEL,
                            sprintf(msg_out, "ERROR in readback of OM_NAVDATA message"));
                    TERMCOLOR(TERM_BLACK);
                }
            }
#else
                TERMCOLOR(TERM_MAGENTA);
                DPRINTF(ERR, DEBUG_LEVEL,
                        sprintf(msg_out, "ERROR Nav not enabled in this build"));
                TERMCOLOR(TERM_BLACK);
#endif

                break;

            case OM_SOALRDAT:
            case OM_MOALRDAT:
                // Always re-broadcast to the web interface
                send_msg(WEB, my_id, msg.hdr.msg_type, strlen(msg.body), msg.body);

                // *LRDAT message format copied from below
                // sprintf(msg_out,"%01d %s %02d %05d %05d %05d %06d %05d %05d %05d %05d %05d %03d %04d %04d %04d %04d %4.2e  %03d %04d %04d %04d %04d %4.2e %02d %05d %03d %03d %03d %03d %04.1f %04.1f %02x %01d %4.1f %04d",
                //        om_tstatus.state, timestamp, om_tstatus.mode,                                                                   // fields 1--3
                //        (int) (om_tstatus.rawratecalc/1000), (int) (om_tstatus.txratecalc/1000), (int) (om_tstatus.rxratecalc/1000),    // fields 4--6
                //        om_tstatus.rxfec_corrd_ps, om_tstatus.rxfec_faild_ps, om_tstatus.rxfec_deadlink, om_tstatus.rxgfp_framedrop,    // fields 7--10
                //        (int) (om_tstatus.tx_bytes_ps/1000), (int) (om_tstatus.rx_bytes_ps/1000),                                       // fields 11--12
                //        om_tstatus.pmt_locsnr, om_tstatus.hv_dac_val, om_tstatus.dcpwr,                                                 // fields 13--15
                //        om_tstatus.acpwr, om_tstatus.bscatter, om_tstatus.pmt_locestpwr,                                                // fields 16--18
                //        om_tstatus.pmt_remsnr, om_tstatus.pmt_remhvdac, om_tstatus.pmt_remdc,                                           // fields 19--21
                //        om_tstatus.pmt_remac, om_tstatus.pmt_rembscatter, om_tstatus.pmt_remestpwr,                                     // fields 22--24
                //        om_tstatus.ber_lock, berdisp,                                                                                   // fields 25--26
                //        om_tstatus.rd_reg1, om_tstatus.rd_reg2, om_tstatus.rd_reg3, om_tstatus.rd_reg4,                                 // fields 27--30
                //        om_tstatus.relhum, om_tstatus.airtemp, env_error, mode_indicator, om_tstatus.sys_volts, om_tstatus.tof_clk);    // fields 31--36

                // Parse msg.body. Note that there is an extra space in the timestamp
                msg_ptr = &msg.body[0];
                for (cnt = 0; cnt < 34; cnt++)
                {
                    token = strtok_r(msg_ptr, space, &msg_ptr);

                    switch (cnt)
                    {
                    case 13:
                        sscanf(token, "%d", &om_status.pmt_remsnr);
                        break;

                    case 14:
                        sscanf(token, "%d", &om_status.pmt_remhvdac);
                        break;

                    case 15:
                        sscanf(token, "%d", &om_status.pmt_remdc);
                        break;

                    case 16:
                        sscanf(token, "%d", &om_status.pmt_remac);
                        break;

                    case 17:
                        sscanf(token, "%d", &om_status.pmt_rembscatter);
                        break;

                    case 18:
                        sscanf(token, "%f", &om_status.pmt_remestpwr);
                        break;

                    default:
                        // nothing to do
                        break;
                    }
                }

                SHM_LOCK(SHM_om_status);
                SHM_om_status.data.pmt_remsnr = om_status.pmt_remsnr;
                SHM_om_status.data.pmt_remhvdac = om_status.pmt_remhvdac;
                SHM_om_status.data.pmt_remdc = om_status.pmt_remdc;
                SHM_om_status.data.pmt_remac = om_status.pmt_remac;
                SHM_om_status.data.pmt_rembscatter = om_status.pmt_rembscatter;
                SHM_om_status.data.pmt_remestpwr = om_status.pmt_remestpwr;
                SHM_UNLOCK(SHM_om_status);
                break;

            case OM_WAKE_REQ:
#if OM_WAKE
                sprintf(msg_out, "ERROR Invalid argument to OM_WAKE_REQ");

                if (sscanf(msg.body, "%d", &cnt) == 1)
                {
                    if (cnt == 1)
                    {
                        sprintf(msg_out, "Enabling wake broadcast routine");
                        wakeThreadEnable = true;
                    }
                    else if (cnt == 0)
                    {
                        sprintf(msg_out, "Disabling wake broadcast routine");
                        wakeThreadEnable = false;
                    }
                }
#else
                sprintf(msg_out, "ERROR Wake broadcast not enabled in this build");
#endif
                TERMCOLOR(TERM_MAGENTA);
                DPRINTF(MSG, DEBUG_LEVEL,
                        printf("MAIN: %s\n", msg_out));
                TERMCOLOR(TERM_BLACK);

                send_msg(msg.hdr.src, my_id, OM_WAKE_RESP, strlen(msg_out), msg_out);

                break;

            default:
                TERMCOLOR(TERM_MAGENTA);
                DPRINTF(ERR, DEBUG_LEVEL,
                        printf("***Unknown msg type[%d] - ignoring\n", msg.hdr.msg_type));
                TERMCOLOR(TERM_BLACK);
            } // end switch case for msg types
        }     // end if msg.hdr.dest is my_id or my_id2
    }         // end-while !done

    close_comms();
    return 0;
}
