/*
 * 		@file socketgeni.c
 * *  	@date Nov 26, 2010
 *      @author Abdallah Abdallah
 *      @brief This is the FINS CORE including (the Jinni name pipes based
 *      server)
 *      notice that A read call will normally block; that is, it will cause the process to
 *       wait until data becomes available. If the other end of the pipe has been closed,
 *       then no process has the pipe open for writing, and the read blocks. Because this isn’t
 *       very helpful, a read on a pipe that isn’t open for writing returns zero rather than
 *       blocking. This allows the reading process to detect the pipe equivalent of end of file
 *       and act appropriately. Notice that this isn’t the same as reading an invalid file
 *       descriptor, which read considers an error and indicates by returning –1.
 *       */

#include "core.h"
#include <ipv4.h>
#include <udp.h>
#include <tcp.h>
#include <arp.h>
#include "switch/swito.h"		//was <swito.h>
#include "RTM/rtm.h"	//was <rtm.h>
#include <sys/types.h>
//#include <stdlib.h> //added
//#include <stdio.h> //added
//kernel stuff

/** Global parameters of the socketjinni
 *
 */

/**
 * TODO free and close/DESTORY all the semaphores before exit !!!
 * POSIX does not clean the garbage of semaphores at exiting
 * It must be cleaned manually incase the program crashes
 *
 *
 */

/*
 * Semaphore for recvfrom_udp/tcp/icmp threads created b/c of blocking
 * in UDPreadFrom_fins. Only lock/unlock when changing jinniSockets,
 * since recvfrom_udp just reads data.
 */
sem_t jinniSockets_sem;
struct finssocket jinniSockets[MAX_sockets];

int recv_thread_index;
int recv_thread_count;
sem_t recv_thread_sem;

/** The list of major Queues which connect the modules to each other
 * including the switch module
 * The list of Semaphores which protect the Queues
 */

finsQueue Jinni_to_Switch_Queue;
finsQueue Switch_to_Jinni_Queue;

finsQueue Switch_to_RTM_Queue;
finsQueue RTM_to_Switch_Queue;

finsQueue Switch_to_UDP_Queue;
finsQueue UDP_to_Switch_Queue;

finsQueue Switch_to_TCP_Queue;
finsQueue TCP_to_Switch_Queue;

finsQueue Switch_to_ARP_Queue;
finsQueue ARP_to_Switch_Queue;

finsQueue Switch_to_IPv4_Queue;
finsQueue IPv4_to_Switch_Queue;

finsQueue Switch_to_EtherStub_Queue;
finsQueue EtherStub_to_Switch_Queue;

finsQueue Switch_to_ICMP_Queue;
finsQueue ICMP_to_Switch_Queue;

sem_t Jinni_to_Switch_Qsem;
sem_t Switch_to_Jinni_Qsem;

/** RunTimeManager Module to connect to the user interface  */
sem_t RTM_to_Switch_Qsem;
sem_t Switch_to_RTM_Qsem;

sem_t Switch_to_UDP_Qsem;
sem_t UDP_to_Switch_Qsem;

sem_t ICMP_to_Switch_Qsem;
sem_t Switch_to_ICMP_Qsem;

sem_t Switch_to_TCP_Qsem;
sem_t TCP_to_Switch_Qsem;

sem_t Switch_to_IPv4_Qsem;
sem_t IPv4_to_Switch_Qsem;

sem_t Switch_to_ARP_Qsem;
sem_t ARP_to_Switch_Qsem;

sem_t Switch_to_EtherStub_Qsem;
sem_t EtherStub_to_Switch_Qsem;

finsQueue modules_IO_queues[MAX_modules];
sem_t *IO_queues_sem[MAX_modules];

/** ----------------------------------------------------------*/

int capture_pipe_fd; /** capture file descriptor to read from capturer */
int inject_pipe_fd; /** inject file descriptor to read from capturer */
int rtm_in_fd;
int rtm_out_fd;

/** Ethernet Stub Variables  */
#ifdef BUILD_FOR_ANDROID
#define FINS_TMP_ROOT "/data/data/fins"
#define CAPTURE_PIPE FINS_TMP_ROOT "/fins_capture"
#define INJECT_PIPE FINS_TMP_ROOT "/fins_inject"
#else
#define FINS_TMP_ROOT "/tmp/fins"
#define CAPTURE_PIPE FINS_TMP_ROOT "/fins_capture"
#define INJECT_PIPE FINS_TMP_ROOT "/fins_inject"
#define SEMAPHORE_ROOT "/dev/shm"
#endif

//bu_mark kernel stuff
#define RECV_BUFFER_SIZE	1024// Pick an appropriate value here
//end kernel stuff

/**
 * @brief read the core parameters from the configuraions file called fins.cfg
 * @param
 * @return nothing
 */
int read_configurations() {

	config_t cfg;
	config_setting_t *setting;
	const char *str;

	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if (!config_read_file(&cfg, "fins.cfg")) {
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
				config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return EXIT_FAILURE;
	}

	config_destroy(&cfg);
	return EXIT_SUCCESS;
}

/**
 * @brief initialize the jinni sockets array by filling with value of -1
 * @param
 * @return nothing
 */
void init_jinnisockets() {
	int i;

	sem_init(&jinniSockets_sem, 0, 1);
	for (i = 0; i < MAX_sockets; i++) {
		jinniSockets[i].uniqueSockID = -1;
		jinniSockets[i].connection_status = 0;
	}

	sem_init(&recv_thread_sem, 0, 1);
	recv_thread_index = 0;
	recv_thread_count = 0;
}

void Queues_init() {

	Jinni_to_Switch_Queue = init_queue("jinni2switch", MAX_Queue_size);
	Switch_to_Jinni_Queue = init_queue("switch2jinni", MAX_Queue_size);
	modules_IO_queues[0] = Jinni_to_Switch_Queue;
	modules_IO_queues[1] = Switch_to_Jinni_Queue;
	sem_init(&Jinni_to_Switch_Qsem, 0, 1);
	sem_init(&Switch_to_Jinni_Qsem, 0, 1);
	IO_queues_sem[0] = &Jinni_to_Switch_Qsem;
	IO_queues_sem[1] = &Switch_to_Jinni_Qsem;

	UDP_to_Switch_Queue = init_queue("udp2switch", MAX_Queue_size);
	Switch_to_UDP_Queue = init_queue("switch2udp", MAX_Queue_size);
	modules_IO_queues[2] = UDP_to_Switch_Queue;
	modules_IO_queues[3] = Switch_to_UDP_Queue;
	sem_init(&UDP_to_Switch_Qsem, 0, 1);
	sem_init(&Switch_to_UDP_Qsem, 0, 1);
	IO_queues_sem[2] = &UDP_to_Switch_Qsem;
	IO_queues_sem[3] = &Switch_to_UDP_Qsem;

	TCP_to_Switch_Queue = init_queue("tcp2switch", MAX_Queue_size);
	Switch_to_TCP_Queue = init_queue("switch2tcp", MAX_Queue_size);
	modules_IO_queues[4] = TCP_to_Switch_Queue;
	modules_IO_queues[5] = Switch_to_TCP_Queue;
	sem_init(&TCP_to_Switch_Qsem, 0, 1);
	sem_init(&Switch_to_TCP_Qsem, 0, 1);
	IO_queues_sem[4] = &TCP_to_Switch_Qsem;
	IO_queues_sem[5] = &Switch_to_TCP_Qsem;

	IPv4_to_Switch_Queue = init_queue("ipv42switch", MAX_Queue_size);
	Switch_to_IPv4_Queue = init_queue("switch2ipv4", MAX_Queue_size);
	modules_IO_queues[6] = IPv4_to_Switch_Queue;
	modules_IO_queues[7] = Switch_to_IPv4_Queue;
	sem_init(&IPv4_to_Switch_Qsem, 0, 1);
	sem_init(&Switch_to_IPv4_Qsem, 0, 1);
	IO_queues_sem[6] = &IPv4_to_Switch_Qsem;
	IO_queues_sem[7] = &Switch_to_IPv4_Qsem;

	ARP_to_Switch_Queue = init_queue("arp2switch", MAX_Queue_size);
	Switch_to_ARP_Queue = init_queue("switch2arp", MAX_Queue_size);
	modules_IO_queues[8] = ARP_to_Switch_Queue;
	modules_IO_queues[9] = Switch_to_ARP_Queue;
	sem_init(&ARP_to_Switch_Qsem, 0, 1);
	sem_init(&Switch_to_ARP_Qsem, 0, 1);
	IO_queues_sem[8] = &ARP_to_Switch_Qsem;
	IO_queues_sem[9] = &Switch_to_ARP_Qsem;

	EtherStub_to_Switch_Queue = init_queue("etherstub2switch", MAX_Queue_size);
	Switch_to_EtherStub_Queue = init_queue("switch2etherstub", MAX_Queue_size);
	modules_IO_queues[10] = EtherStub_to_Switch_Queue;
	modules_IO_queues[11] = Switch_to_EtherStub_Queue;
	sem_init(&EtherStub_to_Switch_Qsem, 0, 1);
	sem_init(&Switch_to_EtherStub_Qsem, 0, 1);
	IO_queues_sem[10] = &EtherStub_to_Switch_Qsem;
	IO_queues_sem[11] = &Switch_to_EtherStub_Qsem;

	ICMP_to_Switch_Queue = init_queue("icmp2switch", MAX_Queue_size);
	Switch_to_ICMP_Queue = init_queue("switch2icmp", MAX_Queue_size);
	modules_IO_queues[12] = ICMP_to_Switch_Queue;
	modules_IO_queues[13] = Switch_to_ICMP_Queue;
	sem_init(&ICMP_to_Switch_Qsem, 0, 1);
	sem_init(&Switch_to_ICMP_Qsem, 0, 1);
	IO_queues_sem[12] = &ICMP_to_Switch_Qsem;
	IO_queues_sem[13] = &Switch_to_ICMP_Qsem;

	RTM_to_Switch_Queue = init_queue("rtm2switch", MAX_Queue_size);
	Switch_to_RTM_Queue = init_queue("switch2rtm", MAX_Queue_size);
	modules_IO_queues[14] = RTM_to_Switch_Queue;
	modules_IO_queues[15] = Switch_to_RTM_Queue;
	sem_init(&RTM_to_Switch_Qsem, 0, 1);
	sem_init(&Switch_to_RTM_Qsem, 0, 1);
	IO_queues_sem[14] = &RTM_to_Switch_Qsem;
	IO_queues_sem[15] = &Switch_to_RTM_Qsem;

}

void *Switch_to_Jinni() {

	struct finsFrame *ff;
	int protocol;
	int index;
	int status;
	uint16_t dstport, hostport;
	uint32_t dstip, hostip;

	while (1) {

		sem_wait(&Switch_to_Jinni_Qsem);
		ff = read_queue(Switch_to_Jinni_Queue);
		sem_post(&Switch_to_Jinni_Qsem);

		if (ff == NULL) {

			continue;
		}

		if (ff->dataOrCtrl == CONTROL) {
			PRINT_DEBUG("control ff");
		} else if (ff->dataOrCtrl == DATA) {
			dstport = -1;
			hostport = -1;
			dstip = -1;
			hostip = -1;
			protocol = -1;

			metadata_readFromElement(ff->dataFrame.metaData, "portdst",
					&dstport);
			metadata_readFromElement(ff->dataFrame.metaData, "portsrc",
					&hostport);
			metadata_readFromElement(ff->dataFrame.metaData, "ipdst", &dstip);
			metadata_readFromElement(ff->dataFrame.metaData, "ipsrc", &hostip);

			metadata_readFromElement(ff->dataFrame.metaData, "protocol",
					&protocol);
			PRINT_DEBUG("NETFORMAT %d,%d,%d,%d,%d,", protocol, hostip, dstip,
					hostport, dstport);

			protocol = ntohs(protocol);
			dstport = ntohs(dstport);
			hostport = ntohs(hostport);
			dstip = ntohl(dstip);
			hostip = ntohl(hostip);

			PRINT_DEBUG("NETFORMAT %d,%d,%d,%d,%d,", protocol, hostip, dstip,
					hostport, dstport);
			struct in_addr *temp = (struct in_addr *) malloc(sizeof(struct in_addr));
			temp->s_addr = hostip;
			struct in_addr *temp2 = (struct in_addr *) malloc(sizeof(struct in_addr));
			temp2->s_addr = dstip;
			PRINT_DEBUG("NETFORMAT %d, host=%s/%d, dst=%s/%d,", protocol,inet_ntoa(*temp), (hostport), inet_ntoa(*temp2), (dstport));
			PRINT_DEBUG("NETFORMAT %d, host=%d/%d, dst=%d/%d,", protocol,(*temp).s_addr, (hostport), (*temp2).s_addr, (dstport));

			/**
			 * check if this datagram comes from the address this socket has been previously
			 * connected to it (Only if the socket is already connected to certain address)
			 */
			if (jinniSockets[index].connection_status > 0) {

				PRINT_DEBUG("ICMP should not enter here at all");
				if ((hostport != jinniSockets[index].dstport) || (hostip
						!= jinniSockets[index].dst_IP)) {
					PRINT_DEBUG(
							"Wrong address, the socket is already connected to another destination");

					freeFinsFrame(ff);
					sem_post(&jinniSockets_sem);
					continue;

				}

			}

			/**
			 * check if this received datagram destIP and destport matching which socket hostIP
			 * and hostport insidee our sockets database
			 */
			if (protocol == IPPROTO_ICMP) {
				index = matchjinniSocket(0, hostip, protocol);
			}

			else {

				index = matchjinniSocket(dstport, dstip, protocol);
			}

			PRINT_DEBUG("index %d", index);
			if (index != -1) {
				PRINT_DEBUG("Matched: host=%d/%d, dst=%d/%d, prot=%d", jinniSockets[index].host_IP, jinniSockets[index].hostport, jinniSockets[index].dst_IP, jinniSockets[index].dstport, jinniSockets[index].protocol);

				int value;
				sem_getvalue(&(jinniSockets[index].Qs), &value);
				PRINT_DEBUG("sem: ind=%d, val=%d", index, value);
				sem_wait(&(jinniSockets[index].Qs));
				/**
				 * TODO Replace The data Queue with a pipeLine at least for
				 * the RAW DATA in order to find a natural way to support
				 * Blocking and Non-Blocking mode
				 */
				write_queue(ff, jinniSockets[index].dataQueue);
				sem_post(&(jinniSockets[index].Qs));
				//PRINT_DEBUG("pdu=\"%s\"", ff->dataFrame.pdu);

				char *buf;
				buf = (char *)malloc(ff->dataFrame.pduLength+1);
				memcpy(buf, ff->dataFrame.pdu, ff->dataFrame.pduLength);
				buf[ff->dataFrame.pduLength] = '\0';
				PRINT_DEBUG("pdu='%s'", buf);
				free(buf);

				PRINT_DEBUG("pdu length %d", ff->dataFrame.pduLength);
			}

			else {
				PRINT_DEBUG("No match, freeing ff");

				freeFinsFrame(ff);
			}
		} else {

			PRINT_DEBUG(
					"unknown FINS Frame type NOT DATA NOT CONTROL !!!Probably FORMAT ERROR");

		} // end of if , else if , else statement


	} // end of while
} // end of function

void *interceptor_to_jinni() {
	int ret_val;
	//int nl_sockfd;
	/*
	 nl_sockfd = init_fins_nl();
	 if (nl_sockfd == -1) { // if you get an error here, check to make sure you've inserted the FINS LKM first.
	 perror("init_fins_nl() caused an error");
	 exit(-1);
	 }
	 */
	// Begin receive message section
	// Allocate a buffer to hold contents of recvfrom call
	void *recv_buf;
	recv_buf = malloc(RECV_BUFFER_SIZE + 16); //16 = NLMSGHDR size
	if (recv_buf == NULL) {
		fprintf(stderr, "buffer allocation failed\n");
		exit(-1);
	}

	struct sockaddr sockaddr_sender; // Needed for recvfrom
	socklen_t sockaddr_senderlen; // Needed for recvfrom

	struct nlmsghdr *nlh;
	void *nl_buf; // Pointer to your actual data payload
	ssize_t nl_len, part_len; // Size of your actual data payload
	u_char *part_pt;

	void *msg_buf = NULL;
	ssize_t msg_len = -1;
	u_char *msg_pt = NULL;

	int okFlag, doneFlag = 0;
	ssize_t test_msg_len;

	int pos;
	int socketCallType; // Integer representing what socketcall type was placed (for testing purposes)
	unsigned long long uniqueSockID;
	int threads;

	PRINT_DEBUG("Waiting for message from kernel\n");

	int counter = 0;
	while (1) {

		PRINT_DEBUG("NL counter = %d", counter++);
		ret_val = recvfrom(nl_sockfd, recv_buf, RECV_BUFFER_SIZE + 16, 0,
				&sockaddr_sender, &sockaddr_senderlen);
		if (ret_val == -1) {
			perror("recvfrom() caused an error");
			exit(-1);
		}
		//PRINT_DEBUG("%d", sockaddr_sender);

		nlh = (struct nlmsghdr *) recv_buf;

		if (okFlag = NLMSG_OK(nlh, ret_val)) {
			switch (nlh->nlmsg_type) {
			case NLMSG_NOOP:
				PRINT_DEBUG("nlh->nlmsg_type=NLMSG_NOOP");
				break;
			case NLMSG_ERROR:
				PRINT_DEBUG("nlh->nlmsg_type=NLMSG_ERROR");
			case NLMSG_OVERRUN:
				PRINT_DEBUG("nlh->nlmsg_type=NLMSG_OVERRUN");
				okFlag = 0;
				break;
			case NLMSG_DONE:
				PRINT_DEBUG("nlh->nlmsg_type=NLMSG_DONE");
				doneFlag = 1;
			default:
				PRINT_DEBUG("nlh->nlmsg_type=default");
				nl_buf = NLMSG_DATA(nlh);
				nl_len = NLMSG_PAYLOAD(nlh, 0);

				PRINT_DEBUG("nl_len= %d", nl_len);

				part_pt = nl_buf;
				test_msg_len = *(ssize_t *) part_pt;
				part_pt += sizeof(ssize_t);

				//PRINT_DEBUG("test_msg_len=%d, msg_len=%d", test_msg_len, msg_len);

				if (msg_len == -1) {
					msg_len = test_msg_len;
				} else if (test_msg_len != msg_len) {
					okFlag = 0;
					PRINT_DEBUG("test_msg_len != msg_len");
					//could just malloc msg_buff again
					break; //might comment out or make so start new
				}

				part_len = *(ssize_t *) part_pt;
				part_pt += sizeof(ssize_t);
				if (part_len > RECV_BUFFER_SIZE) {
					PRINT_DEBUG("part_len (%d) > RECV_BUFFER_SIZE (%d)",
							part_len, RECV_BUFFER_SIZE);
				}

				//PRINT_DEBUG("part_len=%d", part_len);

				pos = *(int *) part_pt;
				part_pt += sizeof(int);
				if (pos > msg_len || pos != msg_pt - (unsigned char *) msg_buf) {
					if (pos > msg_len) {
						PRINT_DEBUG("pos > msg_len");
					} else {
						PRINT_DEBUG("pos != msg_pt - msg_buf");
					}
				}

				//PRINT_DEBUG("pos=%d", pos);

				PRINT_DEBUG("msg_len=%d part_len=%d pos=%d seq=%d", msg_len,
						part_len, pos, nlh->nlmsg_seq);

				if (nlh->nlmsg_seq == 0) {
					if (msg_buf != NULL) {
						PRINT_DEBUG(
								"error: msg_buf != NULL at new sequence, freeing");
						free(msg_buf);
					}
					msg_buf = malloc(msg_len);
					if (msg_buf == NULL) {
						fprintf(stderr, "msg buffer allocation failed\n");
						exit(-1);
					}
					msg_pt = msg_buf;
				}

				if (msg_pt != NULL) {
					msg_pt = msg_buf + pos; //atm redundant, is for if out of sync msgs
					memcpy(msg_pt, part_pt, part_len);
					msg_pt += part_len;
				} else {
					PRINT_DEBUG("error: msg_pt is NULL");
				}

				if ((nlh->nlmsg_flags & NLM_F_MULTI) == 0) {
					//doneFlag = 1; //not multi-part msg //removed multi
				}
				break;
			}
		}

		if (okFlag != 1) {
			doneFlag = 0;
			PRINT_DEBUG("okFlag != 1");
			//send kernel a resend request
			//with pos of part being passed can store msg_buf, then recopy new part when received
		}

		if (doneFlag) {
			msg_pt = msg_buf;

			socketCallType = *(int *) msg_pt;
			msg_pt += sizeof(int);

			uniqueSockID = *(unsigned long long *) msg_pt;
			msg_pt += sizeof(unsigned long long);

			threads = *(int *) msg_pt;
			msg_pt += sizeof(int);

			msg_len -= 2*sizeof(int) + sizeof(unsigned long long);

			PRINT_DEBUG("callType=%d sockID=%llu", socketCallType, uniqueSockID);
			PRINT_DEBUG("msg_len=%d", msg_len);

			//###############################
			unsigned char *temp;
			temp = (char *)malloc(msg_len+1);
			memcpy(temp, msg_pt, msg_len);
			temp[msg_len] = '\0';
			PRINT_DEBUG("msg='%s'", temp);
			free(temp);

			unsigned char *pt;
			temp = (unsigned char *) malloc(3*msg_len);
			pt = temp;
			int i;
			for (i = 0; i < msg_len; i++) {
				if (i == 0) {
					sprintf(pt, "%02x", msg_pt[i]);
					pt += 2;
				} else if (i % 4 == 0) {
					sprintf(pt, ":%02x", msg_pt[i]);
					pt += 3;
				} else {
					sprintf(pt, " %02x", msg_pt[i]);
					pt += 3;
				}
			}
			PRINT_DEBUG("msg='%s'", temp);
			free(temp);
			//###############################

			PRINT_DEBUG("uniqueSockID=%llu, calltype=%d, threads=%d", uniqueSockID, socketCallType, threads);

			switch (socketCallType) {

			case socket_call:
				socket_call_handler(uniqueSockID, threads, msg_pt, msg_len); //DONE
				break;
			case socketpair_call:
				socketpair_call_handler(uniqueSockID, threads, msg_pt, msg_len);
				break;
			case bind_call:
				bind_call_handler(uniqueSockID, threads, msg_pt, msg_len); //DONE
				break;
			case getsockname_call:
				getsockname_call_handler(uniqueSockID, threads, msg_pt, msg_len); //DONE
				break;
			case connect_call:
				connect_call_handler(uniqueSockID, threads, msg_pt, msg_len); //DONE
				break;
			case getpeername_call:
				getpeername_call_handler(uniqueSockID, threads, msg_pt, msg_len); //DONE
				break;
				/**
				 * the write call is encapuslated as a send call with the
				 * parameter flags = -1000  			//DONE
				 */
			case send_call:
				send_call_handler(uniqueSockID, threads, msg_pt, msg_len); //DONE
				break;
			case recv_call:
				recv_call_handler(uniqueSockID, threads, msg_pt, msg_len); //DONE
				break;
			case sendto_call:
				sendto_call_handler(uniqueSockID, threads, msg_pt, msg_len); //DONE
				break;
			case recvfrom_call:
				recvfrom_call_handler(uniqueSockID, threads, msg_pt, msg_len); //DONE
				break;
			case sendmsg_call:
				sendmsg_call_handler(uniqueSockID, threads, msg_pt, msg_len); //DONE
				break;
			case recvmsg_call:
				recvmsg_call_handler(uniqueSockID, threads, msg_pt, msg_len);
				break;
			case getsockopt_call:
				getsockopt_call_handler(uniqueSockID, threads, msg_pt, msg_len); //Dummy response
				break;
			case setsockopt_call:
				setsockopt_call_handler(uniqueSockID, threads, msg_pt, msg_len); // Dummy response
				break;
			case listen_call:
				listen_call_handler(uniqueSockID, threads, msg_pt, msg_len);
				break;
			case accept_call:
				accept_call_handler(uniqueSockID, threads, msg_pt, msg_len);
				break;
			case accept4_call:
				accept4_call_handler(uniqueSockID, threads, msg_pt, msg_len);
				break;
			case shutdown_call:
				shutdown_call_handler(uniqueSockID, threads, msg_pt, msg_len); //DONE
				break;
			case close_call:
				/**
				 * TODO fix the problem into remove jinnisockets
				 * the Queue Terminate function has a bug as explained into it
				 */
				close_call_handler(uniqueSockID, threads, msg_pt, msg_len);
				break;
			case release_call:
				release_call_handler(uniqueSockID, threads, msg_pt, msg_len);
				break;
			case ioctl_call:
				ioctl_call_handler(uniqueSockID, threads, msg_pt, msg_len);
				break;
			default:
				PRINT_DEBUG(
						"unknown opcode received (%d), dropping", socketCallType);
				/** a function must be called to clean and reset the pipe
				 * to original conditions before crashing
				 */
				//exit(1);
			}

			free(msg_buf);
			doneFlag = 0;
			msg_buf = NULL;
			msg_pt = NULL;
			msg_len = -1;
		}
	}

	free(recv_buf);
	close(nl_sockfd);
	exit(0);
}

void *Capture() {

	char *data;
	int datalen;
	int numBytes;
	int capture_pipe_fd;
	struct finsFrame *ff = NULL;

	metadata *ether_meta;

	struct sniff_ethernet *ethernet_header;
	u_char ethersrc[ETHER_ADDR_LEN];
	u_char etherdst[ETHER_ADDR_LEN];
	u_short protocol_type;

	capture_pipe_fd = open(CAPTURE_PIPE, O_RDONLY); //responsible for socket/ioctl call
	if (capture_pipe_fd == -1) {
		PRINT_DEBUG("opening capture_pipe did not work");
		exit(EXIT_FAILURE);
	}

	while (1) {

		numBytes = read(capture_pipe_fd, &datalen, sizeof(int));
		if (numBytes <= 0) {
			PRINT_DEBUG("numBytes written %d\n", numBytes);
			break;
		}
		data = (char *) malloc(datalen);

		numBytes = read(capture_pipe_fd, data, datalen);

		if (numBytes <= 0) {
			PRINT_DEBUG("numBytes written %d\n", numBytes);
			break;
		}

		PRINT_DEBUG("A frame of length %d has been written-----", datalen);

		//print_frame(data,datalen);

		ff = (struct finsFrame *) malloc(sizeof(struct finsFrame));

		PRINT_DEBUG("%d", (int) ff);

		/** TODO
		 * 1. extract the Ethernet Frame
		 * 2. pre-process the frame in order to extract the metadata
		 * 3. build a finsFrame and insert it into EtherStub_to_Switch_Queue
		 */
		ether_meta = (metadata *) malloc(sizeof(metadata));
		metadata_create(ether_meta);

		memcpy(ethersrc, ((struct sniff_ethernet *) data)->ether_shost,
				ETHER_ADDR_LEN);
		PRINT_DEBUG();
		memcpy(etherdst, ((struct sniff_ethernet *) data)->ether_dhost,
				ETHER_ADDR_LEN);
		PRINT_DEBUG();
		protocol_type = ntohs(((struct sniff_ethernet *) data)->ether_type);

		PRINT_DEBUG();

		ff->dataOrCtrl = DATA;
		(ff->destinationID).id = IPV4ID;
		(ff->destinationID).next = NULL;

		(ff->dataFrame).directionFlag = UP;
		ff->dataFrame.metaData = ether_meta;
		ff->dataFrame.pduLength = datalen - SIZE_ETHERNET;
		ff->dataFrame.pdu = data + SIZE_ETHERNET;

		//memcpy( ff->dataFrame.pdu , data + SIZE_ETHERNET ,datalen- SIZE_ETHERNET);

		PRINT_DEBUG("%d", (int) &(ff->dataFrame).pdu);
		PRINT_DEBUG("%d", (int) &((ff->dataFrame).pdu));
		PRINT_DEBUG("%d", (int) data);

		PRINT_DEBUG();

		sem_wait(&EtherStub_to_Switch_Qsem);
		write_queue(ff, EtherStub_to_Switch_Queue);
		sem_post(&EtherStub_to_Switch_Qsem);
		PRINT_DEBUG();

	} // end of while loop


}

void *Inject() {

	//char data[]="loloa7aa7a";
	char *frame;
	int datalen = 10;
	int framelen;
	int inject_pipe_fd;
	int numBytes;
	struct finsFrame *ff = NULL;
	struct ipv4_packet *packet;
	IP4addr destination;
	struct hostent *loop_host;
	uint32_t dstip;

	inject_pipe_fd = open(INJECT_PIPE, O_WRONLY);
	if (inject_pipe_fd == -1) {
		PRINT_DEBUG("opening inject_pipe did not work");
		exit(EXIT_FAILURE);
	}

	PRINT_DEBUG();

	while (1) {

		/** TO DO
		 * 1) read fins frames from the Switch_EthernetStub_queue
		 * 2) extract the data (Ethernet Frame) to be sent
		 * 3) Inject the Ethernet Frame into the injection Pipe
		 */
		sem_wait(&Switch_to_EtherStub_Qsem);
		ff = read_queue(Switch_to_EtherStub_Queue);
		sem_post(&Switch_to_EtherStub_Qsem);
		/** ff->finsDataFrame is an IPv4 packet */
		if (ff == NULL)
			continue;

		PRINT_DEBUG(
				"\n At least one frame has been read from the Switch to Etherstub");

		//	metadata_readFromElement(ff->dataFrame.metaData,"dstip",&destination);
		//	loop_host = (struct hostent *) gethostbyname((char *)"");
		//	if ( destination !=  ((struct in_addr *)(loop_host->h_addr))->s_addr )
		//	{
		/* TODO send ARP REQUEST TO GET THE CORRESPONDING MAC ADDRESS
		 * *
		 */
		//		PRINT_DEBUG("NEED MAC ADDRESS");
		//		freeFinsFrame(ff);
		//		continue;

		//	}


		framelen = ff->dataFrame.pduLength;
		frame = (char *) malloc(framelen + SIZE_ETHERNET);

		/** TODO Fill the dest and src with the correct MAC addresses
		 * you receive from the ARP module
		 */
		//char dest[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		//char src[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

		char dest[] = { 0x00, 0x1c, 0xbf, 0x86, 0xd2, 0xda }; // Mark Machine
		char src[] = { 0x00, 0x1c, 0xbf, 0x87, 0x1a, 0xfd };

		memcpy(((struct sniff_ethernet *) frame)->ether_dhost, dest,
				ETHER_ADDR_LEN);
		memcpy(((struct sniff_ethernet *) frame)->ether_shost, src,
				ETHER_ADDR_LEN);
		((struct sniff_ethernet *) frame)->ether_type = htons(0x0800);

		memcpy(frame + SIZE_ETHERNET, (ff->dataFrame).pdu, framelen);
		datalen = framelen + SIZE_ETHERNET;
		//	print_finsFrame(ff);
		PRINT_DEBUG("jinni inject to ethernet stub \n");
		numBytes = write(inject_pipe_fd, &datalen, sizeof(int));
		if (numBytes <= 0) {
			PRINT_DEBUG("numBytes written %d\n", numBytes);
			return (0);
		}

		numBytes = write(inject_pipe_fd, frame, datalen);

		if (numBytes <= 0) {
			PRINT_DEBUG("numBytes written %d\n", numBytes);
			return (0);
		}

		freeFinsFrame(ff);
		free(frame);
	} // end of while loop


} // end of Inject Function


void *UDP() {

	udp_init();

}

void *RTM() {

	rtm_init();

}

void *TCP() {
	tcp_init();

}

void *IPv4() {
	ipv4_init();

}

void *ICMP() {

	ICMP_init();

}

void *ARP() {

	ARP_init();

}

void *fins_switch() {

	init_switch();

}

void cap_inj_init() {

}

int main() {

	//init the netlink socket connection to daemon
	//int nl_sockfd;
	nl_sockfd = init_fins_nl();
	if (nl_sockfd == -1) { // if you get an error here, check to make sure you've inserted the FINS LKM first.
		perror("init_fins_nl() caused an error");
		exit(-1);
	}

	//prime the kernel to establish daemon's PID
	int daemoncode = daemonconnect_call;
	int ret_val;
	ret_val = send_wedge(nl_sockfd, &daemoncode, sizeof(int), 0);
	if (ret_val != 0) {
		perror("sendfins() caused an error");
		exit(-1);
	}
	PRINT_DEBUG("Connected to wedge at %d", nl_sockfd);

	//added to include code from fins_jinni.sh -- mrd015 !!!!!
	if (mkfifo(RTM_PIPE_IN, 0777) != 0) {
		if (errno == EEXIST) {
			PRINT_DEBUG("mkfifo(" RTM_PIPE_IN ", 0777) already exists.");
		} else {
			PRINT_DEBUG("mkfifo(" RTM_PIPE_IN ", 0777) failed.");
			exit(1);
		}
	}
	if (mkfifo(RTM_PIPE_OUT, 0777) != 0) {
		if (errno == EEXIST) {
			PRINT_DEBUG("mkfifo(" RTM_PIPE_OUT ", 0777) already exists.");
		} else {
			PRINT_DEBUG("mkfifo(" RTM_PIPE_OUT ", 0777) failed.");
			exit(1);
		}
	}

	// added in semaphore clearing
#ifndef BUILD_FOR_ANDROID
	if (system("rm " SEMAPHORE_ROOT "/sem*.*") != 0) {
		PRINT_DEBUG("Cannot remove semaphore files in " SEMAPHORE_ROOT "!\n");
	} else {
		PRINT_DEBUG(SEMAPHORE_ROOT" cleaned successfully.\n\n");
	}
#endif
	// END of added section !!!!!


	/** 1. init the Jinni sockets database
	 * 2. Init the queues connecting Jinnin to thw FINS Switch
	 * 3.
	 */
	//	read_configurations();
	init_jinnisockets();
	Queues_init();

	cap_inj_init();

	pthread_t interceptor_to_jinni_thread;
	pthread_t Switch_to_jinni_thread;

	pthread_t udp_thread;
	pthread_t icmp_thread;
	pthread_t rtm_thread;
	//	pthread_t udp_outgoing;

	pthread_t tcp_thread;
	//	pthread_t tcp_outgoing;

	pthread_t ipv4_thread;
	//	pthread_t ip_outgoing;

	pthread_t arp_thread;
	//	pthread_t arp_outgoing;

	pthread_t etherStub_capturing;
	pthread_t etherStub_injecting;
	pthread_t switch_thread;

	/* original !!!!!
	 pthread_create(&interceptor_to_jinni_thread, NULL, interceptor_to_jinni, NULL);
	 pthread_create(&Switch_to_jinni_thread, NULL, Switch_to_Jinni, NULL);

	 pthread_create(&udp_thread, NULL, UDP, NULL);
	 //	pthread_create(&rtm_thread, NULL, RTM, NULL);

	 //	pthread_create(&icmp_thread,NULL,ICMP,NULL);
	 //	pthread_create(&tcp_thread,NULL,TCP,NULL);

	 pthread_create(&ipv4_thread, NULL, IPv4, NULL);
	 //pthread_create(&arp_thread,NULL,ARP,NULL);

	 pthread_create(&switch_thread, NULL, fins_switch, NULL);

	 pthread_create(&etherStub_capturing, NULL, Capture, NULL);
	 pthread_create(&etherStub_injecting, NULL, Inject, NULL);
	 */

	// ADDED !!!!! start
	pthread_attr_t fins_pthread_attr;
	pthread_attr_init(&fins_pthread_attr);
	pthread_create(&interceptor_to_jinni_thread, &fins_pthread_attr,
			interceptor_to_jinni, NULL); //this has named pipe input from interceptor
	pthread_create(&Switch_to_jinni_thread, &fins_pthread_attr,
			Switch_to_Jinni, NULL);
	pthread_create(&udp_thread, &fins_pthread_attr, UDP, NULL);
	pthread_create(&ipv4_thread, &fins_pthread_attr, IPv4, NULL);
	pthread_create(&switch_thread, &fins_pthread_attr, fins_switch, NULL);
	pthread_create(&etherStub_capturing, &fins_pthread_attr, Capture, NULL);
	pthread_create(&etherStub_injecting, &fins_pthread_attr, Inject, NULL);
	//^^^^^ end added !!!!!

	PRINT_DEBUG("created all threads\n");

	/**
	 *************************************************************
	 */
	pthread_join(interceptor_to_jinni_thread, NULL);
	pthread_join(Switch_to_jinni_thread, NULL);
	pthread_join(etherStub_capturing, NULL);
	pthread_join(etherStub_injecting, NULL);
	pthread_join(switch_thread, NULL);
	pthread_join(udp_thread, NULL);
	//	//	pthread_join(tcp_thread, NULL);
	//	//	pthread_join(icmp_thread, NULL);
	pthread_join(ipv4_thread, NULL);
	//	//	pthread_join(arp_thread, NULL);


	while (1) {

	}

	return (1);

}

