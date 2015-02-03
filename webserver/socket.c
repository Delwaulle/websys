#include "socket.h"
#include "signal.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


int creer_serveur(int port ){
	int socket_serveur ;

	//On ignore le signal SIGPIPE
	initialiser_signaux();

	socket_serveur = socket(AF_INET , SOCK_STREAM , 0);
	if ( socket_serveur == -1){
		perror ("socket_serveur");
		return -1;
	}
	
	//Ctrl+c sans délai
	int optval = 1;
	if(setsockopt(socket_serveur , SOL_SOCKET , SO_REUSEADDR , &optval , sizeof(int)) == -1)
		perror("Can not set SO_REUSEADDR option ");

	struct sockaddr_in saddr ;
	saddr.sin_family = AF_INET ; /* Socket ipv4 */
	saddr.sin_port = htons(port); /* Port d ’ écoute */
	saddr.sin_addr.s_addr = INADDR_ANY ; /* écoute sur toutes les interfaces */
	int bindd=bind(socket_serveur , (struct sockaddr *)&saddr , sizeof( saddr ));
	if ( bindd == -1){
		perror ("bind socket_serveur");
		return -1;
	}
	int listenn=listen(socket_serveur , 10);
	if ( listenn == -1){
		perror ("listen socket_serveur");
		return -1;
	}

	return socket_serveur;
}

char * afficherMessage(){
	char * message_bienvenue=" __          ________ ____   _______     _______ \n\\ \\        / /  ____|  _ \\ / ____\\ \\   / / ____|\n\\ \\  /\\  / /| |__  | |_) | (___  \\ \\_/ / (___  \n\\ \\/  \\/ / |  __| |  _ < \\___ \\  \\   / \\___ \\ \n\\  /\\  /  | |____| |_) |____) |  | |  ____) |\n       \\/  \\/   |______|____/|_____/   |_| |_____/ \n                                                   \n                                                    \n Welcome to websys, our dedicated webserver for your websites. \n Created by Delwaulle Loic & Froment Benoit during a student project.\n";
	return message_bienvenue;
}

char *substr(const char *src,int pos,int len) { 
	char *dest=NULL;                        
	if (len>0) {                           
   		dest = calloc(len+1, 1);       
    		if(NULL != dest) {
       			strncat(dest,src+pos,len);            
    		}
  	}                                       
	return dest;                            
}
int parse_http_request( const char * ligne , http_request * request){
	int nbMots=1;
	int i;
	char get[50];
	char str[50];
	sscanf(ligne,"%s %s", get, str);
	if(strcmp(get,"GET")==0){
		request->method=HTTP_GET;
	}
	else {
		request->method=HTTP_UNSUPPORTED;
	}
	for(i=0;i<strlen(ligne);i++){
		if(ligne[i]==' '){
			nbMots++;
			if(nbMots==3){
				char *http=substr(ligne,i+1,4);
				char *version=substr(ligne,i+6,strlen(ligne));
				if(strcmp(http,"HTTP")!=0){
					return 0;
				}
				char c=version[0];
				char c2=version[2];
				int v1=(int)c-48;
				int vv=(int)c2-48;
				request->major_version=v1;
				request->minor_version=vv;
				if(v1!=1 || (vv!=0 && vv!=1)) {
					return 0;
				}
			}
			else if (nbMots==2){
				char method[50];
				char url[50];
				char reste[50];
				sscanf(ligne,"%s %s %s", method, url,reste);
				request->url=url;
			}
		}
	}
	if(nbMots!=3)
		return 0;
	return 1;
}

char *fgets_or_exit( char * buffer , int size , FILE * stream ){
	if(fgets(buffer,size,stream)==NULL){
		exit(0);
	}
}

void send_response ( FILE * client , int code ,const char * message_cours ,const char * message_long ){
	send_status(client,code,message_cours);
	fprintf(client, "Content-Length: %i\r\n%s\r\n",strlen(message_long),message_long);
}

void send_status( FILE * client , int code , const char * reason_phrase ){
	fprintf(client,"HTTP/1.1 %i\r\n%s\r\n",code,reason_phrase);
}


void skip_headers(FILE *client){


}

#define BUFF_SIZE 256
void traiterClient(int socket_client){
	char p[BUFF_SIZE];
	int i=0;
	const char * mode="w+";
	http_request req;
	http_request *r=&req;
	FILE * f=fdopen(socket_client, mode);
	while(fgets_or_exit(p,BUFF_SIZE,f)){
		if(i==0){
			if(parse_http_request(p,r)==0){
				send_response(f, 400 , "Bad Request" , "Bad request\r\n");
				exit(0);			
			}			
			else if(req.method == HTTP_UNSUPPORTED ){
				send_response( f , 405 , "Method Not Allowed" , "Method Not Allowed\r\n" );
				exit(0);			
			}
			else if(strcmp(req.url, "/" )== 0){
				send_response( f , 200 , "OK" , afficherMessage() );
			}
			else{
				send_response(f, 404 , "Not Found" , "Not Found\r\n" );
				exit(0);			
			}
		}
		if(p[0]=='\n' || (strcmp(p,"\r\n")==0)){
			break;
		}
		
		printf("<websys> %s", p);
		i++;
	}
}

int attendre_socket(int socket_serveur){
	pid_t pid;
	int status;
	while(1){
		int socket_client ;
		socket_client = accept(socket_serveur , NULL , NULL );
		if (socket_client == -1){
			perror("accept");
			return -1;
		}
			
		pid=fork();
		if(pid==0){

			traiterClient(socket_client);
			exit(0);
		}
		else{ 
			close(socket_client);
		}		
	}
}
