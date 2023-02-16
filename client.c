#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>
#define PORT 9897
#define max 2000
#define max1 90000
int clientSocket, ret;
int fdmax;
struct sockaddr_in serverAddr;
char buffer[1024];
fd_set readfds1;
void print_error(char *msg)
{
	perror(msg);
	exit(0);
}
void print_msg(char *msg)
{
	puts(msg);
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
void getcommand(char *smsg)
{
	if (strncmp(buffer, "/users", 6) == 0)
	{

		bzero(buffer, sizeof(buffer));
		if (recv(clientSocket, buffer, sizeof(buffer), 0) < 0)
		{
			print_error("[-]Error in receiving data.\n");
		}
		printf("the number client are : %s\n", buffer);
		int ncount = atoi(buffer);
		int i = 1;
		for (i = 1; i <= ncount; i++)
		{
			bzero(buffer, sizeof(buffer));
			if (recv(clientSocket, buffer, sizeof(buffer), 0) < 0)
			{
				print_error("[-]Error in receiving data.\n");
			}
			printf("%s\n", buffer);
			fflush(stdout);
		}
	}
	else if (strncmp(buffer, "/upload", 7) == 0)
	{
		char filename[1001] = {0};
		char command[20];
		char line[1000];
		sscanf(buffer, "%s %s", command, filename);

		bzero(buffer, sizeof(buffer));
		if (recv(clientSocket, buffer, sizeof(buffer), 0) < 0)
		{
			print_error("[-]Error in receiving data.\n");
		}
		FILE *fp = fopen(filename, "r");
		if (fp == NULL)
		{
			printf("unable to open file");
			printf("[-]Disconnected from server.\n");
			close(clientSocket);
			exit(0);
		}
		else if (strncmp(buffer, "samefile", 8) == 0)
		{
			printf("%s\n", buffer);
			printf("same file exits at server try again with new filename\n");
			printf("unable to uploaded the file\n");
			fclose(fp);
		}
		else
		{
			printf("%s\n", buffer);

			sendfile(filename, clientSocket);
			printf("successfully uploaded the file\n");
		}
	}
	else if (strncmp(buffer, "/files", 6) == 0)
	{
		char temp[max];
		bzero(temp, sizeof(temp));
		if (recv(clientSocket, temp, sizeof(temp), 0) < 0)
		{
			print_error("[-]Error in receiving data.\n");
		}
		if (strlen(temp) == 0)
		{
			printf("NO RECORD FOUND \n");
		}
		else
		{
			printf("%s\n", temp);
		}
		bzero(temp, sizeof(temp));
		bzero(buffer, sizeof(buffer));
	}
	else if (strncmp(buffer, "/download ", 10) == 0)
	{
		char filename[1001] = {0};
		char command[20];
		char line[1000];
		sscanf(buffer, "%s %s", command, filename);
		bzero(buffer, sizeof(buffer));
		if (filename[0] != '\0')
		{
			if (recv(clientSocket, buffer, sizeof(buffer), 0) < 0)
			{
				print_error("[-]Error in receiving data.\n");
			}
			if (strncmp(buffer, "NOT", 3) == 0)
			{
				printf("server says : %s\n", buffer);
				printf("unable to download the file \n");
			}
			else
			{
				printf("server is sending the file....\n please wait...\n");
				strcat(filename, "_downloaded_file");

				getfile(filename, clientSocket);
				printf("successfully downloaded the file \n");
			}
		}
	}
	else if (strncmp(buffer, "/read", 5) == 0)
	{
		char temp[max1];
		bzero(temp, sizeof(temp));
		if (recv(clientSocket, temp, sizeof(temp), 0) < 0)
		{
			print_error("[-]Error in receiving data.\n");
		}
		if (strncmp(temp, "NOT", 3) == 0)
		{
			printf("%s \n", temp);
			printf("operation unsuceesful\n");
		}
		else
		{
			printf("file is succesfully readed \n");
			printf("%s\n", temp);
		}
		bzero(temp, sizeof(temp));
		bzero(buffer, sizeof(buffer));
	}
	else if (strncmp(buffer, "/delete", 7) == 0)
	{
		char temp[max1];
		bzero(temp, sizeof(temp));
		if (recv(clientSocket, temp, sizeof(temp), 0) < 0)
		{
			print_error("[-]Error in receiving data.\n");
		}
		if (strncmp(temp, "NOT", 3) == 0)
		{
			printf("%s \n", temp);
			printf("operation unsuceesful\n");
		}
		else
		{
			printf("file is succesfully readed \n");
			printf("%s\n", temp);
			bzero(temp, sizeof(temp));
			if (recv(clientSocket, temp, sizeof(temp), 0) < 0)
			{
				print_error("[-]Error in receiving data.\n");
			}
			printf("%s\n", temp);
		}
		bzero(temp, sizeof(temp));
		bzero(buffer, sizeof(buffer));
	}
	else if (strncmp(buffer, "/insert", 7) == 0)
	{
		char temp[max1];
		bzero(temp, sizeof(temp));
		if (recv(clientSocket, temp, sizeof(temp), 0) < 0)
		{
			print_error("[-]Error in receiving data.\n");
		}
		if (strncmp(temp, "NOT", 3) == 0)
		{
			printf("%s \n", temp);
			printf("operation unsuceesful\n");
		}
		else
		{
			printf("insert successful\n");
			printf("%s\n", temp);
			bzero(temp, sizeof(temp));
			if (recv(clientSocket, temp, sizeof(temp), 0) < 0)
			{
				print_error("[-]Error in receiving data.\n");
			}
			printf("%s\n", temp);
		}
		bzero(temp, sizeof(temp));
		bzero(buffer, sizeof(buffer));
	}
	else if (strncmp(buffer, "/invite", 7) == 0)
	{
		printf("waiting for response from client...\n");
		bzero(buffer, sizeof(buffer));
		if (recv(clientSocket, buffer, sizeof(buffer), 0) < 0)
		{
			print_error("[-]Error in receiving data.\n");
		}
		printf("%s\n",buffer);
	
	}
}

void communication_fn(int cl_socket)
{

	while (1)
	{
		FD_ZERO(&readfds1);
		FD_SET(clientSocket, &readfds1); /* add sockfd to connset */
		FD_SET(0, &readfds1);			 /* add STDIN to connset */
		fdmax = clientSocket;

		if (select(fdmax + 1, &readfds1, NULL, NULL, NULL) < 0)
		{
			fprintf(stdout, "select() error\n");
			exit(0);
		}

		if (FD_ISSET(clientSocket, &readfds1))
		{

			/*
			 * Server sends msglen
			 * client reads msg of length msglen
			 * client prints it to stdout
			 * client waits for next activity of its listen sockets
			 */
			char buff[1024];
			bzero(buff, sizeof(buff));
			if (recv(clientSocket, buff, 1024, 0) < 0)
			{
				printf("[-]Error in receiving data.\n");
			}
			printf("%s\n", buff);
			if (strncmp(buff, "busy", 4) == 0)
			{

				printf("the server is busy try again later\n");
				printf("[-]Disconnected from server.\n");
				close(clientSocket);
				exit(0);
			}
			if (strncmp(buff, "Invite", 6) == 0)
			{
				//	printf("%s\n", buffer);
				fprintf(stdout, "Reply in yes and no only\n");
				bzero(buff, strlen(buff));
				fgets(buff, 1024, stdin);
				if (strncmp(buff, "yes", 3) == 0 || strncmp(buff, "no", 2) == 0)
				{
					if (send(clientSocket, buff, strlen(buff) - 1, 0) < 0)
					{
						print_error("[-]Error in sending data.\n");
					}
				}
				else
				{
					if (send(clientSocket, buff, strlen(buff) - 1, 0) < 0)
					{
						print_error("[-]Error in sending data.\n");
					}
				}
				printf("feedback succesfully sended \n");
			}

			fflush(stdout);
			printf("Enter the command for client\n");
		}
		if (FD_ISSET(0, &readfds1))
		{

			bzero(buffer, sizeof(buffer));
			fgets(buffer, 1024, stdin);
			send(clientSocket, buffer, strlen(buffer) - 1, 0);
			if (strncmp(buffer, "/exit", 5) == 0)
			{

				printf("[-]Disconnected from server.\n");
				close(clientSocket);
				exit(0);
			}
			getcommand(buffer);
			printf("Enter the command for client\n");
		}
	}
}
int main(int argc, char const *argv[])
{
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket < 0)
	{
		print_error("[-]Error in connection.\n");
	}
	print_msg("[+]Client Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if (ret < 0)
	{
		print_error("[-]Error in connection.\n");
	}
	print_msg("[+]Connected to Server.\n");
	print_msg("waiting for response from server... \n");
	communication_fn(clientSocket);
	close(clientSocket);
	return 0;
}
