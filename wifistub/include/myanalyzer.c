/*
 * myanalyzer.c
 *
 *  Created on: Jun 8, 2010
 *      Author: Abdallah Abdallah
 */



#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "getMAC_Address.h"


int myanalyzer(int argc, char *argv[])

{
        FILE *fs, *ft, *ft2, *ft3, *ft4,*ft5,*ft6,*ft7,*ft8,*ft9,*ft10,*ft11;
        FILE *ft12,*ft13,*ft14,*ft15;


        char *end_bracket_location;
        int length_to_copy;

        unsigned char macAddress_local[13];
        unsigned char macAddress_parsed[13];
        char *packet;
        char *buffer;
        char *buffer2;
        char *buffer3;
        char *buffer4;
        char *network_protocol_type;
		unsigned char *transProtocolType;

        int i=0;
        int j=0;
        int cc=0;
        int number_of_captured_packets;
        char arg_temp1[100];
        cc=0;
        i=0;
        j=0;





       if (argc != 3)
       {
           printf("Number of entered arguments are wrong \n");
           printf("%s <file name> <number of captured packets>", argv[0]);

           exit(1);
       }

       number_of_captured_packets = atoi(argv[2]);
       strcpy(arg_temp1,argv[1]);
        fs = fopen (arg_temp1,"r");
        if ( fs == NULL )
        {
               printf("coud not open the file");
               exit(1);
        }

        ft = fopen (strcat(arg_temp1,"_temp"),"w");
        if ( ft == NULL )
        {
               printf( "Cannot open target file" );
               fclose( fs );
               exit(1);
        }

        strcpy(arg_temp1,argv[1]);

        ft2 = fopen (strcat(arg_temp1,"_ethernetOUT"),"w");
               if ( ft2 == NULL )
               {
                      printf( "Cannot open final target file" );
                      fclose( fs );
                      fclose (ft);
                      exit(1);
               }
               strcpy(arg_temp1,argv[1]);

               ft3 = fopen (strcat(arg_temp1,"_ethernetIN"),"w");
                          if ( ft3 == NULL )
                          {
                                 printf( "Cannot open final target file" );
                                 fclose( fs );
                                 fclose (ft);
                                 fclose (ft2);
                                 exit(1);
                          }
           strcpy(arg_temp1,argv[1]);
           ft4 = fopen (strcat(arg_temp1,"_IPOUT"),"w");
                      if ( ft4 == NULL )
                           {
                          printf( "Cannot open final target file" );
                          fclose( fs );
                          fclose (ft);
                          fclose (ft2);
                          fclose (ft3);
                          exit(1);
                           }
                      strcpy(arg_temp1,argv[1]);
                      ft5 = fopen (strcat(arg_temp1,"_IPIN"),"w");
                                 if ( ft5 == NULL )
                                      {
                                     printf( "Cannot open final target file" );
                                     fclose( fs );
									  fclose (ft);
									  fclose (ft2);
									  fclose (ft3);
                                     fclose (ft4);
                                     exit(1);
                                      }


                     strcpy(arg_temp1,argv[1]);
                  ft6 = fopen (strcat(arg_temp1,"_transportIN"),"w");
                    if ( ft6 == NULL )
                       {
                        printf( "Cannot open final target file" );
									fclose( fs );
									  fclose (ft);
									  fclose (ft2);
									  fclose (ft3);
                                     fclose (ft4);
                        exit(1);
                       }
                    strcpy(arg_temp1,argv[1]);
                          ft7 = fopen (strcat(arg_temp1,"_TCPIN"),"w");
                            if ( ft7 == NULL )
                               {
                                printf( "Cannot open final target file" );
                                fclose(fs);
									  fclose (ft);
									  fclose (ft2);
									  fclose (ft3);
                                     fclose (ft4);
                                     fclose (ft5);
                                exit(1);
                               }
                    strcpy(arg_temp1,argv[1]);
                          ft8 = fopen (strcat(arg_temp1,"_UDPIN"),"w");
                            if ( ft7 == NULL )
                               {
                                printf( "Cannot open final target file" );
                                fclose( fs );
									  fclose (ft);
									  fclose (ft2);
									  fclose (ft3);
                                     fclose (ft4);
                                     fclose (ft5);
                                     fclose (ft7);
                                exit(1);
                               }

                    strcpy(arg_temp1,argv[1]);
                          ft9 = fopen (strcat(arg_temp1,"_TCPOUT"),"w");
                            if ( ft9 == NULL )
                               {
                                printf( "Cannot open final target file" );
                                 fclose( fs );
									  fclose (ft);
									  fclose (ft2);
									  fclose (ft3);
                                     fclose (ft4);
                                     fclose (ft5);
                                     fclose (ft7);
                                fclose (ft8);
                                exit(1);
                               }
                    strcpy(arg_temp1,argv[1]);
                          ft10 = fopen (strcat(arg_temp1,"_UDPOUT"),"w");
                            if ( ft10 == NULL )
                               {
                                printf( "Cannot open final target file" );
                                fclose( fs );
									  fclose (ft);
									  fclose (ft2);
									  fclose (ft3);
                                     fclose (ft4);
                                     fclose (ft5);
                                     fclose (ft7);
                                fclose (ft9);
                                exit(1);
                               }
                   strcpy(arg_temp1,argv[1]);
                  ft11 = fopen (strcat(arg_temp1,"_transportOUT"),"w");
                    if ( ft11 == NULL )
                       {
                        printf( "Cannot open final target file" );
										fclose( fs );
									  fclose (ft);
									  fclose (ft2);
									  fclose (ft3);
                                     fclose (ft4);
                                     fclose (ft5);
                                     fclose (ft7);
                                fclose (ft9);
                                fclose (ft10);

                        exit(1);
                       }
      strcpy(arg_temp1,argv[1]);
                  ft12 = fopen (strcat(arg_temp1,"_TCPSOCKETOUT"),"w");
                    if ( ft12 == NULL )
                       {
                        printf( "Cannot open final target file" );
										fclose( fs );
									  fclose (ft);
									  fclose (ft2);
									  fclose (ft3);
                                     fclose (ft4);
                                     fclose (ft5);
                                     fclose (ft7);
                                fclose (ft9);
                                fclose (ft10);
								fclose (ft11);
                        exit(1);
                       }


  strcpy(arg_temp1,argv[1]);
                  ft13 = fopen (strcat(arg_temp1,"_TCPSOCKETIN"),"w");
                    if ( ft13 == NULL )
                       {
                        printf( "Cannot open final target file" );
										fclose( fs );
									  fclose (ft);
									  fclose (ft2);
									  fclose (ft3);
                                     fclose (ft4);
                                     fclose (ft5);
                                     fclose (ft7);
                                fclose (ft9);
                                fclose (ft10);
								fclose (ft11);
								fclose (ft12);

                        exit(1);
                       }

 strcpy(arg_temp1,argv[1]);
                  ft14 = fopen (strcat(arg_temp1,"_UDPSOCKETOUT"),"w");
                    if ( ft14 == NULL )
                       {
                        printf( "Cannot open final target file" );
										fclose( fs );
									  fclose (ft);
									  fclose (ft2);
									  fclose (ft3);
                                     fclose (ft4);
                                     fclose (ft5);
                                     fclose (ft7);
                                fclose (ft9);
                                fclose (ft10);
								fclose (ft11);
								fclose (ft12);
								fclose (ft13);
                        exit(1);
                       }

				strcpy(arg_temp1,argv[1]);
                  ft15 = fopen (strcat(arg_temp1,"_UDPSOCKETIN"),"w");
                    if ( ft15 == NULL )
                       {
                        printf( "Cannot open final target file" );
										fclose( fs );
									  fclose (ft);
									  fclose (ft2);
									  fclose (ft3);
                                     fclose (ft4);
                                     fclose (ft5);
                                     fclose (ft7);
                                fclose (ft9);
                                fclose (ft10);
								fclose (ft11);
								fclose (ft12);
								fclose (ft13);
								fclose (ft14);

                        exit(1);
                       }
						     packet=  (char *) malloc(10000);
                             buffer = (char *) malloc(100);
                             buffer2= (char *) malloc(10000);
                             buffer3 =(char *) malloc(10000);
                             buffer4= (char *) malloc (10000);




      getMAC_Address(macAddress_local);

        int counter =1;


       // printf("%d",number_of_captured_packets);

        while( counter <= number_of_captured_packets )
        {
			fgets(buffer,100,fs);
        	while( strchr(buffer,'{') ==NULL)
        	{
        		fgets(buffer,100,fs);
        	}

        	fgets(buffer,100,fs);
        	strcpy(buffer2,buffer);
        	fgets(buffer,100,fs);
        	while ( strchr(buffer,'}') == NULL )
        	{
        	strcat(buffer2,buffer);
        	fgets(buffer,100,fs);
        	}

        	strcat(buffer2,buffer);
        	end_bracket_location = strchr(buffer2,'}');
        	length_to_copy = end_bracket_location - buffer2-1;
        	strncpy(buffer3,buffer2,length_to_copy);

				i=0;
				j=0;

                    	 while ( i< length_to_copy )
       					{
       				if ( buffer3[i] == 'x' )
       					 {
       						packet[j]= buffer3[i+1];
       						packet[j+1]= buffer3[i+2];

       						j=j+2;
							cc=cc+1;
       					 }
						i=i+1;

       					}

       					packet[j]= '\0';


       					fputs(packet,ft);
       					fputs("\n",ft);


        	strncpy(macAddress_parsed,packet+12,12);
        	macAddress_parsed[12]='\0';

        	int compare_check;
        	compare_check= strcmp(macAddress_parsed,macAddress_local);

        	if ( compare_check == 0 )

				{ /* Source MAC equal to the local MAC Address */
        		 /* This is the stack out direction */





        		// Insert the captured frame into ethernetOUT file
							fputs(packet,ft2);
							fputs("\n",ft2);

							buffer4= (char *) (malloc) (10000);
							network_protocol_type = (char *) malloc(4);

							strncpy(network_protocol_type, packet + (12*2),4);
// Insert the captured packet into IPOUT file in case it is IP packet TYPE
							if (strcmp(network_protocol_type,"0800")==0)
							{
							strcpy(buffer4, packet + (14*2));
							fputs(buffer4,ft4);
							fputs("\n",ft4);
							free (buffer4);
// copy transport protocol segment into transportOUT only if IP protocol is below
							buffer4= (char *) (malloc) (10000);
							strcpy(buffer4, packet + (14*2) + (20*2));
							fputs(buffer4,ft11);
						     fputs("\n",ft11);
					transProtocolType = (unsigned char *) malloc(2);
					strncpy(transProtocolType, packet + (14*2)+ 18,2);
					if (strcmp(transProtocolType,"06")==0)
					/** In case of TCP protocol */
					{
        	/** push into the _TCPOUT File */
        			fputs(buffer4,ft9);
        			fputs("\n",ft9);

        	/** push into the _TCPSOCKETOUT File */
        			fputs(buffer4+ (20*2),ft12);
        			fputs("\n",ft12);


        			free (buffer4);



					} // end of TCP
					else if (strcmp(transProtocolType,"11")==0)
					/** In case of UDP protocol */
					{
        	/** push into the _TCPUDP File */

        			fputs(buffer4,ft10);
        			fputs("\n",ft10);

        	/** push into the _UDPSOCKETOUT File */
        			fputs(buffer4 + (8*2),ft14);
        			fputs("\n",ft14);
        			free (buffer4);

					} // end of UDP

					else
					/** In case of neither UDP nor TCP protocol */
					{

printf("\n Packet is not either a UDP or TCP packet !!!!!! \n");
free (buffer4);

					}
			free(transProtocolType);

							} // end of if IP packet condition
							else
							{
			// Do nothing the packet is not an IP packet
printf("\n the packet is not an IP packet \n" );
							}
				} // end of the stack Out Direction



        			/** This is the stack IN direction */
else
        		{



        			// Insert the captured frame into ethernetIN file

        			fputs(packet,ft3);
        			fputs("\n",ft3);

        			buffer4= (char *) (malloc) (10000);
        			network_protocol_type = (char *) malloc(4);

					strncpy(network_protocol_type,packet + (12*2),4);
// Insert the captured packet into IPIN file in case it is IP packet TYPE
					if (strcmp(network_protocol_type,"0800")==0)
					{strcpy(buffer4, packet + (14*2));
        			fputs(buffer4,ft5);
        			fputs("\n",ft5);
        			free (buffer4);
// copy transport protocol segment into TCPIN or UDPIN only if IP protocol is below
					buffer4= (char *) (malloc) (10000);
					strcpy(buffer4, packet + (14*2) + (20*2));
					fputs(buffer4,ft6);
        			fputs("\n",ft6);
					transProtocolType = (unsigned char *) malloc(2);
					strncpy(transProtocolType, packet + (14*2)+ 18,2);
					if (strcmp(transProtocolType,"06")==0)
					/** In case of TCP protocol */
					{
		/** Push into the _TCPIN file */
        			fputs(buffer4,ft7);
        			fputs("\n",ft7);
        /** Push into the _TCPSOCKETIN file */
       				fputs(buffer4+ (20*2),ft13);
        			fputs("\n",ft13);
        			free (buffer4);

					} // end of TCP
					else if (strcmp(transProtocolType,"11")==0)
					/** In case of UDP protocol */
					{
    /** Push into the _UDPIN file */
		 			fputs(buffer4,ft8);
        			fputs("\n",ft8);
    /** Push into the _UDPSOCKETIN file */
       				fputs(buffer4+ (8*2),ft15);
        			fputs("\n",ft15);

        			free (buffer4);

					} // end of UDP

					else
					/** In case of neither UDP nor TCP protocol */
					{

printf("\n Error it is not either a UDP or TCP packet !!!!!! \n");
free (buffer4);

					}

					}  // end of if checking if it is IP packets
					else
					{
// Do nothing the packet is not an IP packet
printf("\n the packet is not an IP packet \n");
					}
free(transProtocolType);



        		}  // End of the STACK IN direction else statement



        	counter++;
        }  // end of while loop to read the source file



        fclose ( fs );
        fclose (ft);
        fclose (ft2);
        fclose (ft3);
        fclose (ft4);
        fclose (ft5);
      //  fclose (ft6);
        fclose (ft7);
        fclose (ft8);
        free (buffer);
        free (buffer2);
        free (buffer3);

        free (packet);

        return(1);

} // end of the main function

