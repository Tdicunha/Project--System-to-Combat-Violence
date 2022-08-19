//APLICAÇÃO PARA PROFISSIONAL DE SAÚDE (APS)

//SOCKET USADO: UDP (SENDTO, RECVFROM)

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

//Definição de uma struct que contém todos os parâmetros presentes num registo
typedef struct Utilizador{
	char id[10];
	char username[100];
	char password[100];
} Utilizador;

//Definição de uma struct que contém todos os parâmetros presentes num registo de crime
struct infomarcao{
    char data[40];
    char hora[10];
    char local[110];
    char crime[50];
    char nome[50];
}info;

//Definição das funções necessárias para o header, apresentação de erros, e chat de texto
void header(void);
void erro(char *msg);
void chat_AAS(int fd_chat, char cliente[50]);

int main(){
	//Inicio da definição do servidor
	int clientSocket, nBytes;
	struct sockaddr_in serverAddr;
	socklen_t addr_size;
	
	clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(7000);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	addr_size = sizeof serverAddr;
	//Fim da definição do servidor	
	
	//Variáveis de navegação no menu
	int menu1;
	int menu2;
	int menuregisto;
	char menuapagar;
	char returnmenu;
	
	//Variáveis de credenciais e dados
	Utilizador user;
	Utilizador edit;
	char registo[250] = "";
	int dia, mes, ano;
	char anonimato;
	char crime[300] = "";
	//Variáveis time para permitir ir buscar a data do sistema no botão de emergência
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	
	//Variável usada na apresentação do ficheiro de texto "Help"
	char help[5000] = "";
	
	//Ciclo while para correr o programa sem terminar, a não ser que seja forçado (no caso da saida)
	/*O uso de fflush(stdout) e fflush(stdin) é bastante usado em todo o programa, para limpar as streams de entrada
	e saida e evitar lixo nas mesmas*/
	/*Para além disso em todas as aplicações de cliente (Profissional de Saúde, Agente de Segurança e Gestor de Sistema
	 é usado o o system("clear") para limpar o ecrã e permitir uma apresentação mais limpa*/
	while(1){
		//Menu inicial de login/registo
		loginmenu:
		//Ciclo while em que só sai com os inputs corretos do utilizador
		while(1){
			menu1 = 0;
			system("clear");
			header();
			printf("\n\n1 - Login\n");
			printf("2 - Registo\n");
			printf("3 - ⚠️  EMERGÊNCIA ⚠️\n");
			printf("4 - Sair\n");
			printf("\nInsira a sua escolha: ");
			scanf("%d", &menu1);
			fflush(stdin);
			
			while(getchar() != '\n');
			
			if(menu1 > 0 && menu1 < 5){
				break;
			}
		}
		//Após sair do ciclo e entrar no if correspondente, a primeira ação em todos é enviar ao servidor o código da operação
		
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                										OPCAO 1 - LOGIN 
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
		if(menu1 == 1){
			nBytes = strlen("1")+1;
			sendto(clientSocket, "1", nBytes, 0, (struct sockaddr *)&serverAddr, addr_size);
			fflush(stdout);
			
			char confirm[50] = "";
			
			while(1){
				system("clear");
				header();
				//Aqui vai pedir o ID ao utilizador, mas com duas salvaguardas: que o formato e o tamanho são corretos
				do{
					do{
						printf("\n\nInsira o seu ID de profissional (APS_xxxx): ");
						scanf("%s", user.id);
						fflush(stdin);
						
						if(strlen(user.id) != 8){
							printf("\nID inserido tem um número de caracteres incorreto, tente novamente!");
						}
					}while(strlen(user.id) != 8); //Só sai deste ciclo quando o tamanho do ID for igual a 8
					
					if(strncmp(user.id, "APS_", 4) != 0){
						printf("\nID inserido não corresponde ao exemplo, tente novamente!\n");
					}
				}while(strncmp(user.id, "APS_", 4) != 0); //Só sai deste ciclo quando o ID começar com "APS_"
				//Quando passar as verificações, envia o ID ao servidor
				nBytes=strlen(user.id)+1;
				sendto(clientSocket,user.id,nBytes,0,(struct sockaddr *)&serverAddr,addr_size);
				fflush(stdout);
				
				//Aqui vai pedir a password ao utilizador, com a salvaguarda do tamanho ser inferior ou igual a 50 char
				do{
					printf("\nInsira a sua password: ");
					scanf("%s", user.password);
					fflush(stdin);
					
					while(getchar() != '\n');
					
					if(strlen(user.password) > 50){
						printf("\nPassword comprida demais, tente novamente!\n");
					}
				}while(strlen(user.password) > 50); //Só sai deste ciclo quando o tamanho da password for menor ou igual que 50
				//Quando passar a verificação, envia a password ao servidor
				nBytes=strlen(user.password)+1;
				sendto(clientSocket,user.password,nBytes,0,(struct sockaddr *)&serverAddr,addr_size);			
				fflush(stdout);
				printf("\nA confirmar...\n\n");
				fflush(stdout);
				
				//Recebe confirmação do servidor de volta
				nBytes = recvfrom(clientSocket,confirm,50,0,NULL, NULL);
				confirm[nBytes]='\0';
				fflush(stdin);
				
				/*Se a confirmação for "NEW", significa que o registo associado ao ID ainda não foi aprovado,
				informa o utilizador e volta ao menu de login*/
				if(strcmp(confirm, "NEW") == 0){
					printf("ID está pendente de aprovação pelo Gestor de Sistema, tente de novo mais tarde.\n");
					sleep(2);
					goto loginmenu;
				}
				
				/*Se a confirmação não for nem "NEW" nem "0", significa que as credenciais estão corretas e recebe o username,
				 informa o utilizador e segue para o menu principal*/
				if(strcmp(confirm, "NEW") != 0 && strcmp(confirm, "0") != 0){
					strcpy(user.username, confirm); //Copia o username para a struct user para apresentar no menu seguinte
					printf("Login com sucesso, a entrar no programa...");
					fflush(stdout);
					sleep(2);
					break;
				}
				
				/*Por default, se a confirmação não for nenhuma das anteriores, as credenciais estão erradas,
				informa o utilizador e volta ao menu de login*/
				printf("Credenciais erradas, a voltar ao menu anterior.\n");
				fflush(stdout);
				sleep(2);
				goto loginmenu;
			}
		}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//																	OPCAO 2 - REGISTO 
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
		if(menu1 == 2){
			nBytes = strlen("2")+1;
			sendto(clientSocket, "2", nBytes, 0, (struct sockaddr *)&serverAddr, addr_size);
			fflush(stdout);
			
			system("clear");
			header();
			
			//Tal como no Login, aqui vai pedir o ID ao utilizador, mas com duas salvaguardas: que o formato e o tamanho são corretos
			do{
				do{
					printf("\n\nInsira o seu ID de profissional (APS_xxxx): ");
					scanf("%s", user.id);
					fflush(stdin);
					while(getchar() != '\n');
					
					if(strlen(user.id) != 8){
						printf("\nID inserido tem um número de caracteres incorreto, tente novamente!");
					}
				}while(strlen(user.id)!=8); //Só sai deste ciclo quando o tamanho do ID for igual a 8
				
				if(strncmp(user.id,"APS_",4)!=0)
				{
					printf("\nID inserido não corresponde ao exemplo, tente novamente!\n");
					fflush(stdout);
				}
			}while(strncmp(user.id,"APS_",4)!=0); //Só sai deste ciclo quando o ID começar com "APS_"
			
			/*Aqui vai pedir o username ao utilizador, com a salvaguarda do tamanho ser inferior ou igual a 50 char,
			e pode conter espaços*/
			do{
				printf("\nInsira o seu username (máx 50 char): ");
				fgets(user.username, sizeof(user.username), stdin);
				fflush(stdin);
				
				while(getchar() != '\n');
				
				user.username[strcspn(user.username, "\n")] = 0; /*Como temos um fgets, temos que remover o '\n' da stream para
																									evitar passar o próximo input à frente*/

				if(strlen(user.username)>50){
					printf("\nUsername comprido demais, tente novamente!\n");
					fflush(stdout);
				}
			}while(strlen(user.username)>50 || strlen(user.username) < 2); /*Só sai deste cicilo quando o tamanho da password for menor ou igual que 50
																													 e maior que 2 para evitar confusões com o '\n'*/
			
			//Aqui vai pedir a password ao utilizador, com a salvaguarda do tamanho ser inferior ou igual a 50 char
			do{
				printf("\nInsira a sua password (máx 50 char): ");
				scanf("%s", user.password);
				fflush(stdin);
				
				while(getchar() != '\n');
				
				if(strlen(user.password)>50)
				{
					printf("\nPassword comprida demais, tente novamente!\n.");
					fflush(stdout);
				}
		    }while(strlen(user.password)>50 || strlen(user.password) < 2); /*Só sai deste cicilo quando o tamanho da password for menor ou igual que 50
																													e maior que 2 para evitar confusões com o '\n'*/
			
			//Sprintf vai juntar todos os 3 parâmetros do registo para enviar já formatado ao servidor com o formato ID/User/Pass
			sprintf(registo, "%s\\%s\\%s", user.id, user.username, user.password);
			
			//Envia string ao servidor
			nBytes = strlen(registo)+1;
			sendto(clientSocket, registo, nBytes, 0, (struct sockaddr *)&serverAddr, addr_size);
			fflush(stdout);
			
			char confirm[10] = "";
			
			//Recebe confirmação do servidor de volta
			nBytes = recvfrom(clientSocket, confirm, 10, 0, (struct sockaddr *)&serverAddr, &addr_size);
			confirm[nBytes] = '\0';
			fflush(stdin);
			
			/*Se a confirmação for "1", significa que o registo foi bem sucedido,
			informa o utilizador e volta ao menu de login*/
			if(strcmp(confirm, "1") == 0){
				printf("\nUtilizador registado com sucesso, poderá efetuar login quando for aprovado.\n\n");
				sleep(2);
				goto loginmenu;
			}
			
			/*Se a confirmação for "NEW", significa que já existe um registo por aprovar associado ao ID
			Se a confirmação for "USE", significa que o ID já pertence a outro utilizador registado
			Em qualquer um dos casos informa o utilizador e volta ao menu de login*/
			if(strcmp(confirm, "NEW") == 0 || strcmp(confirm, "USE") == 0){
				printf("\nID de utilizador já em uso, impossivel utilizar.\n\n");
				sleep(2);
				goto loginmenu;
			}
		}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                          								OPCAO 3 - BOTÃO DE EMERGÊNCIA
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
		if(menu1 == 3){
			//Inicio da definição do servidor
			int clientSocket, nBytes;
			struct sockaddr_in serverAddr;
			socklen_t addr_size;
	
			clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

			serverAddr.sin_family = AF_INET;
			serverAddr.sin_port = htons(9500);
			serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
			memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

			addr_size = sizeof serverAddr;
			//Fim da definição do servidor
			
			//Envia ao Agente de Segurança o código de operação 6, que corresponde ao botão de emergência
			nBytes = strlen("6")+1;
			sendto(clientSocket, "6", nBytes, 0, (struct sockaddr *)&serverAddr, addr_size);
			fflush(stdout);
			sleep(1);
			
			system("clear");
			header();
			char confirmar[5] = "";
			/*Matriz bi-dim com vários locais com alta taxa de criminalidade, para adicionar alguma "dinâmica" à localização
			do cliente que ativa o botão de emergência*/
			char locais[10][50] = {"Albufeira", "Lisboa", "Porto", "Almeida", "Lagoa", "Alvito", "Loulé", "Valença", "Ribeira Grande", "Vila do Bispo"};
			int local_numb = 0;
			char local_rand[50] = "";
			char data[15] = "", hora[10] = "";
			char emergencia[250] = "";
			
			//Srand com seed getpid() entre 0 e 10 para escolher aleatoriamente uma localização e guardar na string apropriada
			srand(getpid());
			local_numb = rand() % 10;
			strcpy(local_rand, locais[local_numb]);
			 
			 printf("\n\n*************************************\n*****\t ALERTA EMERGÊNCIA\t*****\n*************************************\n\n");
			 printf("A estabelecer contacto com um agente de segurança...\n\n");
			 
			 //Para as strings data e hora, os dados são recolhidos através do sistema, com recurso à biblioteca <time.h>
			 sprintf(data, "%d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
			 sprintf(hora, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
			 
			 printf("Data: %s\nHora: %s\n", data, hora);
			 printf("Local (obtido por GPS): %s\n", local_rand);
			 
			 /*Formata automaticamente a string para enviar ao Agente de Segurança com os dados necessários no formato
			 Data/Hora/Local, e envia*/
			 sprintf(emergencia, "%s\\%s\\%s", data, hora, local_rand);
			 nBytes = strlen(emergencia) + 1;
			 sendto(clientSocket, emergencia, nBytes, 0, (struct sockaddr *)&serverAddr, addr_size);
			 fflush(stdout);
			 sleep(0.5);
			 
			 /*Espera pela confirmação do Agente de Segurança, e quando receber (recebe sempre), informa e volta para
			 o menu de login*/
			 printf("\nÀ espera de confirmação de assistência de um agente...\n");
			 nBytes = recvfrom(clientSocket, confirmar, 5, 0, (struct sockaddr *)&serverAddr, &addr_size);
			 fflush(stdin);
			 
			 printf("\nAgente de segurança vem a caminho!!\n");
			 fflush(stdout);
			 sleep(10);
			 goto loginmenu;
		}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                          											OPCAO 4 - SAIR
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
		if(menu1 == 4){
			//Limpa as streams de dados stdin e stdout, apresenta a mensagem de copyright e termina o programa
			fflush(stdout);
			fflush(stdin);
			
			printf("\nCopyright (c) [2021] Luis Gaspar (2018274645), Telmo Cunha (2018308321).");
			
			exit(1);
		}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                              										MENU PRINCIPAL
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
		//Menu principal após o login
		mainmenu:
		//Ciclo while que só permite sair do menu quando o input se encontrar no intervalo permitido
		while(1){
			menu2 = 0;
			system("clear");
			header();
			printf("\n\nSeja bem vindo, %s", user.username);
			printf("\n\n1 - Registar crime\n");
			printf("2 - Gerir registo\n");
			printf("3 - Sair da aplicação\n");
			printf("4 - Chat (BETA)\n");
			printf("5 - Help\n");
			printf("\nInsira a sua escolha: ");
			scanf("%d", &menu2);
			fflush(stdin);
			
			while(getchar() != '\n');
			
			if(menu2 > 0 && menu2 < 6){
				break;
			}
		}
		//Após sair do ciclo e entrar no if correspondente, a primeira ação em todos é enviar ao servidor o código da operação

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                               							OPÇAO 1 - REGISTO DE CRIMES
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
		if(menu2 == 1){
			nBytes = strlen("7")+1;
			sendto(clientSocket, "7", nBytes, 0, (struct sockaddr*)&serverAddr, addr_size);
			fflush(stdout);
			
			returnmenu = 'a';
			anonimato = 'a';
			
			system("clear");
			header();
			
			//No caso de o utilizador querer manter o anonimato da submissão, pode escolher aqui
		    while( anonimato != 'y' && anonimato != 'Y' && anonimato != 'n' && anonimato != 'N'){
				printf("\n\nDeseja realizar o registo de forma anônima? (Y/N): ");
				scanf(" %c", &anonimato);
				fflush(stdin);
			}
			
			
			printf("\n\nFormulário de registo de crime:\n");
			//Ciclo para pedir o um dia entre 1 e 31
			while(1){
				printf("\nInsira o dia (1-31): ");
				scanf("%d", &dia);
				fflush(stdin);
				
				while(getchar() != '\n');
				
				if(dia > 0 && dia < 32){
					break;
				}
			}
			
			//Ciclo para pedir o um mês entre 1 e 12
			while(1){
				printf("Insira o mês (1-12): ");
				scanf("%d", &mes);
				fflush(stdin);
				
				while(getchar() != '\n');
				
				if(mes > 0 && mes < 13){
					break;
				}
			}
			
			//Ciclo para pedir o um ano entre 2000 e 2150
			while(1){
				printf("Insira o ano (2000-2150): ");
				scanf("%d", &ano);
				fflush(stdin);
				
				while(getchar() != '\n');
				
				if(ano > 1999 && ano < 2151){
					break;
				}
			}
			
			//Formata a data corretamente: DD/MM/AAAA
			sprintf(info.data, "%d-%d-%d", dia, mes, ano);
			
			//Aqui pede a hora da ocorrência do crime, e só sai quando se encontrar no intervalo permitido
			while(1){
				printf("\nInsira a hora da ocorrência (00:00 - 23:59): ");
				scanf("%s", info.hora);
				fflush(stdin);
				
				while(getchar() != '\n'); //Para evitar problemas de '\n' no próximo input
				
				const char separador[2] = ":"; //Delimitador
				char *hora_tok, *min_tok;
				int hora, min;
				char aux[10] = "";
				
				strcpy(aux, info.hora); //Copiar a string para um auxiliar para usar o strtok
				hora_tok = strtok(aux, separador);
				min_tok = strtok(NULL, separador);
				hora = atoi(hora_tok); //Converter string para inteiro para se poder verificar o intervalo
				min = atoi(min_tok);
				
				if(hora >= 0 && hora < 24 && min >= 0 && min < 60){
					//Formata a hora corretamente: HH:MM e sai do ciclo caso se encontre no intervalo
					sprintf(info.hora, "%02d:%02d", hora, min);
					break;
				}
			}
			
			//Ciclo para pedir o local da ocorrência, com fgets para permitir locais com espaços
			while(1){
				printf("\nInsira o local da ocorrência (máx 100 char): ");
				fgets(info.local, sizeof(info.local), stdin);
				fflush(stdin);
				
				while(getchar() != '\n');
				
				info.local[strcspn(info.local, "\n")] = 0; //Utilização do strcspn para evitar buffer do '\n' para o input seguinte
				
				if(strlen(info.local) > 0 && strlen(info.local) < 101){
					break;
				}
			}
			
			//Ciclo para pedir o tipo de crime, com fgets para permitir tipos com espaços
			while(1){
				printf("\nInsira o tipo de crime (máx 40 char): ");
				fgets(info.crime, sizeof(info.crime), stdin);
				fflush(stdin);
				
				while(getchar() != '\n');
				
				info.crime[strcspn(info.crime, "\n")] = 0; //Utilização do strcspn para evitar buffer do '\n' para o input seguinte
				
				if(strlen(info.crime) > 0 && strlen(info.crime) < 41){
					break;
				}
			}
			
			/*No caso do cliente ter escolhido anonimato anteriormente, escreve "ANON" no lugar do nome para o
			Agente de Segurança identificar facilmente o anonimato*/
			if( anonimato == 'y' || anonimato =='Y'){
				strcpy(info.nome,"ANON");
			}
			//No caso de não ser denuncia anónima, pede o nome ao cliente, ou 0 caso seja o próprio cliente a vitima
			else if(anonimato != 'y' && anonimato !='Y')
			{
				while(1){
					printf("\nInsira o nome da vitima (máx 40 char, 0 se for o utilizador): ");
					fgets(info.nome, sizeof(info.nome), stdin);
					fflush(stdin);
					
					info.nome[strcspn(info.nome, "\n")] = 0;
					
					if(strcmp(info.nome, "0") == 0){
						strcpy(info.nome, user.username); //Se o cliente for a vitima, copia-se o nome recebido no login para a string
					}
					
					if(strlen(info.nome) > 0 && strlen(info.nome) < 41){
						break;
					}
				}
		    }
		    
		    //Formata o crime na forma correta: "Data";"Hora";"Local";"Tipo";"Vitima", e envia ao servidor
			sprintf(crime, "\"%s\";\"%s\";\"%s\";\"%s\";\"%s\"", info.data, info.hora, info.local, info.crime, info.nome);
			
			nBytes = strlen(crime) + 1;
			sendto(clientSocket, crime, nBytes,0,(struct sockaddr *)&serverAddr,addr_size);
			printf("\nObrigado pela sua colaboracao!\n"); 
			
			//Confirmação para sair para o menu anterior
			while(returnmenu != 's' && returnmenu != 'S'){
				printf("\nPressione S para voltar ao menu anterior: ");
				scanf(" %c", &returnmenu);
				fflush(stdin);
			}
			goto mainmenu;
		}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                								OPÇAO 2 - GERIR REGISTO
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
		//Menu de gestão do registo, só sai com uma opção correta
		if(menu2 == 2){
			while(1){
				menuregisto = 0;
				system("clear");
				header();
				printf("\n\n1 - Alterar registo\n");
				printf("2 - Apagar registo\n");
				printf("3 - Voltar ao menu anterior\n\n");
				printf("Insira a sua escolha: ");
				scanf("%d", &menuregisto);
				fflush(stdin);
				
				while(getchar() != '\n');
				
				if(menuregisto > 0 && menuregisto < 4){
					break;
				}
			}
			//Após sair do ciclo e entrar no if correspondente, a primeira ação em todos é enviar ao servidor o código da operação
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                               							 OPÇAO 2.1 - ALTERAR REGISTO
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
			if(menuregisto == 1){
				nBytes = strlen("8")+1;
				sendto(clientSocket, "8", nBytes, 0, (struct sockaddr *)&serverAddr, addr_size);
				fflush(stdout);
				
				system("clear");
				header();
				//Semelhante ao que acontece no Registo, aqui pede o novo username, ou 0 para manter
				while(1){
					printf("\n\nInsira o seu novo nome (0 para manter, máx 100 char): ");
					fgets(edit.username, sizeof(edit.username), stdin);
					fflush(stdin);
					
					while(getchar() != '\n');
					
					edit.username[strcspn(edit.username, "\n")] = 0; //Utilização do strcspn para evitar buffer do '\n' para o input seguinte
					
					if(edit.username[0] == '0'){
						strcpy(edit.username, user.username); //Caso o cliente não mude o username, copia o atual para a string
					}
					
					if(strlen(edit.username) > 0 && strlen(edit.username) <= 100){
						break;
					}
				}
				
				//Semelhante ao que acontece no Registo, aqui pede a nova password, ou 0 para manter
				while(1){
					printf("Insira a sua nova password (0 para manter): ");
					scanf("%s", edit.password);
					fflush(stdin);
					if(edit.password[0] == '0'){
						strcpy(edit.password, user.password); //Caso o cliente não mude a password, copia a atual para a string
					}
					
					if(strlen(edit.password) > 0 && strlen(edit.password) <= 100){
						break;
					}
				}
				
				//Junta os 3 parâmetros no formato correto e envia ao servidor
				sprintf(registo, "%s\\%s\\%s", user.id, edit.username, edit.password);
				nBytes = strlen(registo)+1;
				sendto(clientSocket, registo, nBytes, 0, (struct sockaddr *)&serverAddr, addr_size);
				fflush(stdout);
				
				printf("Registo alterado com sucesso!\n");
				
				returnmenu = 'a';
				//Confirmação para sair para o menu anterior
				while(returnmenu != 's' && returnmenu != 'S'){
					printf("\nPressione S para sair: ");
					scanf(" %c", &returnmenu);
					fflush(stdin);
				}
				goto mainmenu;
			}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                             							   OPÇAO 2.2 - APAGAR REGISTO
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
			if(menuregisto == 2){
				menuapagar = 'a';
				//Double check de se o utilizador quer mesmo apagar o registo ou não, com Y/N
				while(menuapagar != 'y' && menuapagar != 'Y' && menuapagar != 'n' && menuapagar != 'N'){
					system("clear");
					header();
					printf("\n\nDeseja mesmo apagar o seu registo (Y/N): ");
					scanf(" %c", &menuapagar);
					fflush(stdin);
				}
				
				//Caso o utilizador escolha apagar, pede a password por razões de confirmação de segurança
				if(menuapagar == 'y' || menuapagar == 'Y'){
					char pw_conf[50] = "";
					printf("\nInsira a sua password para confirmar: ");
					scanf("%s", pw_conf);
					
					/*Se a password for a mesma que está guardada na struct Utilizador, envia ao servidor o código de operação 9
					e envia o registo a apagar*/
					if(strcmp(pw_conf, user.password) == 0){
						nBytes = strlen("9")+1;
						sendto(clientSocket, "9", nBytes, 0, (struct sockaddr *)&serverAddr, addr_size);
						fflush(stdout);
						
						//Formata o registo com os dados do utilizador para facilitar a eliminação no servidor
						sprintf(registo, "%s\\%s\\%s", user.id, user.username, user.password);
						nBytes = strlen(registo)+1;
						sendto(clientSocket, registo, nBytes, 0, (struct sockaddr *)&serverAddr, addr_size);
						fflush(stdout);
					
						system("clear");
						header();
						
						printf("\n\nRegisto apagado com sucesso!\n");
						
						returnmenu = 'a';
						//Confirmação para sair para o menu anterior
						while(returnmenu != 's' && returnmenu != 'S'){
							printf("\nPressione S para sair do programa: ");
							scanf(" %c", &returnmenu);
							fflush(stdin);
						}
						/*Quanto o utilizador sai do menu após apagar o registo, a struct com os dados é limpa, e o programa sai
						para o menu de login*/
						strcpy(user.id, "");
						strcpy(user.username, "");
						strcpy(user.password, "");
						goto loginmenu;
					}
					
					//No caso da password de confirmação ser errada, a operação cancela e volta ao menu principal
					if(strcmp(pw_conf, user.password) != 0){
						printf("\nPassword errada, operação cancelada.");
						fflush(stdout);
						sleep(2);
						goto mainmenu;
					}
				}
				
				//No caso do utilizador escolher não apagar o registo, nada acontece
				if(menuapagar == 'n' || menuapagar == 'N'){
					system("clear");
					header();
					printf("\n\nOperação cancelada!\n");

					returnmenu = 'a';
					//Confirmação para sair para o menu anterior
					while(returnmenu != 's' && returnmenu != 'S'){
						printf("\nPressione S para voltar ao menu principal: ");
						scanf(" %c", &returnmenu);
						fflush(stdin);
					}
					goto mainmenu;
				}
			}
			
			if(menuregisto == 3){
				goto mainmenu;
			}
		}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                               						 OPÇAO 3 - SAIR DO PROGRAMA
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
		if(menu2 == 3){
			//Limpa as streams de dados stdin e stdout, apresenta a mensagem de copyright e termina o programa
			fflush(stdout);
			fflush(stdin);
			
			printf("\nCopyright (c) [2021] Luis Gaspar (2018274645), Telmo Cunha (2018308321).");
			
			exit(1);
		}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                									OPÇAO 4 - CHAT
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
		if(menu2 == 4){
			/*Nesta operação criamos uma ligação direta entre o Agente de Segurança e o Profissional de Saúde
			com um socket TCP para permitir a máxima fiabilidade nas mensagens e sem precisar de passos adicionais
			a passarem pela Aplicação Central*/
			system("clear");
			header();
			
			printf("\n\nChat de texto\n");
			printf("\nEstamos a encontrar um agente para entrar em contacto...");
			fflush(stdout);
			
			//Inicio da definição do servidor
			#define SERVER_Port  8700
			int fd_chat, client;
			struct sockaddr_in addr, client_addr;
			int client_addr_size;
			//bzero((void *) &addr, sizeof(addr));
			addr.sin_family      = AF_INET;
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
			addr.sin_port        = htons(SERVER_Port);
			if ( (fd_chat = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
				erro("na funcao socket");
			//No caso de erro de bind, volta ao menu principal
			if ( bind(fd_chat,(struct sockaddr*)&addr,sizeof(addr)) < 0){
				erro("na funcao bind\n A voltar ao menu principal...");
				sleep(2);
				goto mainmenu;
			}
			if( listen(fd_chat, 5) < 0) 
				erro("na funcao listen");

			int nclientes=0;

			while (1){
				client_addr_size = sizeof(client_addr);
				//Espera a ligação do Agente de Segurança para dar accept
				client = accept(fd_chat,(struct sockaddr *)&client_addr, &client_addr_size);
				nclientes++;
				if (client > 0){
					close(fd_chat);
					chat_AAS(client, user.username); //Função de chat, semelhante às funções TCP na Aplicação Central
					fflush(stdout);
				}
				close(client);
				break;
			}//Fim da definição do servidor
			fflush(stdout);
			goto mainmenu;
		}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                             										OPÇAO 5 - HELP
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
		if(menu2 == 5){
			FILE *fhelp;
			
			//Aqui abre o ficheiro que contém as informações de Ajuda neste cliente
			fhelp = fopen("HELP Files/HELP_APS.txt", "r");
			system("clear");
			header();
			printf("\n");
			fflush(stdin);
			
			//Lê o conteúdo do ficheiro e apresenta imediatamente no ecrã, de forma a ficar corretamente formatado
			while(fgets(help, sizeof(help), fhelp)){
				printf("%s", help);
			}
			fclose(fhelp);
			
			returnmenu = 'a';
			//Confirmação para sair para o menu anterior
			while(returnmenu != 's' && returnmenu != 'S'){
				printf("\n\nPressione S para voltar ao menu principal: ");
				scanf(" %c", &returnmenu);
			}
			goto mainmenu;
		}
	}
	return 0;
}

//Esta função serve apenas para efeitos de composição gráfica, em que escreve o banner no topo do ecrã
void header(void){
	printf("===============APS - Aplicação para o Profissional de Saúde===============");
	return;
}
//Esta função serve apenas para apresentar erros durante a ligação TCP
void erro(char *msg){
	printf("\n\nErro: %s\n", msg);
	return;
}
//Função de servidor para o chat entre o Profissional de Saúde e o Agente de Segurança
void chat_AAS(int fd_chat, char cliente[50]){
	int nread;
	char enviado[250] = "";
	char recebido[250] = "";
	
	char agente[50] = "";
	/*Para o Profissional de Saúde saber o Agente com quem está em contacto, o primeiro dado e receber
	é o nome do mesmo*/
	nread = read(fd_chat, agente, 50);
	fflush(stdin);
	agente[nread] = '\0';
	
	printf("\nAgente de Segurança encontrado! Ligação estabelecida.\n");
	printf("\nEstá em contacto com o Agente %s!\n", agente);
	fflush(stdout);
	
	/*Com o mesmo efeito, para o Agente de Segurança saber com quem está em contacto, é enviado de seguida o
	nome do Profissional de Saúde*/
	write(fd_chat, cliente, strlen(cliente));
	fflush(stdout);
	
	/*Um chat tem que ter a possibilidade de enviar e receber mensagens em simultâneo, sem esperar pela
	outra operação. Para isso ser possivel, após a conexão estar estabelecida, criamos um fork para ter dois
	processos, um a receber mensagens e outro a enviar.*/
	int chat = fork();
	
	//O processo filho do fork recebe as mensagens
	if(chat == 0){
		//Assim que o processo inicia, entra num ciclo while para poder receber mensagens sem sair constantemente
		while(1){
			//Recebe a mensagem para a string recebido
			nread = read(fd_chat, recebido, 250);
			fflush(stdin);
			recebido[nread] = '\0';
			
			/*No caso de interrupção abrupta da ligação, começa-se a receber lixo em ciclo infinito pela porta
			Se o processo detetar esse lixo, deteta que o servidor foi desligado de repente e sai do ciclo*/
			if(strcmp(recebido, "") == 0){
				printf("\n\nCliente desligado de repente.\n");
				fflush(stdout);
				sleep(2);
				break;
			}
			/*A string "EXIT" é recebida quando o Profissional de Saúde sai do chat, e ajuda a sincronizar as aplicações
			Quando recebe essa string, significa que já não vão ser recebidas mais mensagens, e então este processo
			sai do ciclo while*/
			if(strcmp(recebido, "EXIT") == 0){
				break;
			}
			
			/*Apresenta mensagem recebida com remetente (o Agente de Segurança neste caso)
			e apresenta prompt do outro processo para efeitos de apresentação mais limpa*/
			printf("\33[2K\rAgente %s: %s\n", agente, recebido);
			printf("\nMensagem (SAIR0xAA para sair): ");
			fflush(stdout);
			fflush(stdin);
		}
		fflush(stdout);
		exit(0); //Sai do processo para não haver processos zombie
	}
	//O processo pai do fork envia as mensagens
	else{
		//Assim que o processo inicia, entra num ciclo while para poder enviar mensagens sem sair constantemente
		while(1){
			printf("\nMensagem (SAIR0xAA para sair): ");
			fgets(enviado, 250, stdin); //Uso de fgets para permitir enviar mensagens com várias palavras
			fflush(stdin);
			
			enviado[strcspn(enviado, "\n")] = 0; //Utilização do strcspn para evitar buffer do '\n' para o input seguinte
			
			/*Quando o utilizador sai da aplicação com "SAIR0xAA", informamos a outra aplicação de que não há mais
			mensagens a ser enviadas após enviar o "EXIT". Isto vai permitir alguma sincronização de aplicações, sendo que
			a outra aplicação sai diretamente do ciclo de receção de mensagens. Já esta aplicação, sai do processo de envio, e
			usa kill com o ID do processo filho para matar o processo de receção de mensagens. Após isso, fecha a ligação, e
			sai do ciclo.*/
			if(strcmp(enviado, "SAIR0xAA") == 0){
				write(fd_chat, "EXIT", sizeof("EXIT"));
				fflush(stdout);
				close(fd_chat);
				
				kill(chat, SIGKILL);
				break;
			}
			
			//Envia a mensagem na string enviado
			write(fd_chat, enviado, strlen(enviado));
			fflush(stdout);
			
			//Apresenta a mensagens enviada com o remetente (o utilizador neste caso)
			printf("O utilizador: %s\n", enviado);
			fflush(stdout);
		}
		fflush(stdout);
		/*Não se usa exit(0) neste processo pois como é o processo pai, é o mesmo do resto do programa e não
		se pode fechar*/
	}
	//Espera pelo término do processo filho antes de continuar para a saida do chat
	wait(NULL);
	
	//Limpa as streams de entrada e saida
	fflush(stdin);
	fflush(stdout);

	//Apresenta mensagem de ligação terminada, e volta ao menu principal
	printf("Ligação terminada a voltar ao menu.\n");
	fflush(stdout);
	sleep(2);
	return;
}
