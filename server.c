#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>    //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <fcntl.h>
#define TRUE 1
#define FALSE 0
#define PORT 9897
#define number_of_client 5
#define max_of_collaborators 4
#define number_of_file_a_client_can_upload 8
#define max 20000
#define max1 90000
int opt = TRUE;
int master_socket, addrlen, new_socket, client_socket[number_of_client], max_clients = number_of_client, activity, i, valread, sd;
int max_sd;
struct sockaddr_in address;
char buffer[1025];
fd_set readfds;
int counter = 0;

struct client_info
{
    int id;
    int socket_id;
    struct file_data
    {
        char filename[100];
        // representing the permission of collaborator
        int collaborator[4];
        char permission[4];
        int ownner;
        int linecount;

    } file[number_of_file_a_client_can_upload];

    // char file[number_of_file_a_client_can_upload][1048];
} clients[5];

void print_error(char *msg)
{
    perror(msg);
    exit(0);
}
void print_msg(char *msg)
{
    puts(msg);
}

void check_id(char *client_ids, int *flag, int n)
{
    for (int i = 0; i < n; i++)
    {
        if (client_ids[i] >= '0' && client_ids[i] <= '9')
        {
            continue;
        }
        else if (*flag == 1)
            *flag = 1;
    }
}
void check_num(char *client_ids, int *flag, int n)
{
    for (int i = 0; i < n; i++)
    {
        if (client_ids[i] >= '0' && client_ids[i] <= '9')
        {
            continue;
        }
        else if (client_ids[0] == '-')
        {
            continue;
        }
        else
        {
            if (*flag == 1)
            {
                *flag = 1;
            }
            else
                *flag = 0;
        }
    }
}
void check_field(char *field, int *flag)
{

    if ((field[0] == 'V') || (field[0] == 'E'))
    {

        if (*flag == 1)
        {
            *flag = 1;
        }
        else
            *flag = 0;
    }
    else
    {
        *flag = 1;
    }
}
int checkfile_valid_msg(char message[], char index_value[], int *flag)
{
    int r = 0;
    if (index_value[0] == '\"' && ((index_value[strlen(index_value) - 1] == '\"') || (message[strlen(message) - 1] == '\"')))
    {
        if (*flag == 1)
        {
            *flag = 1;
        }
        else
        {
            *flag = 0;
            r = 1;
        }
    }
    else if (message[0] == '\"' && message[strlen(message) - 1] == '\"')
    {

        check_num(index_value, flag, strlen(index_value));
        if (*flag == 1)
        {
            *flag = 1;
        }
        else
        {
            *flag = 0;
            r = 3;
        }
    }

    return r;
}
void checkfile_name(char *filename, int *flag)
{
    int k = strlen(filename) - 1;
    if ((strlen(filename) > 4) && (filename[k] == 't') && (filename[k - 1] == 'x') && (filename[k - 2] == 't') && (filename[k - 3] == '.'))
    {
        if (*flag == 1)
        {
            *flag = 1;
        }
        else
            *flag = 0;
    }
    else
        *flag = 1;
}
void check_for_ownner(char *file, int clnt_id, int *newflag)
{

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < number_of_file_a_client_can_upload; j++)
        {
            if ((strcmp(clients[i].file[j].filename, file) == 0) && (clients[i].file[j].ownner == clnt_id))
            {
                *newflag = 1;
            }
        }
    }
}
void check_for_client_id(char *file, int clnt_id, int *newflag)
{
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < number_of_file_a_client_can_upload; j++)
        {
            if ((strcmp(clients[i].file[j].filename, file) == 0))
            {
                if (*newflag == 1)
                {
                    *newflag = 1;
                }
                else
                    *newflag = 2;
            }
        }
    }
    int cg=0;
    for (int i = 0; i < 5; i++)
    {
        if (clients[i].id == clnt_id)
        {
            cg++;
            if (*newflag == 1)
            {
                *newflag = 1;
            }
            if (*newflag == 2)
            {
                *newflag = 2;
            }
            
        }
    }
    if(cg==0)
    {
        *newflag =8;
    }
}
void check_for_present_socket_id(char *file, int clnt_id, int *newflag, int *present_client_id)
{

    for (int i = 0; i < 5; i++)
    {
        if (clients[i].socket_id == sd)
        {
            *present_client_id = clients[i].id;
        }
    }
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < number_of_file_a_client_can_upload; j++)
        {
            if ((strcmp(clients[i].file[j].filename, file) == 0) && (clients[i].file[j].ownner == *present_client_id))
            {
                *newflag = 5;
            }
        }
    }
}
void check_for_collaborator(char *file, int clnt_id, int *newflag)
{

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < number_of_file_a_client_can_upload; j++)
        {
            for (int z = 0; z < 4; z++)
            {
                if ((clients[i].file[j].collaborator[z] == clnt_id) && (strcmp(clients[i].file[j].filename, file) == 0))
                {
                    if (*newflag == 1)
                    {
                        *newflag = 1;
                    }
                    if (*newflag == 2)
                    {
                        *newflag = 2;
                    }
                    else
                    {
                        *newflag = 9;
                    }
                }
            }
        }
    }
}
void check_for_permission(char *file, int clnt_id, int *newflag)
{

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < number_of_file_a_client_can_upload; j++)
        {
            for (int z = 0; z < 4; z++)
            {
                if ((clients[i].file[j].collaborator[z] == clnt_id) && (strcmp(clients[i].file[j].filename, file) == 0) && (clients[i].file[j].permission[z] == 'E'))
                {
                    if (*newflag == 1)
                    {
                        *newflag = 1;
                    }
                    if (*newflag == 2)
                    {
                        *newflag = 2;
                    }
                    else
                    {
                        *newflag = 9;
                    }
                }
            }
        }
    }
}
int get_line_count(char *finename)
{
    int k;
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < number_of_file_a_client_can_upload; j++)
        {
            if (strcmp(finename, clients[i].file[j].filename) == 0)
            {
                k = clients[i].file[j].linecount;
            }
        }
    }
    return k;
}

void getfile(char *file1, int socketv_id)
{
    FILE *fp3;
    fp3 = fopen(file1, "w");
    fseek(fp3, 0, 0);
    bzero(buffer, sizeof(buffer));
    while (strcmp(buffer, "EOD") != 0)
    {
        bzero(buffer, sizeof(buffer));
        if (recv(socketv_id, buffer, sizeof(buffer), 0) < 0)
        {
            print_error("error in recieving data");
        }
        if (strcmp(buffer, "EOD") == 0)
        {
            break;
        }
        fseek(fp3, 0, 1);
        fputs(buffer, fp3);
        usleep(9004);
    }
    fclose(fp3);
}
void sendfile(char *file1, int socketv_id)
{
    FILE *fp3;
    fp3 = fopen(file1, "r");
    if (fp3 == NULL)
    {
        printf("file doesnot exits\n");
    }
    fseek(fp3, 0, 0);
    bzero(buffer, strlen(buffer));
    while (fgets(buffer, sizeof(buffer), fp3) != NULL)
    {
        if (send(socketv_id, buffer, strlen(buffer), 0) < 0)
        {
            print_error("error in recieving data");
        }
        bzero(buffer, strlen(buffer));
        usleep(9004);
    }
    bzero(buffer, strlen(buffer));
    sprintf(buffer, "%s", "EOD");
    if (send(socketv_id, buffer, strlen(buffer), 0) < 0)
    {
        print_error("error in recieving data");
    }
    bzero(buffer, strlen(buffer));
    fclose(fp3);
}
int find_num_of_lines_file(char *FILENAME)
{
    int line_count = 0;
    char ch;
    FILE *fp = fopen(FILENAME, "r");
    if (fp == NULL)
    {
        printf("File \"%s\" does not exist!!!\n", FILENAME);
        exit(0);
    }
    for (ch = getc(fp); ch != EOF; ch = getc(fp))
    {
        if (ch == '\n')
        {
            line_count++;
        }
    }
    line_count++;
    fclose(fp);
    return line_count;
}
void update_line_count(char *finename)
{

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < number_of_file_a_client_can_upload; j++)
        {
            if (strcmp(finename, clients[i].file[j].filename) == 0)
            {
                clients[i].file[j].linecount = find_num_of_lines_file(finename);
            }
        }
    }
    printf("line count of file incremented\n");
}
void buff_send(char *filename1, FILE *fp2)
{
    char maxbuff[max1];
    fp2 = fopen(filename1, "r");
    fseek(fp2, 0L, SEEK_END);
    long numbytes = ftell(fp2);
    fseek(fp2, 0L, SEEK_SET);
    bzero(maxbuff, strlen(maxbuff));
    fread(maxbuff, sizeof(char), numbytes, fp2);
    printf("%s\n", maxbuff);
    if (strlen(maxbuff) == 0)
    {
        sprintf(maxbuff, "%s", "file is empty !!");
    }
    int ni = send(sd, maxbuff, strlen(maxbuff), 0);
    if (ni < 0)
    {
        print_error("Error in writing to client, Please try again...process exiting");
    }
    printf("file information sended to client\n");
    fclose(fp2);
}
int user_cmd_check(char *msg, int nv)
{
    int flag = 0;
    char command[100] = {0};
    char filename1[1000] = {0};
    char permission[10] = {0};
    char clientId[10] = {0};
    char extra[21] = {0};
    char str_indx[4] = {0};
    char end_indx[4] = {0};
    char message[1002] = {0};
    if (strncmp(msg, "/users", 6) == 0)
    {
        sscanf(msg, "%s %s", command, filename1);
        if (strlen(command) == 6)
        {
            if (filename1[0] == '\0')
            {
                if (flag == 0)
                {
                    int count;
                    bzero(buffer, strlen(buffer));
                    sprintf(buffer, "%d", counter);
                    if (send(sd, buffer, strlen(buffer), 0) != strlen(buffer))
                    {
                        print_error("send error\n");
                    }

                    for (int i = 0; i < 5; i++)
                    {
                        if (clients[i].id != 0)
                        {
                            printf(" client id: %d,socket id : %d\n", clients[i].id, clients[i].socket_id);
                            fflush(stdout);
                            count = i + 1;
                            bzero(buffer, strlen(buffer));
                            sprintf(buffer, "client : %d  ID : %d", count, clients[i].id);
                            if (send(sd, buffer, strlen(buffer), 0) != strlen(buffer))
                            {
                                print_error("not able to send msg to client error\n");
                            }
                        }
                    }
                }
                else
                    flag = 1;
            }
            else
                flag = 1;
        }
        else
        {
            flag = 1;
        }
    }
    else if (strncmp(msg, "/files", 6) == 0)
    {
        sscanf(msg, "%s %s", command, filename1);
        if (strcmp(command, "/files") == 0)
        {
            if (filename1[0] == '\0')
            {
                if (flag == 0)
                {
                    printf("inside files\n");
                    FILE *nfp;
                    nfp = tmpfile();
                    int fcount = 0;
                    int ccount = 0;
                    char temp1[max];
                    for (int i = 0; i < 5; i++)
                    {
                        for (int k = 0; k < number_of_file_a_client_can_upload; k++)
                        {
                            fcount = 0;
                            if (clients[i].file[k].filename[0] != '\0')
                            {
                                ccount++;
                                fprintf(nfp, "filename: %s :: Ownner_ID: %d :: linecount: %d \n", clients[i].file[k].filename, clients[i].file[k].ownner, clients[i].file[k].linecount);
                                for (int z = 0; z < 4; z++)
                                {
                                    if (clients[i].file[k].collaborator[z] != 0)
                                    {
                                        fcount++;
                                        fprintf(nfp, "collaborator_ID : %d :: permission %c \n", clients[i].file[k].collaborator[z], clients[i].file[k].permission[z]);
                                    }
                                }
                                if (fcount == 0)
                                {
                                    fprintf(nfp, "No collaborators for this file\n");
                                }
                            }
                        }
                    }
                    if (ccount == 0)
                    {
                        fprintf(nfp, "No File Records Found!!!\n");
                    }
                    rewind(nfp);
                    fseek(nfp, 0L, SEEK_END);
                    long numbytes = ftell(nfp);
                    fseek(nfp, 0L, SEEK_SET);
                    bzero(temp1, strlen(temp1));
                    fread(temp1, sizeof(char), numbytes, nfp);
                    printf("%s\n", temp1);
                    int ni = send(sd, temp1, strlen(temp1), 0);
                    if (ni < 0)
                    {
                        print_error("Error in writing to client, Please try again...process exiting");
                    }
                    printf("file information sended to client\n");
                    fclose(nfp);
                }
            }
            else
                flag = 1;
        }
        else
        {
            flag = 1;
        }
    }
    else if (strncmp(msg, "/upload", 7) == 0)
    {
        sscanf(msg, "%s %s %s", command, filename1, permission);
        if (strcmp(command, "/upload") == 0)
        {
            if (filename1[0] != '\0' && permission[0] == '\0')
            {
                checkfile_name(filename1, &flag);
                if (flag == 0)
                {
                    FILE *fp;
                    fp = fopen(filename1, "r");
                    if (fp == NULL)

                    {

                        bzero(buffer, strlen(buffer));
                        sprintf(buffer, "%s", "server is ready ...\n please wait untill upload");
                        if (send(sd, buffer, strlen(buffer), 0) != strlen(buffer))
                        {
                            print_error("send error\n");
                        }

                        getfile(filename1, sd);
                        int ncount = find_num_of_lines_file(filename1);
                        printf("file successfully uploaded in server\n");
                        for (int i = 0; i < 5; i++)
                        {
                            if (clients[i].socket_id == sd)
                            {
                                for (int k = 0; k < number_of_file_a_client_can_upload; k++)
                                {
                                    if (clients[i].file[k].filename[0] == '\0')
                                    {
                                        strcpy(clients[i].file[k].filename, filename1);
                                        clients[i].file[k].linecount = ncount;
                                        clients[i].file[k].ownner = clients[i].id;
                                        break;
                                    }
                                }
                            }
                        }

                        printf("copy of file is made at server side \n");
                    }
                    else
                    {
                        bzero(buffer, strlen(buffer));
                        sprintf(buffer, "%s", "samefile name exits at server ");
                        if (send(sd, buffer, strlen(buffer), 0) != strlen(buffer))
                        {
                            print_error("send error\n");
                        }
                        fclose(fp);
                    }
                }
                else
                    flag = 1;
            }
            else
                flag = 1;
        }
        else
        {
            flag = 1;
        }
    }
    else if (strncmp(msg, "/download", 9) == 0)
    {
        sscanf(msg, "%s %s %s", command, filename1, permission);
        if (strcmp(command, "/download") == 0)
        {
            if (filename1[0] != '\0', permission[0] == '\0')
            {
                checkfile_name(filename1, &flag);
                if (flag == 0)
                {
                    FILE *fp;
                    printf("inside download\n");
                    int fg = 0;
                    int present_id;
                    check_for_present_socket_id(filename1, 0, &fg, &present_id);
                    check_for_collaborator(filename1, present_id, &fg);
                    if (fg == 5 || fg == 9)
                    {
                        sendfile(filename1, sd);
                        printf("successfully sended the file to client \n");
                    }
                    else
                    {
                        bzero(buffer, sizeof(buffer));
                        sprintf(buffer, "%s", "NOT authorised to download this file only ownner and collaborators can download file");
                        if (send(sd, buffer, strlen(buffer), 0) < 0)
                        {
                            print_error("[-]Error in receiving data.\n");
                        }
                    }
                }
                else
                    flag = 1;
            }
            else
                flag = 1;
        }
        else
        {
            flag = 1;
        }
    }
    else if (strncmp(msg, "/invite", 7) == 0)
    {
        sscanf(msg, "%s %s %s %s %s", command, filename1, clientId, permission, extra);
        if (strcmp(command, "/invite") == 0)
        {

            if ((filename1[0] != '\0') && (clientId[0] != '\0') && (permission[0] != '\0') && extra[0] == '\0')
            {
                int fg = 0;
                checkfile_name(filename1, &flag);
                check_id(clientId, &flag, strlen(clientId));
                check_field(permission, &flag);
                check_for_ownner(filename1, atoi(clientId), &fg);
                check_for_client_id(filename1, atoi(clientId), &fg);
                printf("the value of fg %d",fg);
                if (flag == 0)
                {
                    if (fg == 1)
                    {
                        printf("inside invite\n");
                        bzero(buffer, strlen(buffer));
                        sprintf(buffer, "Given client_id is ownner of the file");
                        if (send(sd, buffer, strlen(buffer), 0) != strlen(buffer))
                        {
                            print_error("send error\n");
                        }
                    }
                    else if (fg == 2)
                    {
                        printf("inside invite\n");
                        int present_id;
                        check_for_present_socket_id(filename1, atoi(clientId), &fg, &present_id);
                        check_for_collaborator(filename1, atoi(clientId), &fg);
                        if (fg == 5)
                        {
                            bzero(buffer, strlen(buffer));
                            int id = atoi(clientId);
                            sprintf(buffer, "Invite from client_Id %d do you wanted to accept it?", present_id);
                            for (int i = 0; i < 5; i++)
                            {
                                if (clients[i].id == id)
                                {

                                    printf("client id matched is %d and socket id is %d \n", id, clients[i].socket_id);
                                    id = clients[i].socket_id;
                                }
                            }

                            if (send(id, buffer, strlen(buffer), 0) != strlen(buffer))
                            {
                                print_error("send error\n");
                            }

                            bzero(buffer, sizeof(buffer));
                            int nl = recv(id, buffer, sizeof(buffer), 0);
                            if (nl < 0)
                            {
                                print_error("error in recieving \n");
                            }
                            if (strncmp(buffer, "yes", 3) == 0)
                            {
                                for (int i = 0; i < 5; i++)
                                {
                                    for (int j = 0; j < number_of_file_a_client_can_upload; j++)
                                    {
                                        if ((strcmp(clients[i].file[j].filename, filename1) == 0))
                                        {
                                            for (int z = 0; z < 4; z++)
                                            {
                                                if (clients[i].file[j].collaborator[z] == 0)
                                                {
                                                    clients[i].file[j].collaborator[z] = atoi(clientId);
                                                    clients[i].file[j].permission[z] = permission[0];
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                                bzero(buffer, strlen(buffer));
                                sprintf(buffer, "client accepted the invitation");
                                if (send(sd, buffer, strlen(buffer), 0) != strlen(buffer))
                                {
                                    print_error("send error\n");
                                }
                                printf("successfully recieve the response\n");
                            }
                            else
                            {
                                bzero(buffer, strlen(buffer));
                                sprintf(buffer, "client rejected the invitation");
                                if (send(sd, buffer, strlen(buffer), 0) != strlen(buffer))
                                {
                                    print_error("send error\n");
                                }
                                printf("successfully recieve the response\n");
                            }
                        }
                        else if (fg == 9)
                        {
                            bzero(buffer, strlen(buffer));
                            sprintf(buffer, " Already Collaborator for this file");
                            if (send(sd, buffer, strlen(buffer), 0) != strlen(buffer))
                            {
                                print_error("send error\n");
                            }
                        }
                        else
                        {

                            bzero(buffer, strlen(buffer));
                            sprintf(buffer, "Not Authorised To Send Invitation ,only ownner of file can send invitation");
                            if (send(sd, buffer, strlen(buffer), 0) != strlen(buffer))
                            {
                                print_error("send error\n");
                            }
                        }
                    }
                    else
                    {
                        printf("inside invite\n");
                        bzero(buffer, strlen(buffer));
                        sprintf(buffer, "filename not found or client _ID id wrong");
                        if (send(sd, buffer, strlen(buffer), 0) != strlen(buffer))
                        {
                            print_error("send error\n");
                        }
                    }
                }
            }
            else
                flag = 1;
        }
        else
        {
            flag = 1;
        }
    }
    else if (strncmp(msg, "/read", 5) == 0)
    {
        sscanf(msg, "%s %s %s %s %s", command, filename1, str_indx, end_indx, extra);
        if (strcmp(command, "/read") == 0)
        {
            if (filename1[0] != '\0' && extra[0] == '\0')
            {
                checkfile_name(filename1, &flag);

                FILE *fp2;
                FILE *tmp;

                if (flag == 0)
                {
                    int fg = 0;
                    int present_id;
                    check_for_present_socket_id(filename1, 0, &fg, &present_id);
                    check_for_collaborator(filename1, present_id, &fg);

                    int lcount = get_line_count(filename1);
                    int low = atoi(str_indx);
                    int high = atoi(end_indx);
                    char maxbuff[max1];
                    int k = 0;
                    int signvalue = 9;
                    int nvalue = 8;
                    int pvalue = 7;
                    int jp = 3;

                    if (fg == 5 || fg == 9)
                    {
                        printf("inside read\n");
                        if (str_indx[0] != '\0')
                        {
                            if (low >= 0 && low < lcount)
                            {
                                printf("starting index is positive \n");
                                signvalue = 0;
                            }
                            else if (low >= -lcount && low < 0)
                            {
                                low = low + lcount;
                                printf("starting index is negaive\n");
                                nvalue = 0;
                            }
                            else
                            {
                                k = 1;
                            }
                        }
                        if (end_indx[0] != '\0')
                        {
                            if (high >= 0 && high < lcount)
                            {
                                printf("starting index is positive \n");
                                pvalue = 0;
                            }
                            else if (high >= -lcount && high < 0)
                            {
                                high = high + lcount;
                                printf("starting index is negaive\n");
                                jp = 0;
                            }
                            else
                            {
                                k = 1;
                            }
                        }
                        if (k == 0)
                        {
                            if (str_indx[0] == '\0')
                            {
                                fp2 = fopen(filename1, "r");
                                fseek(fp2, 0L, SEEK_END);
                                long numbytes = ftell(fp2);
                                fseek(fp2, 0L, SEEK_SET);
                                bzero(maxbuff, strlen(maxbuff));
                                fread(maxbuff, sizeof(char), numbytes, fp2);
                                printf("%s\n", maxbuff);
                                if (strlen(maxbuff) == 0)
                                {
                                    sprintf(maxbuff, "%s", "file is empty !!");
                                }
                                int ni = send(sd, maxbuff, strlen(maxbuff), 0);
                                if (ni < 0)
                                {
                                    print_error("Error in writing to client, Please try again...process exiting");
                                }
                                printf("file information sended to client\n");
                                fclose(fp2);
                            }
                            else if (end_indx[0] == '\0')
                            {
                                fp2 = fopen(filename1, "r");
                                char line[1024];
                                k = 0;
                                while (fgets(line, sizeof(line), fp2) != NULL)
                                {

                                    if (k == (low))
                                    {
                                        bzero(buffer, sizeof(buffer));
                                        sprintf(buffer, "%s", line);
                                        if (strlen(buffer) == 0)
                                        {
                                            sprintf(buffer, "%s", "line is empty !!");
                                        }
                                        if (send(sd, buffer, strlen(buffer), 0) < 0)
                                        {
                                            print_error("[-]Error in receiving data.\n");
                                        }
                                        break;
                                    }
                                    k++;
                                }
                                fclose(fp2);
                            }
                            else
                            {
                                if ((((signvalue == 0) && (pvalue == 0)) || ((nvalue == 0) && (jp == 0))) && (high >= low))
                                {
                                    fp2 = fopen(filename1, "r");
                                    tmp = tmpfile();
                                    char line[1024];
                                    int countv = 0;
                                    while (fgets(line, sizeof(line), fp2))
                                    {

                                        if (countv >= low && countv <= high)
                                        {
                                            fputs(line, tmp);
                                            fputs("\n", tmp);
                                            fseek(tmp, 0, 1);
                                        }
                                        countv++;
                                    }
                                    rewind(tmp);
                                    fseek(tmp, 0L, SEEK_END);
                                    long numbytes = ftell(tmp);
                                    fseek(tmp, 0L, SEEK_SET);
                                    bzero(maxbuff, strlen(maxbuff));
                                    fread(maxbuff, sizeof(char), numbytes, tmp);
                                    printf("%s\n", maxbuff);
                                    if (strlen(maxbuff) == 0)
                                    {
                                        sprintf(maxbuff, "%s", "line is empty !!");
                                    }
                                    int ni = send(sd, maxbuff, strlen(maxbuff), 0);
                                    if (ni < 0)
                                    {
                                        print_error("Error in writing to client, Please try again...process exiting");
                                    }
                                    printf("file information sended to client\n");
                                    fclose(fp2);
                                    fclose(tmp);
                                }
                                else
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "%s", "NOT possible to read index values are in correct");
                                    if (send(sd, buffer, strlen(buffer), 0) < 0)
                                    {
                                        print_error("[-]Error in receiving data.\n");
                                    }
                                }
                            }
                        }
                        else
                        {
                            bzero(buffer, sizeof(buffer));
                            sprintf(buffer, "%s", "NOT possible to read file since index is out of range");
                            if (send(sd, buffer, strlen(buffer), 0) < 0)
                            {
                                print_error("[-]Error in receiving data.\n");
                            }
                        }
                    }
                    else
                    {
                        bzero(buffer, sizeof(buffer));
                        sprintf(buffer, "%s", "NOT authorised to read this file only ownner and collaborators can read this file");
                        if (send(sd, buffer, strlen(buffer), 0) < 0)
                        {
                            print_error("[-]Error in receiving data.\n");
                        }
                    }
                }
                else
                    flag = 1;
            }
            else
                flag = 1;
        }

        else
        {
            flag = 1;
        }
    }
    else if (strncmp(msg, "/insert", 7) == 0)
    {
        char index_value[1000] = {0};
        sscanf(msg, "%s %s %s %[^\n]", command, filename1, index_value, message);
        printf("%s,%s", index_value, message);
        if (strcmp(command, "/insert") == 0)
        {
            if (filename1[0] != '\0')
            {

                checkfile_name(filename1, &flag);
                int r = checkfile_valid_msg(message, index_value, &flag);

                if (flag == 0)
                {
                    int fg = 0;
                    int present_id;
                    check_for_present_socket_id(filename1, 0, &fg, &present_id);
                    check_for_permission(filename1, present_id, &fg);
                    FILE *fp;
                    FILE *tmp;
                    int low = atoi(index_value);
                    int lcount = get_line_count(filename1);
                    int k = 0;
                    printf("inside insert\n");
                    if (fg == 5 || fg == 9)
                    {
                        if (index_value[0] != '\0')
                        {
                            if (low >= 0 && low < lcount)
                            {
                                printf("starting index is positive \n");
                            }
                            else if (low >= -lcount && low < 0)
                            {
                                low = low + lcount;
                                printf("starting index is negaive\n");
                            }
                            else
                            {
                                k = 1;
                            }
                        }
                        if (r == 1)
                        {
                            if (message[0] != '\0')
                            {
                                strcat(index_value, " ");
                                strcat(index_value, message);
                            }
                            char mssg[1000];
                            char temper[10];
                            bzero(mssg, sizeof(mssg));
                            strcat(mssg, "\n");
                            for (int j = 1; j <= strlen(index_value) - 2; j++)
                            {
                                if (index_value[j] == 92 && index_value[j + 1] == 'n')
                                {
                                    strcat(mssg, "\n");
                                    ++j;
                                }
                                else
                                {
                                    bzero(temper, sizeof(temper));
                                    sprintf(temper, "%c", index_value[j]);
                                    strcat(mssg, temper);
                                }
                            }
                            fp = fopen(filename1, "a");
                            fseek(fp, 0, 1);
                            fputs(mssg, fp);
                            fclose(fp);
                            bzero(buffer, sizeof(buffer));
                            sprintf(buffer, "%s", "sucessfully modify the file with msg");
                            if (send(sd, buffer, strlen(buffer), 0) < 0)
                            {
                                print_error("[-]Error in receiving data.\n");
                            }
                            update_line_count(filename1);
                            buff_send(filename1, fp);
                        }
                        else if (r == 3 && k == 0)
                        {
                            printf("msg value is: %s\n", message);
                            char mssg[1000];
                            char temper[10];
                            bzero(mssg, sizeof(mssg));
                            // strcat(mssg, "\n");
                            for (int j = 1; j <= strlen(message) - 2; j++)
                            {
                                if (message[j] == 92 && message[j + 1] == 'n')
                                {
                                    strcat(mssg, "\n");
                                    ++j;
                                }
                                else
                                {
                                    bzero(temper, sizeof(temper));
                                    sprintf(temper, "%c", message[j]);
                                    strcat(mssg, temper);
                                }
                            }
                            FILE *fp5;
                            FILE *tmps;
                            fp5 = fopen(filename1, "r");
                            tmps = tmpfile();
                            char line[1000];
                            bzero(line, sizeof(line));
                            int cuntv = 0;
                            while (fgets(line, sizeof(line), fp5) != NULL)
                            {

                                if (cuntv == low)
                                {
                                    fputs(mssg, tmps);
                                    fputs("\n", tmps);
                                    fseek(tmps, 0, 1);
                                }
                                fputs(line, tmps);
                                fseek(fp5, 0, 1);
                                fseek(tmps, 0, 1);
                                line[0] = '\0';
                                cuntv++;
                            }
                            fclose(fp5);
                            fp5 = fopen(filename1, "w");
                            rewind(tmps);
                            line[0] = '\0';
                            while (fgets(line, sizeof(line), tmps) != NULL)
                            {
                                fputs(line, fp5);
                                fseek(fp5, 0, 1);
                                fseek(tmps, 0, 1);
                                line[0] = '\0';
                            }
                            fclose(tmps);
                            fclose(fp5);
                            bzero(buffer, sizeof(buffer));
                            sprintf(buffer, "%s", "sucessfully modify the file with msg");
                            if (send(sd, buffer, strlen(buffer), 0) < 0)
                            {
                                print_error("[-]Error in receiving data.\n");
                            }
                            update_line_count(filename1);
                            buff_send(filename1, fp);
                        }
                        else if (k == 1)
                        {
                            bzero(buffer, sizeof(buffer));
                            sprintf(buffer, "%s", "NOT possible to insert file since index is out of range");
                            if (send(sd, buffer, strlen(buffer), 0) < 0)
                            {
                                print_error("[-]Error in receiving data.\n");
                            }
                        }
                        else
                        {
                            bzero(buffer, sizeof(buffer));
                            sprintf(buffer, "%s", "NOT possible to insert in the given file - unsuccesful");
                            if (send(sd, buffer, strlen(buffer), 0) < 0)
                            {
                                print_error("[-]Error in receiving data.\n");
                            }
                        }
                    }
                    else
                    {
                        bzero(buffer, sizeof(buffer));
                        sprintf(buffer, "%s", "NOT authorised to modify this file\n only ownner and collaborators  with 'E' permission can modify the file");
                        if (send(sd, buffer, strlen(buffer), 0) < 0)
                        {
                            print_error("[-]Error in receiving data.\n");
                        }
                    }
                }
                else
                    flag = 1;
            }
            else
                flag = 1;
        }

        else
        {
            flag = 1;
        }
    }
    else if (strncmp(msg, "/delete", 7) == 0)
    {

        sscanf(msg, "%s %s %s %s %s", command, filename1, str_indx, end_indx, extra);
        if (strcmp(command, "/delete") == 0)
        {
            if (filename1[0] != '\0' && extra[0] == '\0')
            {
                checkfile_name(filename1, &flag);

                FILE *fp2;
                FILE *tmp;

                if (flag == 0)
                {
                    int fg = 0;
                    int present_id;
                    check_for_present_socket_id(filename1, 0, &fg, &present_id);
                    check_for_permission(filename1, present_id, &fg);

                    int lcount = get_line_count(filename1);
                    int low = atoi(str_indx);
                    int high = atoi(end_indx);
                    char maxbuff[max1];
                    int k = 0;
                    int signvalue = 9;
                    int nvalue = 8;
                    int pvalue = 7;
                    int jp = 3;

                    if (fg == 5 || fg == 9)
                    {
                        printf("inside delete\n");
                        if (str_indx[0] != '\0')
                        {
                            if (low >= 0 && low < lcount)
                            {
                                printf("starting index is positive \n");
                                signvalue = 0;
                            }
                            else if (low >= -lcount && low < 0)
                            {
                                low = low + lcount;
                                printf("starting index is negaive\n");
                                nvalue = 0;
                            }
                            else
                            {
                                k = 1;
                            }
                        }
                        if (end_indx[0] != '\0')
                        {
                            if (high >= 0 && high < lcount)
                            {
                                printf("starting index is positive \n");
                                pvalue = 0;
                            }
                            else if (high >= -lcount && high < 0)
                            {
                                high = high + lcount;
                                printf("starting index is negaive\n");
                                jp = 0;
                            }
                            else
                            {
                                k = 1;
                            }
                        }
                        if (k == 0)
                        {
                            if (str_indx[0] == '\0')
                            {
                                fp2 = fopen(filename1, "w");
                                fclose(fp2);
                                bzero(buffer, sizeof(buffer));
                                sprintf(buffer, "Deleted  all content of %s", filename1);
                                int ni = send(sd, buffer, strlen(buffer), 0);
                                if (ni < 0)
                                {
                                    print_error("Error in writing to client, Please try again...process exiting");
                                }
                                printf("file information\n");
                                update_line_count(filename1);
                                buff_send(filename1, fp2);
                            }
                            else if (end_indx[0] == '\0')
                            {
                                fp2 = fopen(filename1, "r");
                                char line[1024];
                                bzero(line, sizeof(line));
                                tmp = tmpfile();
                                k = 0;
                                char *t;
                                while (fgets(line, sizeof(line), fp2) != NULL)
                                {
                                    if ((k == low - 1) && (low == lcount - 1))
                                    {
                                        t = strtok(line, "\n");
                                        fputs(line, tmp);
                                        k++;
                                        continue;
                                    }
                                    else if (k == (low))
                                    {

                                        line[0] = '\0';
                                    }
                                    fputs(line, tmp);
                                    printf("%s", line);
                                    fseek(tmp, 0, 1);
                                    line[0] = '\0';
                                    k++;
                                }
                                fclose(fp2);
                                fp2 = fopen(filename1, "w");
                                rewind(tmp);
                                line[0] = '\0';

                                while (fgets(line, sizeof(line), tmp) != NULL)
                                {
                                    fputs(line, fp2);
                                    fseek(fp2, 0, 1);
                                    fseek(tmp, 0, 1);
                                    line[0] = '\0';
                                }
                                fclose(tmp);
                                fclose(fp2);
                                bzero(buffer, sizeof(buffer));
                                sprintf(buffer, "Deleted content the given line index %d", low);
                                int ni = send(sd, buffer, strlen(buffer), 0);
                                if (ni < 0)
                                {
                                    print_error("Error in writing to client, Please try again...process exiting");
                                }
                                printf("delete successfull\n");
                                update_line_count(filename1);
                                buff_send(filename1, fp2);
                            }
                            else
                            {
                                if ((((signvalue == 0) && (pvalue == 0)) || ((nvalue == 0) && (jp == 0))) && (high >= low))
                                {
                                    fp2 = fopen(filename1, "r");
                                    tmp = tmpfile();
                                    char line[1024];
                                    int countv = 0;
                                    char *t;
                                    while (fgets(line, sizeof(line), fp2))
                                    {
                                        if ((countv == low - 1) && (high == lcount - 1))
                                        {
                                            t = strtok(line, "\n");
                                            fputs(line, tmp);
                                            countv++;
                                            continue;
                                        }
                                        else if (countv >= low && countv <= high)
                                        {
                                            line[0] = '\0';
                                        }
                                        fputs(line, tmp);
                                        printf("%s", line);
                                        fseek(tmp, 0, 1);
                                        line[0] = '\0';
                                        countv++;
                                    }
                                    fclose(fp2);
                                    rewind(tmp);
                                    fp2 = fopen(filename1, "w");
                                    rewind(tmp);
                                    line[0] = '\0';

                                    while (fgets(line, sizeof(line), tmp) != NULL)
                                    {
                                        fputs(line, fp2);
                                        fseek(fp2, 0, 1);
                                        fseek(tmp, 0, 1);
                                        line[0] = '\0';
                                    }

                                    fclose(fp2);
                                    fclose(tmp);
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "Deleted content of line index %d to %d", low, high);
                                    int ni = send(sd, buffer, strlen(buffer), 0);
                                    if (ni < 0)
                                    {
                                        print_error("Error in writing to client, Please try again...process exiting");
                                    }
                                    printf("delete successfull\n");
                                    update_line_count(filename1);
                                    buff_send(filename1, fp2);
                                }
                                else
                                {
                                    bzero(buffer, sizeof(buffer));
                                    sprintf(buffer, "%s", "NOT possible to delete_index values are in correct");
                                    if (send(sd, buffer, strlen(buffer), 0) < 0)
                                    {
                                        print_error("[-]Error in receiving data.\n");
                                    }
                                }
                            }
                        }
                        else
                        {
                            bzero(buffer, sizeof(buffer));
                            sprintf(buffer, "%s", "NOT possible to delete_file -error");
                            if (send(sd, buffer, strlen(buffer), 0) < 0)
                            {
                                print_error("[-]Error in receiving data.\n");
                            }
                        }
                    }
                    else
                    {
                        bzero(buffer, sizeof(buffer));
                        sprintf(buffer, "%s", "NOT authorised to delete this file only ownner and collaborators can read this file");
                        if (send(sd, buffer, strlen(buffer), 0) < 0)
                        {
                            print_error("[-]Error in receiving data.\n");
                        }
                    }
                }
                else
                    flag = 1;
            }
            else
                flag = 1;
        }

        else
        {
            flag = 1;
        }
    }
    else if (strncmp(buffer, "/exit", 5) == 0)
    {
        getpeername(sd, (struct sockaddr *)&address,
                    (socklen_t *)&addrlen);
        printf("Host disconnected , ip %s , port %d \n",
               inet_ntoa(address.sin_addr), ntohs(address.sin_port));
        counter--;
        for (int i = 0; i < 5; i++)
        {
            if (clients[i].socket_id == sd)
            {
                clients[i].id = 0;
                clients[i].socket_id = 0;
                for (int k = 0; k < number_of_file_a_client_can_upload; k++)
                {
                    remove(clients[i].file[k].filename);
                    clients[i].file[k].filename[0] = '\0';
                    clients[i].file[k].linecount = 0;
                    clients[i].file[k].ownner = 0;
                    clients[i].file[k].permission[0] = '\0';

                    for (int m = 0; m < 4; m++)
                    {
                        clients[i].file[k].collaborator[m] = 0;
                    }
                }
            }
        }
        close(sd);
        client_socket[i] = 0;
        flag = 1;
    }
    else if (nv == 0)
    {
        // Somebody disconnected , get his details and print
        getpeername(sd, (struct sockaddr *)&address,
                    (socklen_t *)&addrlen);
        printf("Host disconnected , ip %s , port %d \n",
               inet_ntoa(address.sin_addr), ntohs(address.sin_port));
        counter--;
        for (int i = 0; i < 5; i++)
        {
            if (clients[i].socket_id == sd)
            {
                clients[i].id = 0;
                clients[i].socket_id = 0;
                for (int k = 0; k < number_of_file_a_client_can_upload; k++)
                {
                    remove(clients[i].file[k].filename);
                    clients[i].file[k].filename[0] = '\0';
                    clients[i].file[k].linecount = 0;
                    clients[i].file[k].ownner = 0;
                    clients[i].file[k].permission[0] = '\0';

                    for (int m = 0; m < 4; m++)
                    {
                        clients[i].file[k].collaborator[m] = 0;
                    }
                }
            }
        }
        // Close the socket and mark as 0 in list for reuse
        close(sd);
        client_socket[i] = 0;
    }

    return flag;
}

void communication_fn()
{
    char *message = "welcome msg from server \n\r\n";
    char *message2 = "busy";

    while (TRUE)
    {
        // clear the socket set
        FD_ZERO(&readfds);

        // add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        // add child sockets to set
        for (i = 0; i < max_clients; i++)
        {
            // socket descriptor
            sd = client_socket[i];

            // if valid socket descriptor then add to read list
            if (sd > 0)
                FD_SET(sd, &readfds);

            // highest file descriptor number, need it for the select function
            if (sd > max_sd)
                max_sd = sd;
        }

        // wait for an activity on one of the sockets , timeout is NULL ,
        // so wait indefinitely
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        // If something happened on the master socket ,
        // then its an incoming connection

        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket,
                                     (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            if (counter < number_of_client)
            {
                printf("New connection , socket fd is %d , ip is : %s , port : %d\n ", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                // send new connection greeting message
                if (send(new_socket, message, strlen(message), 0) != strlen(message))
                {
                    print_error("send");
                }

                puts("Welcome message sent successfully");

                // add new socket to array of sockets
                for (i = 0; i < max_clients; i++)
                {
                    // if position is empty
                    if (client_socket[i] == 0)
                    {
                        client_socket[i] = new_socket;
                        printf("Adding to list of sockets as %d\n", i);
                        counter++;
                        clients[i].id = (new_socket + 20000);
                        clients[i].socket_id = new_socket;
                        break;
                    }
                }

                printf("\nactive clients = %d\n", counter);
            }
            else
            {
                if (send(new_socket, message2, strlen(message2), 0) != strlen(message2))
                {
                    print_error("send");
                }
            }
        }

        // else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds))
            {
                // Check if it was for closing , and also read the
                // incoming message
                bzero(buffer, sizeof(buffer));
                valread = recv(sd, buffer, sizeof(buffer), 0);
                if (valread < 0)
                {
                    print_error("error in recieving \n");
                }

                printf("client says:\t  %s \n", buffer);
                int flag = user_cmd_check(buffer, valread);
                printf("\nend of loop\n ");
            }
        }
    }
}
int main(int argc, char const *argv[])
{

    // initialise null value to structure
    for (int i = 0; i < 5; i++)
    {
        clients[i].id = 0;
        clients[i].socket_id = 0;
        for (int k = 0; k < number_of_file_a_client_can_upload; k++)
        {
            clients[i].file[k].filename[0] = '\0';
            clients[i].file[k].linecount = 0;
            clients[i].file[k].ownner = 0;
            clients[i].file[k].permission[0] = '\0';
            for (int m = 0; m < 4; m++)
            {
                clients[i].file[k].collaborator[m] = 0;
            }
        }
    }

    // initialise all client_socket[] to 0 so not checked

    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }

    // create a master socket
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // set master socket to allow multiple connections ,
    // this is just a good habit, it will work without this
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                   sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    // try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");
    communication_fn();

    return 0;
}
