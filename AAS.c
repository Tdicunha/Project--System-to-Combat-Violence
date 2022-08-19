//APLICAÇÃO PARA AGENTE DE SEGURANÇA (AAS)

//SOCKET USADO: TPC (WRITE, READ)

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <signal.h>

//Definição de uma struct que contém todos os parâmetros presentes num registo
typedef struct Utilizador{
	char id[10];
	char username[100];
	char password[100];
} Utilizador;

#define BUF_SIZE 5000

//Definição das funções necessárias para o header, apresentação de erros, e chat de texto
void processo_cliente_servidor(int client_fd);
void erro(char *msg);
void header(void);

int main(){
		//Inicio da definição do servidor
		char endServer[100] = "127.0.0.1";
		int fd, nread;
		//int client;
		struct sockaddr_in addr;
		//client_addr;
		//int client_addr_size;
		struct hostent *hostPtr;

		if ((hostPtr = gethostbyname(endServer)) == 0) 
		{
			printf("Couldn t get host address.\n");
			exit(-1);
		}

		bzero((void *) &addr, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
		addr.sin_port = htons(8000);

		if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
		erro("socket");
		if( connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
		erro("Connect");
		//Fim da definição do servidor	

		//Variáveis de navegação no menu
		int menu1;
		int menu2;
		int menuregisto;
		int menucrimes;
		char menuapagar;
		char returnmenu;
		
		//Variáveis de credenciais e dados
		Utilizador user;
		Utilizador edit;
		char registo[250] = "";
		
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
				printf("3 - Sair\n");
				printf("\nInsira a sua escolha: ");
				scanf("%d", &menu1);
				fflush(stdin);
				
				while(getchar() != '\n');
				
				if(menu1 > 0 && menu1 < 4){
					break;
				}
			}
			//Após sair do ciclo e entrar no if correspondente, a primeira ação em todos é enviar ao servidor o código da operação

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                										OPCAO 1 - LOGIN 
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
			if(menu1 == 1){
				write(fd, "1", sizeof("1"));
				fflush(stdout);
				
				char confirm[50] = "";
				
				while(1){
					system("clear");
					header();
					//Aqui vai pedir o ID ao utilizador, mas com duas salvaguardas: que o formato e o tamanho são corretos
					do{
						do{
							printf("\n\nInsira o seu ID de profissional (AAS_xxxx): ");
							scanf("%s", user.id);
							fflush(stdin);
							
							if(strlen(user.id) != 8){
								printf("\nID inserido tem um número de caracteres incorreto, tente novamente!");
							}
						}while(strlen(user.id) != 8); //Só sai deste ciclo quando o tamanho do ID for igual a 8
						
						if(strncmp(user.id, "AAS_", 4) != 0){
							printf("\nID inserido não corresponde ao exemplo, tente novamente!\n");
						}
					}while(strncmp(user.id, "AAS_", 4) != 0); //Só sai deste ciclo quando o ID começar com "AAS_"
					//Quando passar as verificações, envia o ID ao servidor
					write(fd, user.id, strlen(user.id));
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
					write(fd, user.password, strlen(user.password));
					fflush(stdout);
					printf("\nA confirmar...\n\n");
					fflush(stdout);

					//Recebe confirmação do servidor de volta
					nread=read(fd, confirm, BUF_SIZE-1);
					confirm[nread] = '\0'; 
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
				write(fd, "2", sizeof("2"));
				fflush(stdout);
				
				system("clear");
				header();

				//Tal como no Login, aqui vai pedir o ID ao utilizador, mas com duas salvaguardas: que o formato e o tamanho são corretos
				do{
					do{
						printf("\n\nInsira o seu ID de profissional (AAS_xxxx): ");
						scanf("%s", user.id);
						fflush(stdin);
						while(getchar() != '\n');
						
						if(strlen(user.id) != 8){
							printf("\nID inserido tem um número de caracteres incorreto, tente novamente!");
						}
					}while(strlen(user.id)!=8); //Só sai deste ciclo quando o tamanho do ID for igual a 8
					
					if(strncmp(user.id,"AAS_",4)!=0)
					{
						printf("\nID inserido não corresponde ao exemplo, tente novamente!\n");
					}
				}while(strncmp(user.id,"AAS_",4)!=0); //Só sai deste ciclo quando o ID começar com "AAS_"
					
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
					}
				}while(strlen(user.username)>50 || strlen(user.username) < 2); /*Só sai deste cicilo quando o tamanho da password for menor ou igual que 50
																														e maior que 2 para evitar confusões com o '\n'*/
				
				//Aqui vai pedir a password ao utilizador, com a salvaguarda do tamanho ser inferior ou igual a 50 char
				do{
					printf("\nInsira a sua password (máx 50 char): ");
					scanf("%s", user.password);
					fflush(stdin);
					
					while(getchar() != '\n');
					
					if(strlen(user.password)>50){
						printf("\nPassword comprida demais, tente novamente!\n");
					}
				}while(strlen(user.password)>50 || strlen(user.password) < 2); /*Só sai deste cicilo quando o tamanho da password for menor ou igual que 50
																														e maior que 2 para evitar confusões com o '\n'*/

				//Sprintf vai juntar todos os 3 parâmetros do registo para enviar já formatado ao servidor com o formato ID/User/Pass
				sprintf(registo, "%s\\%s\\%s", user.id, user.username, user.password);

				//Envia string ao servidor
				write(fd, registo, strlen(registo));
				fflush(stdout);
				
				char confirm[10] = "";
				
				//Recebe confirmação do servidor de volta
				nread = read(fd, confirm, BUF_SIZE-1);
				confirm[nread] = '\0';
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
//                          											OPCAO 3 - SAIR
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
			if(menu1 == 3){
				/*Como é um servidor TCP, o cliente e o servidor estão ligados. Para evitar interrupções abruptas e lixo na porta,
				enviamos o código de operação 3 ao servidor, que faz com que esse feche a ligação*/
				//Limpa as streams de dados stdin e stdout, apresenta a mensagem de copyright e termina o programa
				write(fd, "3", sizeof("3"));
				fflush(stdout);
				fflush(stdin);
				
				close(fd);
				
				printf("\nCopyright (c) [2021] Luis Gaspar (2018274645), Telmo Cunha (2018308321).");
				
				exit(1);
			}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                              										MENU PRINCIPAL
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
			/*Assim que o Profissional de Saúde faz login, o programa cria imediatamente um fork. No processo filho (div),
			efetua as operações normais de menu, crimes, registo, etc. O processo pai, vai ficar em constante "escuta" pelo
			código de operação de emergência enviado pelo Profissional de Saúde, e assim que o recebe, termina o processo
			filho e o programa fica inteiramente focado na parte do botão de emergência*/
			int div = fork();
	
			if(div == 0){
				/*Após o login foi necessário efetuar uma segunda ligação TCP ao servidor na Aplicação Central pois entramos
				num	processo diferente e a ligação do anterior tinha interrompido*/
				
				//Inicio da definição do servidor
				char endServer[100] = "127.0.0.1";
				int fd, nread;
				//int client;
				struct sockaddr_in addr;
				//client_addr;
				//int client_addr_size;
				struct hostent *hostPtr;

				if ((hostPtr = gethostbyname(endServer)) == 0) 
				{
					printf("Couldn t get host address.\n");
					exit(-1);
				}

				bzero((void *) &addr, sizeof(addr));
				addr.sin_family = AF_INET;
				addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
				addr.sin_port = htons(8000);

				if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
				erro("socket");
				if( connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
				erro("Connect");
				//Fim da definição do servidor
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
					printf("\n\n1 - Ver crimes\n");
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
//                               									OPÇAO 1 - VER CRIMES
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
				if(menu2 == 1){
					write(fd, "7", sizeof("7"));
					fflush(stdout);
					
					char crimes[1000][300] = {};
					
					//Variáveis para ajudar com a filtragem/assistência de crimes
					int num_crimes = 0;
					int crime_id;
					char filter_data[50] = "";
					char filter_local[110] = "";
					char filter_nome[50] = "";
					
					//Ciclo para ler os crimes que o servidor envia para a tabela, 1 a 1, até receber "Done", que significa que não há mais
					while(1){
						nread=read(fd, crimes[num_crimes], BUF_SIZE-1);
						fflush(stdin);
						
						if(strcmp(crimes[num_crimes], "Done") == 0){
							break;
						}
						crimes[num_crimes][nread] = '\0';
						num_crimes++; //Contador do número de crimes
					}
					
					/*Este ciclo serve para "embelezar" a apresentação dos crimes, ao separar a string recebida nas partes devidas
					e apresentar o crime corretamente formatado no menu do terminal*/
					while(1){
						const char separador[2] = ";";
						char *data_tok, *hora_tok, *local_tok, *tipo_tok, *nome_tok; //Tokens para os campos do crime todos
						char aux_tok[250] = "";
						while(1){
							menucrimes = 0;
							system("clear");
							header();
							printf("\n\nOs crimes registados são: \n\n");
							for(int i = 0; i < num_crimes; i++){
								strcpy(aux_tok, crimes[i]); //Copiar a string para um auxiliar para usar com o strtok
								data_tok = strtok(aux_tok, separador);
								hora_tok = strtok(NULL, separador);
								local_tok = strtok(NULL, separador);
								tipo_tok = strtok(NULL, separador);
								nome_tok = strtok(NULL, separador);
								
								//Apresentação dos crimes no menu no formato correto
								printf("ID - %d\nData - %s\nHora - %s\nLocal - %s\nTipo - %s\nVitima - %s\n", i+1, data_tok, hora_tok, local_tok, tipo_tok, nome_tok);
								printf("===============\n");
								fflush(stdout);
							}
							
							printf("\n1 - Marcar como assistido\n");
							printf("2 - Filtrar por data\n");
							printf("3 - Filtrar por local\n");
							printf("4 - Filtrar por nome\n");
							printf("5 - Voltar ao menu anterior\n\n");
							printf("Insira a sua escolha: ");
							scanf("%d", &menucrimes);
							fflush(stdin);
							
							while(getchar() != '\n');
							
							if(menucrimes > 0 && menucrimes < 6){
								break;
							}
						}
						
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                               						 OPÇAO 1.1 - MARCAR COMO ASSISTIDO
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
						if(menucrimes == 1){
							/*Caso o Agente de Segurança queira marcar o crime como assistido, envia "ASSIST" ao servidor para entrar
							no ciclo correto*/
							write(fd, "ASSIST", sizeof("ASSIST"));
							fflush(stdout);
							
							//Pede o ID do crime, que é o indice na tabela essencialmente, e envia ao servidor
							while(1){
								printf("\nInsira o ID do crime a assitir: ");
								scanf("%d", &crime_id);
								fflush(stdin);
								
								while(getchar() != '\n');
								
								if(crime_id > 0 && crime_id <= num_crimes){
									break;
								}
							}
							
							//Como a matriz vai de 0 a x, mas a apresentação vai de 1 a x+1, temos que diminuir uma unidade ao ID
							crime_id--;
							write(fd, crimes[crime_id], strlen(crimes[crime_id]));
							fflush(stdout);
							
							//Mensagem de confirmação e volta para o menu principal
							printf("\nCrime marcada como resolvido!\n\nBom trabalho Agente %s\n", user.username);
							printf("A voltar ao menu principal...");
							fflush(stdout);
							sleep(2);
							goto mainmenu;
						}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                       							         OPÇAO 1.2 - FILTRAR POR DATA
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
						if(menucrimes == 2){
							/*Caso o Agente de Segurança apenas queira filtrar os crimes, envia "NOASSIST" ao servidor para este
							não entrar nessa opção*/
							write(fd, "NOASSIST", sizeof("NOASSIST"));
							fflush(stdout);
							
							system("clear");
							header();
							
							int dia, mes, ano;
							
							//Ciclo para pedir um dia entre 1 e 31
							while(1){
								printf("\n\nInsira o dia (1-31): ");
								scanf("%d", &dia);
								fflush(stdin);
								
								while(getchar() != '\n');
								
								if(dia > 0 && dia < 32){
									break;
								}
							}
							
							//Ciclo para pedir um mês entre 1 e 12
							while(1){
								printf("\nInsira o mês (1-12): ");
								scanf("%d", &mes);
								fflush(stdin);
								
								while(getchar() != '\n');
								
								if(mes > 0 && mes < 13){
									break;
								}
							}
							
							//Ciclo para pedir um ano entre 2000 e 2150
							while(1){
								printf("\nInsira o ano (2000-2150): ");
								scanf("%d", &ano);
								fflush(stdin);
								
								while(getchar() != '\n');
								
								if(ano > 1999 && ano < 2151){
									break;
								}
							}
							
							char aux_data[52];
							//Formata a data corretamente: DD/MM/AAAA
							sprintf(filter_data, "%02d-%02d-%d", dia, mes, ano);
							sprintf(aux_data, "\"%s""\"", filter_data); //Adiciona os caracteres corretos à data (aspas) para ajudar na comparação
							
							printf("\n\nCrimes encontrados:\n");
							int encontrou = 0;
							//Este ciclo percorre a lista de todos os crimes e verifica com os filtros aqueles que se adequam
							for(int i = 0; i < num_crimes; i++){
								char separador[2] = ";"; //Delimitador
								char aux_tok[250] = "";
								char *data_tok, *hora_tok, *local_tok, *tipo_tok, *nome_tok;
								
								strcpy(aux_tok, crimes[i]); //Copiar a string para um auxiliar para usar o strtok
								data_tok = strtok(aux_tok, separador);
								hora_tok = strtok(NULL, separador);
								local_tok = strtok(NULL, separador);
								tipo_tok = strtok(NULL, separador);
								nome_tok = strtok(NULL, separador);
								
								//Caso a data do crime coincida com a data escolhida, apresenta esse crime
								if(strcmp(data_tok, aux_data) == 0){
									encontrou = 1;
									printf("ID - %d\nData - %s\nHora - %s\nLocal - %s\nTipo - %s\nVitima - %s\n", i+1, data_tok, hora_tok, local_tok, tipo_tok, nome_tok);
									printf("===============\n");
									fflush(stdout);
								}
							}
							
							if(encontrou == 0){
								printf("\nNenhum crime encontrado com este filtro.\n");
								fflush(stdout);
							}
							
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
//                               							 OPÇAO 1.3 - FILTRAR POR LOCAL
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
						if(menucrimes == 3){
							/*Caso o Agente de Segurança apenas queira filtrar os crimes, envia "NOASSIST" ao servidor para este
							não entrar nessa opção*/
							write(fd, "NOASSIST", sizeof("NOASSIST"));
							fflush(stdout);
							
							system("clear");
							header();
							
							//Ciclo para pedir o local, com fgets para permitir locais com espaços
							while(1){
								printf("\n\nInsira o local a filtrar (máx 100 char): ");
								fgets(filter_local, sizeof(filter_local), stdin);
								fflush(stdin);
								
								while(getchar() != '\n');
								
								filter_local[strcspn(filter_local, "\n")] = 0; //Utilização do strcspn para evitar buffer do '\n' para o input seguinte
								
								if(strlen(filter_local) > 2 && strlen(filter_local) < 100){
									break;
								}
								
								//Caso o tamanho da string seja demasiado grande, informa o cliente e repete o ciclo
								if(strlen(filter_local) > 100){
										printf("\nLocal comprido demais, tente novamente!");
										fflush(stdout);
								}
							}
							
							char aux_local[112] = "";
							sprintf(aux_local, "\"%s""\"", filter_local);
							
							printf("\nCrimes encontrados:\n");
							int encontrou = 0;
							//Este ciclo percorre a lista de todos os crimes e verifica com os filtros aqueles que se adequam
							for(int i = 0; i < num_crimes; i++){
								char separador[2] = ";"; //Delimitador
								char aux_tok[250] = "";
								char *data_tok, *hora_tok, *local_tok, *tipo_tok, *nome_tok;
								
								strcpy(aux_tok, crimes[i]); //Copiar a string para um auxiliar para usar o strtok
								data_tok = strtok(aux_tok, separador);
								hora_tok = strtok(NULL, separador);
								local_tok = strtok(NULL, separador);
								tipo_tok = strtok(NULL, separador);
								nome_tok = strtok(NULL, separador);

								//Caso o local do crime coincida com o local escolhido, apresenta esse crime
								if(strcmp(local_tok, aux_local) == 0){
									encontrou = 1;
									printf("ID - %d\nData - %s\nHora - %s\nLocal - %s\nTipo - %s\nVitima - %s\n", i+1, data_tok, hora_tok, local_tok, tipo_tok, nome_tok);
									printf("===============\n");
									fflush(stdout);
								}
							}
							
							if(encontrou == 0){
								printf("\nNenhum crime encontrado com este filtro.\n");
								fflush(stdout);
							}
							
							returnmenu = 'a';
							//Confirmação para sair para o menu anterior
							while(returnmenu != 's' && returnmenu != 'S'){
								printf("\n\nPressione S para sair: ");
								scanf(" %c", &returnmenu);
								fflush(stdin);
							}
							goto mainmenu;
						}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         								       OPÇAO 1.4 - FILTRAR POR NOME
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
						if(menucrimes == 4){
							/*Caso o Agente de Segurança apenas queira filtrar os crimes, envia "NOASSIST" ao servidor para este
							não entrar nessa opção*/
							write(fd, "NOASSIST", sizeof("NOASSIST"));
							fflush(stdout);
							
							system("clear");
							header();

							//Ciclo para pedir o nome, com fgets para permitir nomes com espaços
							while(1){
								strcpy(filter_nome, "");
								printf("\n\nInsira o nome a filtrar (máx 40 char): ");
								fgets(filter_nome, sizeof(filter_nome), stdin);
								fflush(stdin);
								
								while(getchar() != '\n');
								
								filter_nome[strcspn(filter_nome, "\n")] = 0; //Utilização do strcspn para evitar buffer do '\n' para o input seguinte
								
								if(strlen(filter_nome) > 2 && strlen(filter_nome) < 40){
									break;
								}
								
								//Caso o tamanho da string seja demasiado grande, informa o cliente e repete o ciclo
								if(strlen(filter_nome) > 40){
										printf("\nNome comprido demais, tente novamente!");
										fflush(stdout);
								}
							}
							
							char aux_nome[52] = "";
							sprintf(aux_nome, "\"%s""\"", filter_nome);
							
							printf("\nCrimes encontrados:\n");
							int encontrou = 0;
							//Este ciclo percorre a lista de todos os crimes e verifica com os filtros aqueles que se adequam
							for(int i = 0; i < num_crimes; i++){
								char separador[2] = ";"; //Delimitador
								char aux_tok[250] = "";
								char *data_tok, *hora_tok, *local_tok, *tipo_tok, *nome_tok;
								
								strcpy(aux_tok, crimes[i]); //Copiar a string para um auxiliar para usar o strtok
								data_tok = strtok(aux_tok, separador);
								hora_tok = strtok(NULL, separador);
								local_tok = strtok(NULL, separador);
								tipo_tok = strtok(NULL, separador);
								nome_tok = strtok(NULL, separador);
								
								//Caso o local do crime coincida com o local escolhido, apresenta esse crime
								if(strcmp(nome_tok, aux_nome) == 0){
									encontrou = 1;
									printf("ID - %d\nData - %s\nHora - %s\nLocal - %s\nTipo - %s\nVitima - %s\n", i+1, data_tok, hora_tok, local_tok, tipo_tok, nome_tok);
									printf("===============\n");
									fflush(stdout);
								}
							}
							
							if(encontrou == 0){
								printf("\nNenhum crime encontrado com este filtro.\n");
								fflush(stdout);
							}
							
							returnmenu = 'a';
							//Confirmação para sair para o menu anterior
							while(returnmenu != 's' && returnmenu != 'S'){
								printf("\n\nPressione S para sair: ");
								scanf(" %c", &returnmenu);
								fflush(stdin);
							}
							goto mainmenu;
						}
					
						/*No caso de sair do menu dos crimes, significa que não vai marcar nenhum como assistido, logo
						envia ao servidor "NOASSIST" para este sair do ciclo de espera*/
						if(menucrimes == 5){
							write(fd, "NOASSIST", sizeof("NOASSIST"));
							fflush(stdout);
							goto mainmenu;
						}
					}
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
						write(fd, "8", sizeof("8"));
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
						write(fd, registo, strlen(registo));
						fflush(stdout);
						
						printf("\nRegisto alterado com sucesso!\n");
						
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
								write(fd, "9", sizeof("9"));
								fflush(stdout);
								sleep(0.5);
								
								//Formata o registo com os dados do utilizador para facilitar a eliminação no servidor
								sprintf(registo, "%s\\%s\\%s", user.id, user.username, user.password);
								write(fd, registo, strlen(registo));
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
					/*Como é um servidor TCP, o cliente e o servidor estão ligados. Para evitar interrupções abruptas e lixo na porta,
					enviamos o código de operação 3 ao servidor, que faz com que esse feche a ligação*/
					//Limpa as streams de dados stdin e stdout, fecha a ligação, apresenta a mensagem de copyright e termina o programa
					write(fd, "3", sizeof("3"));
					fflush(stdout);
					fflush(stdin);
					
					printf("\nCopyright (c) [2021] Luis Gaspar (2018274645), Telmo Cunha (2018308321).");
					
					close(fd);
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
					printf("\nA entrar em contacto com um Profissional de Saúde...");
					fflush(stdout);
					
					//Inicio da definição do servidor
					char endServer[100] = "127.0.0.1";
					int fd_chat, nread;
					//int client;
					struct sockaddr_in addr;
					//client_addr;
					//int client_addr_size;
					struct hostent *hostPtr;

					if ((hostPtr = gethostbyname(endServer)) == 0) 
					{
						printf("Couldn t get host address.\n");
						exit(-1);
					}

					bzero((void *) &addr, sizeof(addr));
					addr.sin_family = AF_INET;
					addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
					addr.sin_port = htons(8700);

					if((fd_chat = socket(AF_INET,SOCK_STREAM,0)) == -1)
					erro("socket");
					if( connect(fd_chat,(struct sockaddr *)&addr,sizeof (addr)) < 0)
					erro("Connect");
					//Fim da definição do servidor
					
					char cliente[50] = "";
					
					/*Para o Profissional de Saúde saber com quem está em contacto, o primeiro dado a enviar é o nome
					do Agente de Segurança*/
					write(fd_chat, user.username, strlen(user.username));
					fflush(stdout);
					
					/*Com o mesmo efeito, para o Profissional de Saúde com quem está em contacto, é recebido de seguida o
					o nome do Agente de Segurança*/
					nread = read(fd_chat, cliente, 50);
					fflush(stdin);
					cliente[nread] = '\0';
					
					printf("\nProfissional de Saúde encontrado! Ligação estabelecida.\n");
					printf("\nEstá em contacto com o Profissional de Saúde %s!\n", cliente);
					fflush(stdout);
				
					char enviado[250] = "";
					char recebido[250] = "";
					
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
								printf("\n\nServidor desligado de repente.\n");
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
							
							/*Apresenta mensagem recebida com remetente (o Profissional de Saúde neste caso)
							e apresenta prompt do outro processo para efeitos de apresentação mais limpa*/
							printf("\33[2K\rProfissional de Saúde %s: %s\n", cliente, recebido);
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
							
							//Apresenta a mensagem enviada com o remetente (o utilizador neste caso)
							printf("O utilizador: %s\n", enviado);
							fflush(stdout);
						}
						fflush(stdout);
					}
					//Espera pelo término do processo filho antes de continuar para a saida do chat
					wait(NULL);
					
					//Limpa as streams de entrada e saida
					fflush(stdin);
					fflush(stdout);
					
					//Apresenta mensagem de ligação terminada, e volta ao menu principal
					printf("\nLigação terminada, a voltar ao menu.\n");
					fflush(stdout);
					sleep(2);
					goto mainmenu;
					/*Não se usa exit(0) neste processo pois como é o processo pai, é o mesmo do resto do programa e não
					se pode fechar*/
				}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                             										OPÇAO 5 - HELP
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
				if(menu2 == 5){
					FILE *fhelp;
					
					//Aqui abre o ficheiro que contém as informações de Ajuda neste cliente					
					fhelp = fopen("HELP Files/HELP_AAS.txt", "r");
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
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         								     FORK BOTÃO DE EMERGENCIA
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
			else{
				//Inicio da definição do servidor
				int udpSocket;
				int nBytes;
				//, nSize;

				struct sockaddr_in serverAddr;
				//clientAddr;
				struct sockaddr_storage serverStorage;

				socklen_t addr_size;
				//client_addr_size;	

				udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

				serverAddr.sin_family = AF_INET;
				serverAddr.sin_port = htons(9500);
				serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
				memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

				bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

				addr_size = sizeof serverStorage;
				//Fim da definição do servidor
				
				char emergencia[1];
				
				//Dentro deste processo está sempre à espera de receber o código de operação 6 para iniciar o procedimento
				nBytes = recvfrom(udpSocket, emergencia, 2, 0, (struct sockaddr *)&serverStorage, &addr_size);
				fflush(stdin);
				
				if(emergencia[0] == '6'){
					/*Assim que recebe o código de emergência, usa kill para terminar automaticamente o processo do programa normal
					Isto serve para adicionar alguma sincronização de processos e evitar processos zombie, e presume que o Agente
					vá de imediato ao socorro da vitima e não precise de utilizar mais a aplicação.*/
					kill(div, SIGKILL);
					
					system("clear");
					header();
					while(1){
						char help;
						char dados_emergencia[250] = "";
						
						//Recebe os dados de emergência para a string
						nBytes = recvfrom(udpSocket, dados_emergencia, 250, 0, (struct sockaddr *)&serverStorage, &addr_size);
						fflush(stdin);
						
						printf("\n\n*************************************\n*****\t ALERTA EMERGÊNCIA\t*****\n*************************************\n\n");
						
						const char separador[2] = "\\"; //Delimitador
						char *data_tok;
						char *hora_tok;
						char *local_tok;
						
						//Uso de strtok várias vezes para separar os dados da emergência
						data_tok = strtok(dados_emergencia, separador);
						hora_tok = strtok(NULL, separador);
						local_tok = strtok(NULL, separador);
						
						//Apresentação dos dados de emergência: Data, Hora, e Local
						printf("Data: %s\nHora: %s\n", data_tok, hora_tok);
						printf("Local: %s\n", local_tok);
						
						help = 'a';
						//Ciclo para confirmação de assistência
						while(1){
							printf("\nPressione S para confimar a assistência: ");
							scanf(" %c", &help);
							fflush(stdin);
							if(help == 's' || help == 'S'){
								/*Assim que o Agente de Segurança confirmar a assistência, envia ao Profissional de Saúde
								a string "CONF", e sai do programa geral com exit pois este é o processo pai*/
								nBytes = strlen("CONF") + 1;
								sendto(udpSocket, "CONF", nBytes, 0, (struct sockaddr *)&serverStorage, addr_size);
								fflush(stdout);
								printf("\nProfissional de Saúde avisado, boa sorte agente!\n\n");
								printf("A terminar programa...\n");
								
								printf("\nCopyright (c) [2021] Luis Gaspar (2018274645), Telmo Cunha (2018308321).");
								exit(1);
							}
						}
					}
				}
			} 
		}
	return 0;
}
//Esta função serve apenas para apresentar erros durante a ligação TCP
void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}
//Esta função serve apenas para efeitos de composição gráfica, em que escreve o banner no topo do ecrã
void header(void){
	printf("===============AAS - Aplicação para o Agente de Segurança===============");
	return;
}
