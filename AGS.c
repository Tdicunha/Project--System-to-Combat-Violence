//APLICAÇÃO PARA GESTOR DE SISTEMA (AGS)

//SOCKET USADO: TPC (WRITE, READ)

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define BUF_SIZE 5000

//Definição de uma struct que contém todos os parâmetros presentes num registo
typedef struct Utilizador{
	char id[10];
	char username[100];
	char password[100];
} Utilizador;

//Definição das funções necessárias para o header, apresentação de erros, e chat de texto
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
	addr.sin_port = htons(9000);

	if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	erro("socket");
	if( connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
	erro("Connect");
	//Fim da definição do servidor	

	//Variáveis de navegação no menu
	int menu1;
	int menu2;
	char returnmenu;
	
	//Variáveis de credenciais e dados
	Utilizador user;
	char registo[250] = "";
	char aps_manage[250][250] = {};
	char aas_manage[250][250] = {};
	char id_accept[100] = "";
	
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
						printf("\n\nInsira o seu ID de profissional (AGS_xxxx): ");
						scanf("%s", user.id);
						fflush(stdin);
						
						if(strlen(user.id) != 8){
							printf("\nID inserido tem um número de caracteres incorreto, tente novamente!");
						}
					}while(strlen(user.id) != 8); //Só sai deste ciclo quando o tamanho do ID for igual a 8
					
					if(strncmp(user.id, "AGS_", 4) != 0){
						printf("\nID inserido não corresponde ao exemplo, tente novamente!\n");
					}
				}while(strncmp(user.id, "AGS_", 4) != 0); //Só sai deste ciclo quando o ID começar com "AGS_"
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
				nread=read(fd, confirm, BUF_SIZE-1); //ler os crimes
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
					printf("\n\nInsira o seu ID de profissional (AGS_xxxx): ");
					scanf("%s", user.id);
					fflush(stdin);
					while(getchar() != '\n');
					
					if(strlen(user.id) != 8){
						printf("\nID inserido tem um número de caracteres incorreto, tente novamente!");
					}
				}while(strlen(user.id)!=8); //Só sai deste ciclo quando o tamanho do ID for igual a 8
				
				if(strncmp(user.id,"AGS_",4)!=0){
					printf("ID inserido não corresponde ao exemplo, tente novamente!\n");
					fflush(stdout);
				}
			}while(strncmp(user.id,"AGS_",4)!=0); //Só sai deste ciclo quando o ID começar com "AGS_"
			
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
					printf("\nUsername comprido demais, tente novamente!\n.");
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
				
				if(strlen(user.password)>50){
					printf("\nPassword comprida demais, tente novamente!\n");
					fflush(stdout);
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
			//Limpa as streams de dados stdin e stdout, fecha a ligação, apresenta a mensagem de copyright e termina o programa
			write(fd, "3", sizeof("3"));
			fflush(stdout);
			
			close(fd);
			
			printf("\nCopyright (c) [2021] Luis Gaspar (2018274645), Telmo Cunha (2018308321).");
			
			exit(1);
		}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                              										MENU PRINCIPAL
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
		//Menu principal após o login
		mainmenu:
		//Ciclo while em que só sai com os inputs corretos do utilizador
		while(1){
			menu2 = 0;
			system("clear");
			header();
			printf("\n\nSeja bem vindo, %s", user.username);
			printf("\n\n1 - Autorizar registos\n");
			printf("2 - Editar registos\n");
			printf("3 - Apagar registos\n");
			printf("4 - Sair da aplicação\n");
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
//                              						OPÇAO 1 - AUTORIZAR NOVOS REGISTOS
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
		if(menu2 == 1){
			write(fd, "7", sizeof("7"));
			fflush(stdout);
			
			id_accept[0] = 'a';
			int newaps_count = 0;
			int newaas_count = 0;
			
			system("clear");
			header();
			printf("\n\nLista de novos registos de saúde por aceitar: \n\n");
			
			/*Neste ciclo vai receber os registos de Saúde por aceitar, 1 a 1, e guardar na tabela correspondente,
			bem como contar a quantidade de registos que recebe*/
			while(1){
				nread=read(fd, aps_manage[newaps_count], BUF_SIZE-1);
				fflush(stdin);
				//Se receber a string "Done" enviada pelo servidor, significa que a lista acabou, e sai do ciclo
				if(strcmp(aps_manage[newaps_count], "Done") == 0){
					break;
				}
				//Se receber a string "Done" enviada pelo servidor, significa que a lista acabou, e sai do ciclo
				aps_manage[newaps_count][nread] = '\0';
				printf("%d - %s\n", newaps_count+1, aps_manage[newaps_count]);
				newaps_count++;
			}
			
			printf("\nLista de novos registos de segurança por aceitar: \n\n");
			
			/*Neste ciclo vai receber os registos de Segurança por aceitar, 1 a 1, e guardar na tabela correspondente,
			bem como contar a quantidade de registos que recebe*/
			while(1){
				nread = read (fd, aas_manage[newaas_count], BUF_SIZE-1);
				fflush(stdin);
				//Se receber a string "Done" enviada pelo servidor, significa que a lista acabou, e sai do ciclo
				if(strcmp(aas_manage[newaas_count], "Done") == 0){
					break;
				}
				aas_manage[newaas_count][nread] = '\0';
				printf("%d - %s\n", newaas_count+1, aas_manage[newaas_count]);
				newaas_count++;
			}
			
			//Pede ao Gestor um primeiro ID para aceitar
			printf("\nSelecione um ID para aceitar (S para sair): ");
			fflush(stdout);
			scanf("%s", id_accept);
			fflush(stdin);
			
			//Continua a pedir ID's ao Gestor até receber 'S' para sair e envia ao servidor
			while(id_accept[0] != 's' && id_accept[0] != 'S'){
				write(fd, id_accept, sizeof(id_accept));
				fflush(stdout);
				printf("\nInsira outro ID para aceitar (S para sair): ");
				scanf("%s", id_accept);
				fflush(stdin);
			}
			write(fd, id_accept, sizeof(id_accept));
			fflush(stdout);
			goto mainmenu;
		}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//                              								OPÇAO 2 - EDITAR REGISTOS
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
		if(menu2 == 2){
			write(fd, "8", sizeof("8"));
			fflush(stdout);
			
			id_accept[0] = 'a';
			int editaps_count = 0;
			int editaas_count = 0;
			
			system("clear");
			header();
			printf("\n\nLista de registos de saúde existentes: \n\n");
			
			/*Neste ciclo vai receber os registos de Saúde existentes, 1 a 1, e guardar na tabela correspondente,
			bem como contar a quantidade de registos que recebe*/
			while(1){
				nread = read(fd, aps_manage[editaps_count], BUF_SIZE-1);
				fflush(stdin);
				//Se receber a string "Done" enviada pelo servidor, significa que a lista acabou, e sai do ciclo
				if(strcmp(aps_manage[editaps_count], "Done") == 0){
					break;
				}
				aps_manage[editaps_count][nread] = '\0';
				printf("%d - %s\n", editaps_count+1, aps_manage[editaps_count]);
				editaps_count++;
			}
			
			/*Neste ciclo vai receber os registos de Segurança existentes, 1 a 1, e guardar na tabela correspondente,
			bem como contar a quantidade de registos que recebe*/
			printf("\nLista de registos de segurança existentes: \n\n");
			while(1){
				nread = read(fd, aas_manage[editaas_count], BUF_SIZE-1);
				fflush(stdin);
				//Se receber a string "Done" enviada pelo servidor, significa que a lista acabou, e sai do ciclo
				if(strcmp(aas_manage[editaas_count], "Done") == 0){
					break;
				}
				aas_manage[editaas_count][nread] = '\0';
				printf("%d - %s\n", editaas_count+1, aas_manage[editaas_count]);
				editaas_count++;
			}
					
			const char separador[2] = "\\"; //Delimitador
			char *id_tok, *user_tok, *pw_tok;
			char aux[250] = "";
			
			while(1){
				int id_correto = 0;
				//Pede ao Gestor o ID do registo a editar
				printf("\nSelecione o ID de um utilizador para editar (S para sair): ");
				scanf("%s", id_accept);
				fflush(stdin);
				
				while(getchar() != '\n');
				
				/*Este ciclo serve para percorrer a lista de registos de Saúde e obter todos os parâmetros do registo separadamente,
				e so entra neste ciclo se o ID escolhido for do tipo APS_xxxx*/
				if(strncmp(id_accept, "APS", 3) == 0){
					for(int i = 0; i < editaps_count; i++){
						if(strncmp(aps_manage[i], id_accept, 8) == 0){
							id_correto = 1;
							strcpy(aux, aps_manage[i]); //Copiar o registo para uma string auxiliar para se usar o strtok
							id_tok = strtok(aux, separador);
							user_tok = strtok(NULL, separador);
							pw_tok = strtok(NULL, separador);
							break;
						}
					}
				}
				
				/*Este ciclo serve para percorrer a lista de registos de Saúde e obter todos os parâmetros do registo separadamente,
				e so entra neste ciclo se o ID escolhido for do tipo AAS_xxxx*/
				if(strncmp(id_accept, "AAS", 3) == 0){
					for(int i = 0; i < editaas_count; i++){
						if(strncmp(aas_manage[i], id_accept, 8) == 0){
							id_correto = 1;
							strcpy(aux, aas_manage[i]); //Copiar o registo para uma string auxiliar para se usar o strtok
							id_tok = strtok(aux, separador);
							user_tok = strtok(NULL, separador);
							pw_tok = strtok(NULL, separador);
							break;
						}
					}
				}
				
				if(id_correto == 1){
					break;
				}
				
				printf("\nID errado, tente novamente!");
				fflush(stdout);
			}
			
			//Nova instância da struct Utilizador para facilitar a organização
			Utilizador edit;
			system("clear");
			header();
			
			//Apresenta os dados atuais do utilizador a editar
			printf("\n\nEdição de registo\n\n");
			printf("Dados atuais:\nID - %s\nUsername - %s\nPassword - %s\n\n", id_tok, user_tok, pw_tok);
			fflush(stdout);
			
			//Semelhante à funcionalidade de Registo, vai pedir o novo username, ou 0 para manter o atual
			while(1){
				printf("Insira o novo username do utilizador (0 para manter, máx 100 char): ");
				fgets(edit.username, sizeof(edit.username), stdin);
				fflush(stdin);
				
				edit.username[strcspn(edit.username, "\n")] = 0; //Utilização do strcspn para evitar buffer do '\n' para o input seguinte
				
				if(edit.username[0] == '0'){
					strcpy(edit.username, user_tok); //Caso o Gestor não mude o parâmetro, copia o atual para o novo destino
				}
				if(strlen(edit.username) > 0 && strlen(edit.username) <= 100){
					break;
				}
			}
			
			//Semelhante à funcionalidade de Registo, vai pedir a nova password, ou 0 para manter a atual
			while(1){
				printf("Insira a nova password do utilizador (0 para manter, máx 100 char): ");
				scanf("%s", edit.password);
				fflush(stdin);
				
				if(edit.password[0] == '0'){
					strcpy(edit.password, pw_tok); //Caso o Gestor não mude o parâmetro, copia o atual para o novo destino
				}
				if(strlen(edit.password) > 0 && strlen(edit.password) <= 100){
					break;
				}
			}
			
			//Junta os 3 parâmetros no formato correto e envia ao servidor
			sprintf(registo, "%s\\%s\\%s", id_tok, edit.username, edit.password);
			write(fd, registo, strlen(registo));
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
//                               								OPÇAO 3 - APAGAR REGISTOS
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
		if(menu2 == 3){
			write(fd, "9", sizeof("9"));
			fflush(stdout);
			
			id_accept[0] = 'a';
			int delaps_count = 0;
			int delaas_count = 0;
			
			system("clear");
			header();
			printf("\n\nLista de registos de saúde existentes: \n\n");
			
			/*Neste ciclo vai receber os registos de Saúde existentes, 1 a 1, e guardar na tabela correspondente,
			bem como contar a quantidade de registos que recebe*/
			while(1){
				nread = read(fd, aps_manage[delaps_count], BUF_SIZE-1);
				fflush(stdin);
				//Se receber a string "Done" enviada pelo servidor, significa que a lista acabou, e sai do ciclo
				if(strcmp(aps_manage[delaps_count], "Done") == 0){
					break;
				}
				aps_manage[delaps_count][nread] = '\0';
				printf("%d - %s\n", delaps_count+1, aps_manage[delaps_count]);
				delaps_count++;
			}
			
			/*Neste ciclo vai receber os registos de Segurança existentes, 1 a 1, e guardar na tabela correspondente,
			bem como contar a quantidade de registos que recebe*/
			printf("\nLista de registos de segurança existentes: \n\n");
			while(1){
				nread = read(fd, aas_manage[delaas_count], BUF_SIZE-1);
				fflush(stdin);
				//Se receber a string "Done" enviada pelo servidor, significa que a lista acabou, e sai do ciclo
				if(strcmp(aas_manage[delaas_count], "Done") == 0){
					break;
				}
				aas_manage[delaas_count][nread] = '\0';
				printf("%d - %s\n", delaas_count+1, aas_manage[delaas_count]);
				delaas_count++;
			}
			
			//Pede ao Gestor o ID do registo a apagar e envia ao servidor
			printf("\nSelecione um ID para apagar (S para sair): ");
			scanf("%s", id_accept);
			fflush(stdin);
			
			while(getchar() != '\n');
			
			write(fd, id_accept, strlen(id_accept));
			fflush(stdout);
			
			printf("Registo apagado com sucesso!\n");
			
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
//                                								OPÇAO 4 - SAIR DO PROGRAMA
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
		if(menu2 == 4){
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
//                             											OPÇAO 5 - HELP
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
		if(menu2 == 5){
			FILE *fhelp;
			
			//Aqui abre o ficheiro que contém as informações de Ajuda neste cliente
			fhelp = fopen("HELP Files/HELP_AGS.txt", "r");
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
//Esta função serve apenas para apresentar erros durante a ligação TCP
void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}
//Esta função serve apenas para efeitos de composição gráfica, em que escreve o banner no topo do ecrã
void header(void){
	printf("===============AGS - Aplicação de Gestor de Sistema===============");
	return;
}
