//Includes para stdin e stdout
#include <string.h>
#include <stdio.h>

//Includes para server UDP
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <ctype.h>

//Includes para server TCP
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

//Definição das funções necessárias para as ligações TCP, apresentação de erro, e header.
void process_client_AAS(int fd);
void process_client_AGS(int fd);
void erro(char *msg);
void header(char client[3], char type[50]);

#define BUF_SIZE 5000

int main(){
	//aps = 0 -> Servidor UDP; aps != 0 -> Servidores TCP
	int aps = fork();   

	while (1){
		if(aps == 0){
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                  									SERVIDOR UDP -> APLICAÇÃO PROFISSIONAL DE SAUDE 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
		//Inicio da definição do servidor
		int udpSocket, nBytes;
		//, nSize;

		struct sockaddr_in serverAddr;
		//clientAddr;
		struct sockaddr_storage serverStorage;

		socklen_t addr_size;
		//client_addr_size;	

		udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(7000);
		serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

		bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

		addr_size = sizeof serverStorage;
		printf("Ligação UDP APS criada.\n");
		fflush(stdout);
		//Fim da definição do servidor
		
		//Definição das variáveis necessárias ao funcionamento do servidor para o Profissional de Saúde
		char operacao[1];
		char id_login[10] = "";
		char password[100] = "";
		char registo[250] = "";
		char crime[1000] = "";
		FILE *credenciais_aps;
		FILE *fcrimes;
		FILE *reg_edit;
		FILE *reg_del;

		while(1){
			/*O uso de fflush(stdout) e fflush(stdin) é bastante usado em todo o programa, para limpar as streams de entrada
			e saida e evitar lixo nas mesmas*/
			inicio:
			//Recebe a operação que o servidor terá de executar, para fácil orientação
			nBytes = recvfrom(udpSocket, operacao, 2, 0, (struct sockaddr *)&serverStorage, &addr_size);
			fflush(stdin);
			printf("OPÇÃO RECEBIDA DEBUG APS: %s\n", operacao);
			sleep(1);
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                												OPERAÇÃO 1 - LOGIN 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
			if(operacao[0] == '1'){
				FILE *checks;
				char checkaux[250] = "";
				
				//Recebe o ID de utilizador
				nBytes = recvfrom(udpSocket, id_login, 100, 0, (struct sockaddr *)&serverStorage, &addr_size);
				fflush(stdin);
				header("APS", "LOGIN");
				printf("\n\nID: %s\n", id_login);
				fflush(stdout);
				
				//Recebe a password do utilizador
				nBytes = recvfrom(udpSocket, password, 100, 0, (struct sockaddr *)&serverStorage, &addr_size);
				fflush(stdin);
				printf("Password: %s\n", password);
				fflush(stdout);
				
				//Aqui vai abrir o ficheiro de registos por aceitar e verifica linha a linha se o ID está pendente aprovação
				checks = fopen("DATABASE Files/saude_new.txt", "r");
				while(fscanf(checks, "%s\n", checkaux) != EOF){
					fflush(stdout);
					fflush(stdin);
					//Strstr verifica se id_login está presente na string checkaux, que é onde a linha lida do ficheiro se encontra
					if(strstr(checkaux, id_login)){
						//Caso entre neste ciclo significa que ID está em uso, informa cliente e volta ao inicio
						printf("\nID pendente aprovação, a informar cliente.\n");
						char new[4] = "NEW";
						nBytes = strlen(new)+1;
						sendto(udpSocket, new, nBytes, 0, (struct sockaddr *)&serverStorage, addr_size);
						fflush(stdout);
						goto inicio;
					}
				}
				fclose(checks);
				
				//Aqui vai abrir o ficheiro de registos existentes e verifica linha a linha se o ID é o correto, para poder
				//fazer comparação de password
				checks = fopen("DATABASE Files/credenciais_saude.txt", "r");
				while(fgets(checkaux, sizeof(checkaux), checks) != NULL){
					fflush(stdout);
					fflush(stdin);
					
					checkaux[strcspn(checkaux, "\n")] = 0; /*Como o fgets lê a linha toda incluindo o "\n" após a password, este tem
																					que ser removido as comparações funcionarem corretamente*/
					
					//Strstr verifica se id_login está presente na string checkaux, que é onde a linha lida do ficheiro se encontra
					if(strstr(checkaux, id_login)){
						const char separador[2] = "\\"; //Char delimitador para o strtok
						char *pw_tok;
						char *user_tok;
						
						//Utilização do strtok para separar a linha de registos nos 3 componentes, ID, username e password
						pw_tok = strtok(checkaux, separador);
						user_tok = strtok(NULL, separador);
						pw_tok = strtok(NULL, separador);

						//Se a password for igual a pw_tok (retirado da linha de registo existente), aceita o login, informa cliente
						//e volta ao inicio
						if(strcmp(pw_tok, password) == 0){
							nBytes = strlen(user_tok)+1;
							sendto(udpSocket, user_tok, nBytes, 0, (struct sockaddr *)&serverStorage, addr_size);
							printf("\nCredenciais corretas, a permitir ligação.\n");
							fflush(stdout);
							goto inicio;
						}
					}
				}
				
				//Caso passe por todas as verificações e não entre em nenhuma, significa que está errado o login, informa o
				//cliente e volta ao inicio
				char isnt[1] = "0";
				nBytes = strlen(isnt)+1;
				sendto(udpSocket, isnt, nBytes, 0, (struct sockaddr *)&serverStorage, addr_size);
				printf("\nCredenciais incorretas, a informar o cliente.\n");
				fflush(stdout);
			}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                              											OPERAÇÂO 2 - REGISTO 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
			if(operacao[0] == '2'){
				//Assim que entra no registo fica logo à espera de receber o registo completo
				nBytes = recvfrom(udpSocket, registo, 250, 0, (struct sockaddr *)&serverStorage, &addr_size);
				fflush(stdin);
				
				const char separador[2] = "\\";
				char *id_tok;
				char aux[250] = "";
				//Copia-se o registo para uma variável auxiliar pois o strtok vai cortar a string em que é usado
				strcpy(aux, registo);
				id_tok = strtok(aux, separador); //Vai-se buscar o ID do registo recebido com strtok
				
				FILE *checks;
				char checkaux[250] = "";
				
				header("APS", "REGISTO");
				printf("\n\nNovo registo: %s", registo);
				
				//Aqui vai abrir o ficheiro dos registos existentes e verificar linha a linha se o ID já está em uso
				checks = fopen("DATABASE Files/credenciais_saude.txt", "r");
				while(fscanf(checks, "%s\n", checkaux) != EOF){
					fflush(stdin);
					//Se entrar neste if significa que já existe, informa cliente e volta ao inicio
					if(strstr(checkaux, id_tok)){
						printf("\n\nID já em uso, a informar cliente.\n");
						nBytes = strlen("USE")+1;
						sendto(udpSocket, "USE", nBytes, 0, (struct sockaddr *)&serverStorage, addr_size);
						fflush(stdout);
						goto inicio;
					}
				}
				fclose(checks);
				
				//Aqui vai abrir o ficheiro dos registos de saude novos e verificar linha a linha se o ID já está em uso por aprovar
				checks = fopen("DATABASE Files/saude_new.txt", "r");
				while(fscanf(checks, "%s\n", checkaux) != EOF){
					fflush(stdin);
					//Se entrar neste if sinifica que já existe, informa cliente e volta ao inicio
					if(strstr(checkaux, id_tok)){
						printf("\n\nID pendente aprovação, a informar cliente.\n");
						nBytes = strlen("NEW")+1;
						sendto(udpSocket, "NEW", nBytes, 0, (struct sockaddr *)&serverStorage, addr_size);
						fflush(stdout);
						goto inicio;
					}
				}
				fclose(checks);
				
				//Caso passe por todas as verificações e não saia, significa que registo é valido
				//Abre o ficheiro dos registos novos, escreve em append, informa o cliente e volta ao inicio
				credenciais_aps = fopen("DATABASE Files/saude_new.txt", "a");
				fprintf(credenciais_aps, "%s\n", registo);
				fflush(stdout);
				printf("\n\nUtilizador registado com sucesso!\n\n");
				fclose(credenciais_aps);
				
				nBytes = strlen("1")+1;
				sendto(udpSocket, "1", nBytes, 0, (struct sockaddr *)&serverStorage, addr_size);
				fflush(stdout);
				sleep(3);
			}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                										OPERAÇÃO 7 - SUBMETER CRIME 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
			if(operacao[0] == '7'){
				//Assim que entra nesta operação recebe logo um registo de crime
				nBytes = recvfrom(udpSocket, crime, 1000, 0, (struct sockaddr *)&serverStorage, &addr_size);
				fflush(stdin);
				header("APS", "CRIME");
				printf("\n\nNovo crime recebido: %s", crime);
				
				//Abre ficheiro dos crimes e dá append, e volta ao inicio
				fcrimes = fopen("DATABASE Files/crimes.txt", "a");
				fprintf(fcrimes, "%s\n", crime);
				fflush(stdout);
				printf("\n\nCrime registado com sucesso!\n");
				fclose(fcrimes);
				sleep(3);				
			}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                										OPERAÇÃO 8 - EDITAR REGISTO 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
			if(operacao[0] == '8'){
				//Tal como no registo, assim que entra nesta operação recebe logo o registo atualizado do utilizador
				nBytes = recvfrom(udpSocket, registo, 250, 0, (struct sockaddr *)&serverStorage, &addr_size);
				fflush(stdin);
				header("APS", "REG EDIT");
				printf("\n\nNova edição de registo recebida: %s", registo);
				reg_edit = fopen("DATABASE Files/credenciais_saude.txt", "r");
				
				char reg_aps[1000][250] = {};
				char aux[250] = "";
				int regaps_count = 0;
				
				//Copiar registo editado para variável auxiliar para usar com strtok
				strcpy(aux, registo);
				const char separador[2] = "\\";
				char *id_tok;
				
				//Obtenção do ID do registo recebido
				id_tok = strtok(aux, separador);
				
				//Ciclo para ir buscar para uma matriz bi-dimensional todos os registos existentes
				while(fgets(reg_aps[regaps_count], sizeof(reg_aps[regaps_count]), reg_edit) != NULL){
					fflush(stdin);
					
					reg_aps[regaps_count][strcspn(reg_aps[regaps_count], "\n")] = 0; /*Como o fgets lê a linha toda incluindo o "\n" após a password, este tem
																																que ser removido as comparações funcionarem corretamente*/
					
					regaps_count++; //Variável para contar a quantidade de registos existentes
				}
				fclose(reg_edit);
				
				//Este ciclo vai percorrer a lista de registos existentes
				for(int i = 0; i < regaps_count; i++){
					//Quando encontrar o ID recebido na lista de registos existentes, copia registo atualizado para o lugar do atual
					if(strncmp(reg_aps[i], id_tok, 8) == 0){
						strcpy(reg_aps[i], registo);
						break;
					}
				}
				
				//Abre ficheiro dos registos existentes e escreve por completo a matriz que contém os registos existentes
				reg_edit = fopen("DATABASE Files/credenciais_saude.txt", "w");
				for(int i = 0; i < regaps_count; i++){
					fprintf(reg_edit, "%s\n", reg_aps[i]);
					fflush(stdout);
				}
				printf("\n\nEdição registada com sucesso!\n");
				fclose(reg_edit);
				sleep(2);
			}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                            										    OPERAÇÃO 9 - APAGAR REGISTO 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
			if(operacao[0] == '9'){
				//Tal como na operação anterior, assim que entra aqui recebe logo o registo a apagar
				nBytes = recvfrom(udpSocket, registo, 250, 0, (struct sockaddr *)&serverStorage, &addr_size);
				fflush(stdin);
				header("APS", "REG DEL");
				printf("\n\nNovo apagamento de registo recebido: %s\n", registo);
				reg_del = fopen("DATABASE Files/credenciais_saude.txt", "r");
				
				char reg_aps[1000][250] = {};
				char aux[250] = "";
				int regaps_count = 0;
				
				//Copiar registo editado para variável auxiliar para usar com strtok
				strcpy(aux, registo);
				const char separador[2] = "\\";
				char *id_tok;
				
				//Obtenção do ID do registo recebido
				id_tok = strtok(aux, separador);
				
				//Ciclo para ir buscar para uma matriz bi-dimensional todos os registos existentes
				while(fgets(reg_aps[regaps_count], sizeof(reg_aps[regaps_count]), reg_del) != NULL){
					fflush(stdin);
					
					reg_aps[regaps_count][strcspn(reg_aps[regaps_count], "\n")] = 0; /*Como o fgets lê a linha toda incluindo o "\n" após a password, este tem
																																que ser removido as comparações funcionarem corretamente*/
					
					regaps_count++; //Variável para contar a quantidade de registos existentes
				}
				fclose(reg_del);
				
				//Este ciclo vai percorrer a lista de registos existentes
				for(int i = 0; i < regaps_count; i++){
					//Quando encontrar o ID recebido na lista de registos existentes, vai escrever "DEL" no lugar desse registo
					if(strncmp(reg_aps[i], id_tok, 8) == 0){
						strcpy(reg_aps[i], "DEL");
						break;
					}
				}
				
				//Abre ficheiro dos registos existentes e escreve por completo a matriz que contém os registos existentes
				reg_del = fopen("DATABASE Files/credenciais_saude.txt", "w");
				for(int i = 0; i < regaps_count; i++){
					//No caso da linha a escrever ser "DEL" (registo apagado), passa por cima e não escreve
					if(strcmp(reg_aps[i], "DEL") == 0){
						continue;
					}
					fprintf(reg_del, "%s\n", reg_aps[i]);
					fflush(stdout);
				}
				printf("\n\nApagamento registado com sucesso!\n");
				fclose(reg_del);
				sleep(2);
				}
			}
		}
		else{
			int aas=fork(); //aas = 0 -> Servidor TCP Agente de Segurança, aas != 0 -> Servidor TCP Gestor de Sistema
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                  									SERVIDOR TCP -> APLICAÇÃO AGENTE DE SEGURANÇA 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
			if(aas == 0){
				//Inicio da definição do servidor
				#define SERVER_Port  8000

				int fd, client;
				struct sockaddr_in addr, client_addr;
				int client_addr_size;

				//bzero((void *) &addr, sizeof(addr));
				addr.sin_family      = AF_INET;
				addr.sin_addr.s_addr = htonl(INADDR_ANY);
				addr.sin_port        = htons(SERVER_Port);

				if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
				erro("na funcao socket (AAS, porta 8000)");
				
				//Este ciclo while vai forçar o socket a esperar que a porta esvazie e reutiliza-la no caso de bind error, através do setsockopt com SO_REUSEADDR
				while(bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0){
					erro("na funcao bind (AAS, porta 8000)");			
						
					int yes = 1;
					setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
				}
				
				if( listen(fd, 5) < 0) 
				erro("na funcao listen (AAS, porta 8000)");

				int nclientes=0;
				
				printf("Ligação TCP APS criada.\n");
				fflush(stdout);

				while (1){
					client_addr_size = sizeof(client_addr);
					client = accept(fd,(struct sockaddr *)&client_addr, &client_addr_size);
					nclientes++;
					if (client > 0){
						if (fork() == 0){
							close(fd);
							//Quando estabelece uma conexão entra na função do Agente de Segurança
							process_client_AAS(client);
							exit(0);
						}
						close(client);
					}
				}
				return 0;  
			}//Fim da definição do servidor
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                  										SERVIDOR TCP -> APLICAÇÃO GESTOR DE SISTEMA 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
			else{
				//Inicio da definição do servidor	
				#define SERVER  9000

				int fd, client;
				struct sockaddr_in addr, client_addr;
				int client_addr_size;

				//bzero((void *) &addr, sizeof(addr));
				addr.sin_family      = AF_INET;
				addr.sin_addr.s_addr = htonl(INADDR_ANY);
				addr.sin_port        = htons(SERVER);

				if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
				erro("na funcao socket (AGS, porta 9000)");
				
				//Este ciclo while vai forçar o socket a esperar que a porta esvazie e reutiliza-la no caso de bind error, através do setsockopt com SO_REUSEADDR
				while(bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0){
					erro("na funcao bind (AGS, porta 9000)");
					
					int yes = 1;
					setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
				}
				
				if( listen(fd, 5) < 0) 
				erro("na funcao listen (AGS, porta 9000)");

				int nclientes=0;
				
				printf("Ligação TCP AGS criada.\n");
				fflush(stdout);

				while (1){
					client_addr_size = sizeof(client_addr);
					client = accept(fd,(struct sockaddr *)&client_addr, &client_addr_size);
					nclientes++;
					if (client > 0){
						if (fork() == 0){
							close(fd);
							//Quando estabelece uma conexão entra na função do Gestor de Sistema
							process_client_AGS(client);
							exit(0);
						}
						close(client);
					}
				}
				return 0;  
			}//Fim da definição do servidor
		}        
	}	
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                      											APLICAÇÃO AGENTE DE SEGURANÇA 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
 void process_client_AAS(int client_fd){
	 char operacao[1];
	 char id_login[10] = "";
	 char password[100] = "";
	 char registo[250] = "";
	 FILE *credenciais_aas;
	 FILE *fcrimes;
	 FILE *reg_edit;
	 FILE *reg_del;
	 
	 int nread = 0;
	 
	 while(1){
		 inicio_aas:
		 //Recebe a operação que o servidor terá de executar, para fácil orientação
		 operacao[0] = '0';
		 nread = read(client_fd, operacao, BUF_SIZE-1);
		 operacao[nread] = '\0';
		 fflush(stdin);
		 printf("OPÇÃO RECEBIDA DEBUG AAS: %s\n", operacao);
		 sleep(1);
		 
		 /*No caso de uma interrupção abrupta da conexão TCP, o servidor começa a receber lixo pela porta.
		 Como as operações são predefinidas e estão numeradas, qualquer dado recebido aqui que seja diferente é lixo
		 e significa que o cliente se desconectou de repente, sendo que a função vai para a saida em que termina
		 corretamente a ligação e sai.*/
		 if(strcmp(operacao, "") == 0){
			 operacao[0] = '3';
		 }
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                            												    OPERAÇÃO 1 - LOGIN 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
		 if(operacao[0] == '1'){
			 FILE *checks;
			 char checkaux[250] = "";
			 
			 //Recebe o ID de utilizador
			 nread = read(client_fd, id_login, BUF_SIZE-1);
			 id_login[nread] = '\0';
			 fflush(stdin);
			 header("AAS", "LOGIN");
			 printf("\n\nID: %s\n", id_login);
			 fflush(stdout);
			 
			 //Recebe a password do utilizador
			 nread = read(client_fd, password, BUF_SIZE-1);
			 password[nread] = '\0';
			 fflush(stdin);
			 printf("Password: %s\n", password);
			 fflush(stdout);
			 
			 //Aqui vai abrir o ficheiro de registos por aceitar e verifica linha a linha se o ID está pendente aprovação
			 checks = fopen("DATABASE Files/psp_new.txt", "r");
			 while(fscanf(checks, "%s\n", checkaux) != EOF){
				fflush(stdout);
				fflush(stdin);
				//Strstr verifica se id_login está presente na string checkaux, que é onde a linha lida do ficheiro se encontra
				if(strstr(checkaux, id_login)){
					//Caso entre neste ciclo significa que ID está em uso, informa cliente e volta ao inicio
					printf("\n\nID pendente aprovação, a informar cliente.\n");
					write(client_fd, "NEW", strlen("NEW"));
					fflush(stdout);
					goto inicio_aas;
				}
			}
			fclose(checks);
			
			/*Aqui vai abrir o ficheiro de registos existentes e verifica linha a linha se o ID é o correto, para poder
			fazer comparação de password*/
			checks = fopen("DATABASE Files/credenciais_psp.txt", "r");
			while(fgets(checkaux, sizeof(checkaux), checks) != NULL){
				fflush(stdout);
				fflush(stdin);
				
				checkaux[strcspn(checkaux, "\n")] = 0; /*Como o fgets lê a linha toda incluindo o "\n" após a password, este tem
																				que ser removido as comparações funcionarem corretamente*/
				
				//Strstr verifica se id_login está presente na string checkaux, que é onde a linha lida do ficheiro se encontra
				if(strstr(checkaux, id_login)){ //SE ENCONTRAR ID
					const char separador[2] = "\\"; //Char delimitador para o strtok
					char *pw_tok;
					char *user_tok;
					
					//Utilização do strtok para separar a linha de registos nos 3 componentes, ID, username e password
					pw_tok = strtok(checkaux, separador);
					user_tok = strtok(NULL, separador);
					pw_tok = strtok(NULL, separador);
					
					/*Se a password for igual a pw_tok (retirado da linha de registo existente), aceita o login, informa cliente
					e volta ao inicio*/
					if(strcmp(pw_tok, password) == 0){
						write(client_fd, user_tok, strlen(user_tok));
						printf("\nCredenciais corretas, a permitir ligação.\n");
						fflush(stdout);
						goto inicio_aas;
					}
				}
			}
			
			/*Caso passe por todas as verificações e não entre em nenhuma, significa que está errado o login, informa o
			cliente e volta ao inicio*/
			write(client_fd, "0", strlen("0"));
			printf("\nCredenciais incorretas, a informar o cliente.\n");
			fflush(stdout);
		 }
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                             											   OPERAÇÃO 2 - REGISTO 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
		 if(operacao[0] == '2'){
			 //Assim que entra no registo fica logo à espera de receber o registo completo
			 nread = read(client_fd, registo, BUF_SIZE-1);
			 registo[nread] = '\0';
			 fflush(stdin);
			 
			 const char separador[2] = "\\";
			 char *id_tok;
			 char aux[250] = "";
			 //Copia-se o registo para uma variável auxiliar pois o strtok vai cortar a string em que é usado
			 strcpy(aux, registo);
			 id_tok = strtok(aux, separador);
			 
			 FILE *checks;
			 char checkaux[250] = "";
			 
			 header("AAS", "REGISTO");
			 printf("\n\nNovo registo: %s", registo);
			 
			 //Aqui vai abrir o ficheiro dos registos existentes e verificar linha a linha se o ID já está em uso
			 checks = fopen("DATABASE Files/credenciais_psp.txt", "r");
			 while(fscanf(checks, "%s\n", checkaux) != EOF){
				 fflush(stdin);
				 //Se entrar neste if significa que já existe, informa cliente e volta ao inicio
				 if(strstr(checkaux, id_tok)){
					 printf("\n\nID já em uso, a informar cliente.\n");
					 write(client_fd, "USE", strlen("USE"));
					 fflush(stdout);
					 goto inicio_aas;
				 }
			 }
			 fclose(checks);
			 
			 //Aqui vai abrir o ficheiro dos registos de saude novos e verificar linha a linha se o ID já está em uso por aprovar
			 checks = fopen("DATABASE Files/psp_new.txt", "r");
			 while(fscanf(checks, "%s\n", checkaux) != EOF){
				 fflush(stdin);
				 //Se entrar neste if sinifica que já existe, informa cliente e volta ao inicio
				 if(strstr(checkaux, id_tok)){
					 printf("\n\nID pendente aprovação, a informar cliente.\n");
					 write(client_fd, "NEW", strlen("NEW"));
					 fflush(stdout);
					 goto inicio_aas;
				 }
			 }
			 fclose(checks);
			 
			 /*Caso passe por todas as verificações e não saia, significa que registo é valido
			 Abre o ficheiro dos registos novos, escreve em append, informa o cliente e volta ao inicio*/
			 credenciais_aas = fopen("DATABASE Files/psp_new.txt", "a");
			 fprintf(credenciais_aas, "%s\n", registo);
			 fflush(stdout);
			 printf("\n\nUtilizador registado com sucesso!\n");
			 fclose(credenciais_aas);
			 
			 write(client_fd, "1", sizeof("1"));
			 fflush(stdout);
			 sleep(3);
		 }
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                             												   OPERAÇÃO 3 - SAIR
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
		 if(operacao[0] == '3'){
			 header("AAS", "EXIT");
			 printf("\n\nUtilizador desconectado\n");
			 
			 //Limpa streams de entrada e saida, fecha a ligação, e sai da função
			 fflush(stdin);
			 fflush(stdout);
			 close(client_fd);
			 exit(1);
		}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                             											   OPERAÇÃO 7 - VER CRIMES
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
		 if(operacao[0] == '7'){
			 header("AAS", "CRIMES");
			 fcrimes = fopen("DATABASE Files/crimes.txt", "r");
			 fflush(stdin);
			 
			 char crimes[1000][300] = {};
			 
			 printf("\n\nLista de crimes:\n");
			 int crime_count = 0;
			 
			 /*Neste ciclo vai ler o ficheiro de crimes, previamente aberto, vai buscar linha a linha para uma matriz bi-dim
			 e escreve ao cliente a linha que leu*/
			 while(fgets(crimes[crime_count], sizeof(crimes[crime_count]), fcrimes) != NULL){
				 fflush(stdin);
				 
				 crimes[crime_count][strcspn(crimes[crime_count], "\n")] = 0; /*Como o fgets lê a linha toda incluindo o "\n" após a password, este tem
																														que ser removido as comparações funcionarem corretamente*/
				 
				 printf("%d - %s\n", crime_count, crimes[crime_count]);
				 write(client_fd, crimes[crime_count], strlen(crimes[crime_count]));
				 fflush(stdout);
				 sleep(0.5);
				 crime_count++; //Variável para contar número de crimes
			 }
			 //Quando acaba de enviar os crimes todos, escreve "Done" para o cliente sair do ciclo respetivo
			 write(client_fd, "Done", strlen("Done"));
			 fflush(stdout);	 
			 printf("\nCrimes enviados com sucesso!\n");
			 fclose(fcrimes);
			 
			 //O Agente de Segurança tem a possibilidade de marcar um crime como assistido, aqui vai receber essa confirmação
			 char assist[10] = "", crime_del[250] = "";
			 nread = read(client_fd, assist, BUF_SIZE-1);
			 assist[nread] = '\0';
			 
			 //Caso receba sinal de assistência, recebe de seguida o crime assistido
			 if(strcmp(assist, "ASSIST") == 0){
				 nread = read(client_fd, crime_del, BUF_SIZE-1);
				 crime_del[nread] = '\0';
				 
				 fcrimes = fopen("DATABASE Files/crimes.txt", "w");
				 fflush(stdin);
				 
				 /*Neste ciclo vai percorrer a lista de crimes, e quando encontrar o crime correspondente, substitui por "DEL"
				 e salta por cima da escrita no ficheiro. Nos outros crimes, escreve normalmente no ficheiro*/
				 for(int i = 0; i < crime_count; i++){
					 if(strcmp(crimes[i], crime_del) == 0){
						 strcpy(crimes[i], "DEL");
						 continue;
 			 		 }
 			 		 fprintf(fcrimes, "%s\n", crimes[i]);
 			 		 fflush(stdout);
				 }
				 fclose(fcrimes);
				 printf("\nCrime assistido com sucesso!\n");
				 fflush(stdout);				 
			 }
			 
			 //No caso do Agente de Segurança sair do menu de crimes, o servidor recebe "NOASSIST" e volta ao inicio
			 else if(strcmp(assist, "NOASSIST") == 0){
				 printf("Sem assistência de crimes.\n");
				 sleep(1);
			 }
		 }
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                          										      OPERAÇÃO 8 - EDITAR REGISTO
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
		 if(operacao[0] == '8'){
			 //Tal como no registo, assim que entra nesta operação recebe logo o registo atualizado do utilizador
			 nread = read(client_fd, registo, BUF_SIZE-2);
			 registo[nread] = '\0';
			 fflush(stdin);
			 header("AAS", "REG EDIT");
			 printf("\n\nNova edição de registo recebida: %s", registo);
			 reg_edit = fopen("DATABASE Files/credenciais_psp.txt", "r");
			 
			 char reg_aas[1000][250] = {};
			 char aux[250] = "";
			 int regaas_count = 0;
			 
			 //Copiar registo editado para variável auxiliar para usar com strtok
			 strcpy(aux, registo);
			 const char separador[2] = "\\";
			 char *id_tok;
			 
			 //Obtenção do ID do registo recebido
			 id_tok = strtok(aux, separador);
			 
			 //Ciclo para ir buscar para uma matriz bi-dimensional todos os registos existentes
			 while(fgets(reg_aas[regaas_count], sizeof(reg_aas[regaas_count]), reg_edit) != NULL){
				 fflush(stdin);
				 
				 reg_aas[regaas_count][strcspn(reg_aas[regaas_count], "\n")] = 0; /*Como o fgets lê a linha toda incluindo o "\n" após a password, este tem
																															que ser removido as comparações funcionarem corretamente*/
				 
				 regaas_count++; //Variável para contar a quantidade de registos existentes
			 }
			 fclose(reg_edit);
			 
			 //Este ciclo vai percorrer a lista de registos existentes
			 for(int i = 0; i < regaas_count; i++){
				 //Quando encontrar o ID recebido na lista de registos existentes, copia registo atualizado para o lugar do atual
				 if(strncmp(reg_aas[i], id_tok, 8) == 0){
					 strcpy(reg_aas[i], registo);
					 break;
				 }
			 }
			 
			 //Abre ficheiro dos registos existentes e escreve por completo a matriz que contém os registos existentes
			 reg_edit = fopen("DATABASE Files/credenciais_psp.txt", "w");
			 for(int i = 0; i < regaas_count; i++){
				 fprintf(reg_edit, "%s\n", reg_aas[i]);
				 fflush(stdout);
			 }
			 printf("\n\nEdição registada com sucesso!\n");
			 fclose(reg_edit);
			 sleep(2);
		 }
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                               										 OPERAÇÃO 9 - APAGAR REGISTO 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------

		 if(operacao[0] == '9'){
			 //Tal como na operação anterior, assim que entra aqui recebe logo o registo a apagar
			 nread = read(client_fd, registo, BUF_SIZE-1);
			 registo[nread] = '\0';
			 fflush(stdin);
			 header("AAS", "REG DEL");
			 printf("\n\nNovo apagamento de registo recebido: %s", registo);
			 reg_del = fopen("DATABASE Files/credenciais_psp.txt", "r");
			 
			 char reg_aas[1000][250] = {};
			 char aux[250] = "";
			 int regaas_count = 0;
			 
			 //Copiar registo editado para variável auxiliar para usar com strtok
			 strcpy(aux, registo);
			 const char separador[2] = "\\";
			 char *id_tok;
			 
			 //Obtenção do ID do registo recebido
			 id_tok = strtok(aux, separador);
			 
			 //Ciclo para ir buscar para uma matriz bi-dimensional todos os registos existentes
			 while(fgets(reg_aas[regaas_count], sizeof(reg_aas[regaas_count]), reg_del) != NULL){
				 fflush(stdin);
				 
				 reg_aas[regaas_count][strcspn(reg_aas[regaas_count], "\n")] = 0; /*Como o fgets lê a linha toda incluindo o "\n" após a password, este tem
																															que ser removido as comparações funcionarem corretamente*/
				 
				 regaas_count++; //Variável para contar a quantidade de registos existentes
			 }
			 fclose(reg_del);
			 
			 //Este ciclo vai percorrer a lista de registos existentes
			 for(int i = 0; i < regaas_count; i++){
				 //Quando encontrar o ID recebido na lista de registos existentes, vai escrever "DEL" no lugar desse registo
				 if(strncmp(reg_aas[i], id_tok, 8) == 0){
					 strcpy(reg_aas[i], "DEL");
					 break;
				 }
			 }
			 
			 //Abre ficheiro dos registos existentes e escreve por completo a matriz que contém os registos existentes
			 reg_del = fopen("DATABASE Files/credenciais_psp.txt", "w");
			 for(int i = 0; i < regaas_count; i++){
				 //No caso da linha a escrever ser "DEL" (registo apagado), passa por cima e não escreve
				 if(strcmp(reg_aas[i], "DEL") == 0){
					 continue;
				 }
				 fprintf(reg_del, "%s\n", reg_aas[i]);
				 fflush(stdout);
			 }
			 printf("\n\nApagamento registado com sucesso!\n");
			 fclose(reg_del);
			 sleep(2);
		 }
	 }
 }
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                    													  APLICAÇÃO GESTOR DE SISTEMA 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
void process_client_AGS(int client_fd){
	 char operacao[1];
	 char id_login[10] = "";
	 char password[100] = "";
	 char registo[250] = "";
	
	 char id_accept[10] = "";
	 FILE *credenciais_ags;
	 FILE *newreg_aas;
	 FILE *newreg_aps;
	 
	 char regexist_aas[1000][250] = {};
	 char regexist_aps[1000][250] = {};
	 
	 int nread = 0;
	 
	 while(1){
		 inicio_ags:
		 //Recebe a operação que o servidor terá de executar, para fácil orientação
		 operacao[0] = '0';
		 nread = read(client_fd, operacao, BUF_SIZE-1);
		 operacao[nread] = '\0';
		 fflush(stdin);
		 printf("OPÇÃO RECEBIDA DEBUG AGS: %s\n", operacao);
		 sleep(1);
		 
		 /*No caso de uma interrupção abrupta da conexão TCP, o servidor começa a receber lixo pela porta.
		 Como as operações são predefinidas e estão numeradas, qualquer dado recebido aqui que seja diferente é lixo
		 e significa que o cliente se desconectou de repente, sendo que a função vai para a saida em que termina
		 corretamente a ligação e sai.*/
		 if(strcmp(operacao, "") == 0){
			 operacao[0] = '3';
		 }
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                              												  OPERAÇÃO 1 - LOGIN 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
		 if(operacao[0] == '1'){
			 FILE *checks;
			 char checkaux[250] = "";
			 
			 //Recebe o ID de utilizador
			 nread = read(client_fd, id_login, BUF_SIZE-1);
			 id_login[nread] = '\0';
			 fflush(stdin);
			 header("AGS", "LOGIN");
			 printf("\n\nID: %s\n", id_login);
			 fflush(stdout);
			 
			 //Recebe a password do utilizador
			 nread = read(client_fd, password, BUF_SIZE-1);
			 password[nread] = '\0';
			 fflush(stdin);
			 printf("Password: %s\n", password);
			 fflush(stdout);
			 
			 //Aqui vai abrir o ficheiro de registos por aceitar e verifica linha a linha se o ID está pendente aprovação
			 checks = fopen("DATABASE Files/gestor_new.txt", "r");
			 while(fscanf(checks, "%s\n", checkaux) != EOF){
				fflush(stdout);
				fflush(stdin);
				//Strstr verifica se id_login está presente na string checkaux, que é onde a linha lida do ficheiro se encontra
				if(strstr(checkaux, id_login)){
					//Caso entre neste ciclo significa que ID está em uso, informa cliente e volta ao inicio
					printf("\n\nID pendente aprovação, a informar cliente.\n");
					write(client_fd, "NEW", strlen("NEW"));
					fflush(stdout);
					goto inicio_ags;
				}
			}
			fclose(checks);
			
			/*Aqui vai abrir o ficheiro de registos existentes e verifica linha a linha se o ID é o correto, para poder
			fazer comparação de password*/
			checks = fopen("DATABASE Files/credenciais_gestor.txt", "r");
			while(fgets(checkaux, sizeof(checkaux), checks) != NULL){
				fflush(stdout);
				fflush(stdin);
				
				checkaux[strcspn(checkaux, "\n")] = 0; /*Como o fgets lê a linha toda incluindo o "\n" após a password, este tem
																				que ser removido as comparações funcionarem corretamente*/
				
				//Strstr verifica se id_login está presente na string checkaux, que é onde a linha lida do ficheiro se encontra
				if(strstr(checkaux, id_login)){
					const char separador[2] = "\\"; //Char delimitador para o strtok
					char *pw_tok;
					char *user_tok;
					
					//Utilização do strtok para separar a linha de registos nos 3 componentes, ID, username e password
					pw_tok = strtok(checkaux, separador); //1º Tok: ID
					user_tok = strtok(NULL, separador); //2º Tok: User
					pw_tok = strtok(NULL, separador); //3º Tok: Pass
					
					/*Se a password for igual a pw_tok (retirado da linha de registo existente), aceita o login, informa cliente
					e volta ao inicio*/
					if(strcmp(pw_tok, password) == 0){
						write(client_fd, user_tok, strlen(user_tok));
						printf("\nCredenciais corretas, a permitir ligação.\n");
						fflush(stdout);
						goto inicio_ags;
					}
				}
			}
			
			/*Caso passe por todas as verificações e não entre em nenhuma, significa que está errado o login, informa o
			cliente e volta ao inicio*/
			write(client_fd, "0", strlen("0"));
			printf("\nCredenciais incorretas, a informar o cliente.\n");
			fflush(stdout);
		 }
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                           											     OPERAÇÃO 2 - REGISTO 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
		 if(operacao[0] == '2'){
			 //Assim que entra no registo fica logo à espera de receber o registo completo
			 nread = read(client_fd, registo, BUF_SIZE-1);
			 registo[nread] = '\0';
			 fflush(stdin);
			 
			 const char separador[2] = "\\";
			 char *id_tok;
			 char aux[250] = "";
			 //Copia-se o registo para uma variável auxiliar pois o strtok vai cortar a string em que é usado
			 strcpy(aux, registo);
			 id_tok = strtok(aux, separador);
			 
			 FILE *checks;
			 char checkaux[250] = "";
			 
			 header("AGS", "REGISTO");
			 printf("\n\nNovo registo: %s", registo);
			 
			 //Aqui vai abrir o ficheiro dos registos existentes e verificar linha a linha se o ID já está em uso
			 checks = fopen("DATABASE Files/credenciais_gestor.txt", "r");
			 while(fscanf(checks, "%s\n", checkaux) != EOF){
				 fflush(stdin);
				 //Se entrar neste if significa que já existe, informa cliente e volta ao inicio
				 if(strstr(checkaux, id_tok)){
					 printf("\n\nID já em uso, a informar cliente.\n");
					 write(client_fd, "USE", strlen("USE"));
					 fflush(stdout);
					 goto inicio_ags;
				 }
			 }
			 fclose(checks);
			 
			 //Aqui vai abrir o ficheiro dos registos de saude novos e verificar linha a linha se o ID já está em uso por aprovar
			 checks = fopen("DATABASE Files/gestor_new.txt", "r");
			 while(fscanf(checks, "%s\n", checkaux) != EOF){
				 fflush(stdin);
				 //Se entrar neste if sinifica que já existe, informa cliente e volta ao inicio
				 if(strstr(checkaux, id_tok)){
					 printf("\n\nID pendente aprovação, a informar cliente.\n");
					 write(client_fd, "NEW", strlen("NEW"));
					 fflush(stdout);
					 goto inicio_ags;
				 }
			 }
			 fclose(checks);
			 
			 /*Caso passe por todas as verificações e não saia, significa que registo é valido
			 Abre o ficheiro dos registos novos, escreve em append, informa o cliente e volta ao inicio*/
			 credenciais_ags = fopen("DATABASE Files/gestor_new.txt", "a");
			 fprintf(credenciais_ags, "%s\n", registo);
			 fflush(stdout);
			 printf("\n\nUtilizador registado com sucesso!\n");
			 fclose(credenciais_ags);
			 
			 write(client_fd, "1", sizeof("1"));
			 fflush(stdout);
			 sleep(3);
		 }
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                       											         	OPERAÇÃO 3 - SAIR
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
		 if(operacao[0] == '3'){
			 header("AGS", "EXIT");
			 printf("\n\nUtilizador desconectado\n");
			 
			 //Limpa streams de entrada e saida, fecha a ligação, e sai da função
			 fflush(stdin);
			 fflush(stdout);
			 close(client_fd);
			 exit(1);
		}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                     											  OPERAÇÃO 7 - AUTORIZAR NOVOS REGISTOS 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
		 if(operacao[0] == '7'){
			 header("AGS", "REG NEW AUTH");
			 newreg_aps = fopen("DATABASE Files/saude_new.txt", "r");
			 newreg_aas = fopen("DATABASE Files/psp_new.txt", "r");
			 
			 char new_aps[250][250] = {};
			 char new_aas[250][250] = {};
			 
			 int count = 0;
			 //Este ciclo vai ler para uma matriz bi-dim todos os registos por aprovar de Profissionais de Saúde, e envia para o cliente
			 while(fgets(new_aps[count], sizeof(new_aps[count]), newreg_aps) != NULL){
				 fflush(stdin);
				 
				 new_aps[count][strcspn(new_aps[count], "\n")] = 0; /*Como o fgets lê a linha toda incluindo o "\n" após a password, este tem
																										que ser removido as comparações funcionarem corretamente*/
				 
				 write(client_fd, new_aps[count], sizeof(new_aps[count]));
				 fflush(stdout);
				 sleep(0.5);
				 count++;
			 }
			 //Quando acaba de enviar os registos, envia "Done" ao cliente para ele sair do ciclo respetivo
			 write(client_fd, "Done", sizeof("Done"));
			 fflush(stdout);
			 printf("\n\nNovos registos de saúde enviados!\n");
			 fclose(newreg_aps);
			 sleep(1);
			 
			 //Este ciclo vai ler para uma matriz bi-dim todos os registos por aprovar de Agentes de Segurança, e envia para o cliente
			 count = 0;
			 while(fgets(new_aas[count], sizeof(new_aas[count]), newreg_aas) != NULL){
				 fflush(stdin);
				 
				 new_aas[count][strcspn(new_aas[count], "\n")] = 0; /*Como o fgets lê a linha toda incluindo o "\n" após a password, este tem
																										que ser removido as comparações funcionarem corretamente*/
				 
				 write(client_fd, new_aas[count], sizeof(new_aas[count]));
				 sleep(0.5);
				 fflush(stdout);
				 count++;
			 }
			 //Quando acaba de enviar os registos, envia "Done" ao cliente para ele sair do ciclo respetivo
			 write(client_fd, "Done", sizeof("Done"));
			 fflush(stdout);
			 printf("\nNovos registos de segurança enviados!\n");
			 fclose(newreg_aas);
			 sleep(1);
			 
			 newreg_aps = fopen("DATABASE Files/credenciais_saude.txt", "a");
			 newreg_aas = fopen("DATABASE Files/credenciais_psp.txt", "a");
			 
			 //Dentro deste while, o servidor começa por receber ID's aceites pelo Gestor de Sistema
			 while(1){
				 nread = read(client_fd, id_accept, BUF_SIZE-1);
				 id_accept[nread] = '\0';
				 fflush(stdin);
				 
				 //Se o ID recebido for este, sai do ciclo pois o Gestor saiu do menu respetivo também, e volta para o inicio
				 if(id_accept[0] == 's' || id_accept[0] == 'S'){
					 break;
				 }
				 
				 //Este ciclo vai percorrer ambas as listas de registos por aprovar, para verificar se o ID corresponde
				 for(int i = 0; i < 250; i++){
					 /*Caso o ID corresponda a um Profissional de Saúde por aprovar, escreve no ficheiro de credenciais
					 e substitui na lista por "DEL"*/
					 if(strstr(new_aps[i], id_accept)){
						 fprintf(newreg_aps, "%s\n", new_aps[i]);
						 fflush(stdout);
						 printf("\nRegisto APS aceite: %s\n", new_aps[i]);
						 strcpy(new_aps[i], "DEL");
						 break;
					 }
					 
					 /*Caso o ID corresponda a um Agente de Segurança por aprovar, para verificar se o ID corresponde
					  e substitui na lista por "DEL"*/
					 else if(strstr(new_aas[i], id_accept)){
						 fprintf(newreg_aas, "%s\n", new_aas[i]);
						 fflush(stdout);
						 printf("\nRegisto AAS aceite: %s\n", new_aas[i]);
						 strcpy(new_aas[i], "DEL");
						 break;
					 }
				 }
			 }
			 fclose(newreg_aps);
			 fclose(newreg_aas);
			 
			 newreg_aps = fopen("DATABASE Files/saude_new.txt", "w");
			 newreg_aas = fopen("DATABASE Files/psp_new.txt", "w");
			 
			 //Este ciclo vai percorrer a lista de registos de Saúde por aprovar, e escreve no ficheiro
			 for(int i = 0; i < 250; i++){
				 /*Como percorrer a lista de 0 a 250, em vez da contagem de registos por aprovar, quando encontrar a primeira
				 string vazia, sai deste ciclo pois não há mais dados a seguir*/
				 if(strcmp(new_aps[i], "\0") == 0){
					 break;
				 }
				 //Se a string for "DEL", salta à frente e não escreve pois esta já foi aprovada
				 if(strcmp(new_aps[i], "DEL") == 0){
					 continue;
				 }
				 fprintf(newreg_aps, "%s\n", new_aps[i]);
				 fflush(stdout);
			 }
			 fclose(newreg_aps);
			 
			 //Este ciclo vai percorrer a lista de registos de Segurança por aprovar, e escreve no ficheiro
			 for(int i = 0; i < 250; i++){
				 /*Como percorrer a lista de 0 a 250, em vez da contagem de registos por aprovar, quando encontrar a primeira
				 string vazia, sai deste ciclo pois não há mais dados a seguir*/
				 if(strcmp(new_aas[i], "\0") == 0){
					 break;
				 }
				 //Se a string for "DEL", salta à frente e não escreve pois esta já foi aprovada
				 if(strcmp(new_aas[i], "DEL") == 0){
					 continue;
				 }
				 fprintf(newreg_aas, "%s\n", new_aas[i]);
				 fflush(stdout);
			 }
			 fclose(newreg_aas);
			 
			 printf("\nNovos registos aceites com sucesso!\n");
			 sleep(3);
		 }
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//                  													     OPERAÇÃO 8 - EDITAR REGISTOS 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
		 if(operacao[0] == '8'){
			 header("AGS", "REG EDIT AUTH");
			 newreg_aps = fopen("DATABASE Files/credenciais_saude.txt", "r");
			 newreg_aas = fopen("DATABASE Files/credenciais_psp.txt", "r");
			 
			 int regaps_count = 0;
			 //Este ciclo vai ler para uma matriz bi-dim todos os registos existentes de Profissionais de Saúde, e envia para o cliente
			 while(fgets(regexist_aps[regaps_count], sizeof(regexist_aps[regaps_count]), newreg_aps) != NULL){
				 fflush(stdin);
				 
				 regexist_aps[regaps_count][strcspn(regexist_aps[regaps_count], "\n")] = 0; /*Como o fgets lê a linha toda incluindo o "\n" após a password, este tem
																																			que ser removido as comparações funcionarem corretamente*/
				 
				 write(client_fd, regexist_aps[regaps_count], sizeof(regexist_aps[regaps_count]));
				 fflush(stdout);
				 sleep(0.5);
				 regaps_count++;
			 }
			 //Quando acaba de enviar os registos, envia "Done" ao cliente para ele sair do ciclo respetivo
			 write(client_fd, "Done", sizeof("Done"));
			 fflush(stdout);
			 
			 printf("\nLista de registos de saúde (%d quant): \n", regaps_count);
			 for(int i = 0; i < regaps_count; i++){
				 printf("\n%d - %s", i, regexist_aps[i]);
			 }
			 
			 printf("\n\nRegistos de saúde enviados!\n");
			 fclose(newreg_aps);
			 sleep(1);
			 
			 //Este ciclo vai ler para uma matriz bi-dim todos os registos existentes de Agentes de Segurança, e envia para o cliente
			 int regaas_count = 0;
			 while(fgets(regexist_aas[regaas_count], sizeof(regexist_aas[regaas_count]), newreg_aas) != NULL){
				 fflush(stdin);
				 
				regexist_aas[regaas_count][strcspn(regexist_aas[regaas_count], "\n")] = 0; /*Como o fgets lê a linha toda incluindo o "\n" após a password, este tem
																																			que ser removido as comparações funcionarem corretamente*/
				 
				 write(client_fd, regexist_aas[regaas_count], sizeof(regexist_aas[regaas_count]));
				 fflush(stdout);
				 sleep(0.5);
				 regaas_count++;
			 }
			 //Quando acaba de enviar os registos, envia "Done" ao cliente para ele sair do ciclo respetivo
			 write(client_fd, "Done", sizeof("Done"));
			 fflush(stdout);
			 
			 printf("\nLista de registos de segurança (%d quant): \n", regaas_count);
			 for(int i = 0; i < regaas_count; i++){
				 printf("\n%d - %s", i, regexist_aas[i]);
			 }
			 
			 printf("\n\nRegistos de segurança enviados!\n");
			 fclose(newreg_aas);
			 sleep(1);
			 
			//Aqui lê o registo já editado pelo Gestor
			nread = read(client_fd, registo, BUF_SIZE-1);
			fflush(stdin);
			registo[nread] = '\0';
			
			const char separador[2] = "\\"; //Delimitador
			char *id_tok;
			char aux[250] = "";
			
			//Copia o registo para uma variável auxiliar para usar com o strtok, e vai buscar o ID
			strcpy(aux, registo);
			id_tok = strtok(aux, separador);
			
			//Neste ciclo percorre a lista de registos de Saúde existentes
			for(int i = 0; i < regaps_count; i++){
				//Se o ID do registo editado corresponder a um ID dos registos existentes, substitui o registo pelo novo, e sai do ciclo
				if(strncmp(regexist_aps[i], id_tok, 8) == 0){
					strcpy(regexist_aps[i], registo);
					printf("Registo aceite: %s\n", registo);
					break;
				}
			}
			 
			 //Neste ciclo percorre a lista de registos de Segurança existentes
			for(int i = 0; i < regaas_count; i++){
				//Se o ID do registo editado corresponder a um ID dos registos existentes, substitui o registo pelo novo, e sai do ciclo
				if(strncmp(regexist_aas[i], id_tok, 8) == 0){
					 strcpy(regexist_aas[i], registo);
					 printf("Registo aceite: %s\n", registo);
					 break;
				}
			}
			
			 newreg_aps = fopen("DATABASE Files/credenciais_saude.txt", "w");
			 newreg_aas = fopen("DATABASE Files/credenciais_psp.txt", "w");
			 
			 //Este ciclo vai percorrer a lista de registos de Saúde existentes, e escreve no ficheiro
			 for(int i = 0; i < regaps_count; i++){
				 fprintf(newreg_aps, "%s\n", regexist_aps[i]);
				 fflush(stdout);
			 }
			 fclose(newreg_aps);
			 
			 //Este ciclo vai percorrer a lista de registos de Segurança existentes, e escreve no ficheiro
			 for(int i = 0; i < regaas_count; i++){
				 fprintf(newreg_aas, "%s\n", regexist_aas[i]);
				 fflush(stdout);
			 }
			 fclose(newreg_aas);
			 
			 printf("\n\nEdição de registos aceites com sucesso!\n");
			 sleep(2);
		 }
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//              												         OPERAÇÃO 9 - APAGAR REGISTOS 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
		 if(operacao[0] == '9'){
			 header("AGS", "REG DEL AUTH");
			 newreg_aps = fopen("DATABASE Files/credenciais_saude.txt", "r");
			 newreg_aas = fopen("DATABASE Files/credenciais_psp.txt", "r");
			 
			 int regaps_count = 0;
			 //Este ciclo vai ler para uma matriz bi-dim todos os registos existentes de Profissionais de Saúde, e envia para o cliente
			 while(fgets(regexist_aps[regaps_count], sizeof(regexist_aps[regaps_count]), newreg_aps) != NULL){
				fflush(stdin);
				
				regexist_aps[regaps_count][strcspn(regexist_aps[regaps_count], "\n")] = 0; /*Como o fgets lê a linha toda incluindo o "\n" após a password, este tem
																																			que ser removido as comparações funcionarem corretamente*/
				
				write(client_fd, regexist_aps[regaps_count], sizeof(regexist_aps[regaps_count]));
				fflush(stdout);
				sleep(0.5);
				regaps_count++;
			 }
			 //Quando acaba de enviar os registos, envia "Done" ao cliente para ele sair do ciclo respetivo
			 write(client_fd, "Done", sizeof("Done"));
			 fflush(stdout);
			 
			 printf("\nLista de registos de saúde (%d quant): \n", regaps_count);
			 for(int i = 0; i < regaps_count; i++){
				 printf("\n%d - %s", i, regexist_aps[i]);
			 }
			 
			 printf("\n\nRegistos de saúde enviados!\n");
			 fclose(newreg_aps);
			 sleep(1);
			 
			 //Este ciclo vai ler para uma matriz bi-dim todos os registos existentes de Agentes de Segurança, e envia para o cliente
			 int regaas_count = 0;
			 while(fgets(regexist_aas[regaas_count], sizeof(regexist_aas[regaas_count]), newreg_aas) != NULL){
				 fflush(stdin);
				 
				 regexist_aas[regaas_count][strcspn(regexist_aas[regaas_count], "\n")] = 0; /*Como o fgets lê a linha toda incluindo o "\n" após a password, este tem
																																				que ser removido as comparações funcionarem corretamente*/
				 
				 write(client_fd, regexist_aas[regaas_count], sizeof(regexist_aas[regaas_count]));
				 fflush(stdout);
				 sleep(0.5);
				 regaas_count++;
			 }
			 //Quando acaba de enviar os registos, envia "Done" ao cliente para ele sair do ciclo respetivo
			 write(client_fd, "Done", sizeof("Done"));
			 fflush(stdout);
			 
			 printf("\nLista de registos de segurança (%d quant): \n", regaas_count);
			 for(int i = 0; i < regaas_count; i++){
				 printf("\n%d - %s", i, regexist_aas[regaas_count]);
			 }
			 
			 printf("\n\nRegistos de segurança enviados!\n");
			 fclose(newreg_aas);
			 sleep(1);
			 
			 //Aqui lê um ID que o Gestor quer apagar
			 nread = read(client_fd, id_accept, BUF_SIZE-1);
			 fflush(stdin);
			 id_accept[nread] = '\0';
			 
			//Neste ciclo percorre a lista de registos de Saúde existentes
			for(int i = 0; i < regaps_count; i++){
				//Se o ID a apagar corresponder a um ID dos registos existentes, substitui o registo por "DEL"
				if(strncmp(regexist_aps[i], id_accept, 8) == 0){
					printf("Registo apagado: %s\n", regexist_aps[i]);
					fflush(stdout);
					strcpy(regexist_aps[i], "DEL");
					break;
				}
			}
			
			//Neste ciclo percorre a lista de registos de Segurança existentes
			for(int i = 0; i < regaas_count; i++){
				//Se o ID a apagar corresponder a um ID dos registos existentes, substitui o registo por "DEL"
				if(strncmp(regexist_aas[i], id_accept, 8) == 0){
					printf("Registo apagado: %s\n", regexist_aas[i]);
					fflush(stdout);
					strcpy(regexist_aas[i], "DEL");
					break;
				}
			}
			 
			 newreg_aps = fopen("DATABASE Files/credenciais_saude.txt", "w");
			 newreg_aas = fopen("DATABASE Files/credenciais_psp.txt", "w");
			 
			 //Este ciclo vai percorrer a lista de registos de Saúde existentes, e escreve no ficheiro
			 for(int i = 0; i < regaps_count; i++){
				 //Se a string for "DEL", salta à frente e não escreve nada pois este registo foi apagado
				 if(strcmp(regexist_aps[i], "DEL") == 0){
					 continue;
				 }
				 fprintf(newreg_aps, "%s\n", regexist_aps[i]);
				 fflush(stdout);
			 }
			 fclose(newreg_aps);
			 
			 //Este ciclo vai percorrer a lista de registos de Segurança existentes, e escreve no ficheiro
			 for(int i = 0; i < regaas_count; i++){
				 //Se a string for "DEL", salta à frente e não escreve nada pois este registo foi apagado
				 if(strcmp(regexist_aas[i], "DEL") == 0){
					 continue;
				 }
				 fprintf(newreg_aas, "%s\n", regexist_aas[i]);
				 fflush(stdout);
			 }
			 fclose(newreg_aas);
	
			 printf("\n\Eliminação de registos aceites com sucesso!\n");
			 sleep(2);
		 }
	 }
 }
//Esta função serve apenas para efeitos de composição gráfica, em que escreve o banner no topo do ecrã com os dados recebidos
void header(char client[3], char type[5]){
	printf("===============CONEXÃO %s - %s===============", client, type);
	return;
}
//Esta função serve apenas para apresentar erros durante a ligação TCP
void erro(char *msg){
	printf("Erro: %s\n", msg);
	exit(-1);
}
