#include <stdio.h>	//printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

#define _GNU_SOURCE
#define BUFLEN 512 // Tamanho do buffer
#define PORT 10001 // porta
#define PASSWORD "batata123*"

FILE *logged_users_file;
FILE *insert_text_file;

void die(char *s)
{
	perror(s);
	exit(1);
}

char *try_login(char *user)
{
	// Recupera a senha
	char *password = strtok(NULL, " ");
	// Inicializa a mensagem de retorno
	char *return_message;
	password[strcspn(password, "\n")] = 0;

	// Compara password inserida com password do sistema. Se for, autentica
	if (strncmp(password, PASSWORD, strlen(password)) == 0)
	{
		printf("\nUsuário %s autenticado.\n", user);
		// Abre o arquivo 
		logged_users_file = fopen("logged_users.txt", "a");
		// Insere a linha
		fprintf(logged_users_file, "%s\n", user);
		// Determina mensagem de sucesso
		asprintf(&return_message, "Autenticado com sucesso");
		// Fecha a conexão
		fclose(logged_users_file);
	}
	// Se não, retorna erro
	else
	{
		asprintf(&return_message, "Falha na autenticação");
	}
	return return_message;
}

bool register_user()
{
	// Verificar se o usuário está logado
	return false;
}

// Verifica se o usuário está autenticado
bool is_user_logged_in(char *user)
{
	// Abre o arquivo 
	logged_users_file = fopen("logged_users.txt", "r");
	bool result = false;
	char *line;
	size_t len = 0;
	ssize_t read;
	// until the last character of file is obtained
	while (((read = getline(&line, &len, logged_users_file)) != -1) && !result)
	{
		line[strcspn(line, "\n")] = 0;
		result = strncmp(line, user, strlen(line)) == 0;
	}
	// Fecha a conexão
	fclose(logged_users_file);
	return result;
}

int main(void)
{
	struct sockaddr_in si_me, si_other;
	int s, i, slen = sizeof(si_other), recv_len;
	char buf[BUFLEN];

	logged_users_file = fopen("logged_users.txt", "w");
	fclose(logged_users_file);
	insert_text_file = fopen("arquivoTexto.txt", "w");
	fclose(insert_text_file);

	// create a UDP socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		die("socket");
	}

	memset((char *)&si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me)) == -1)
	{
		die("bind");
	}

	while (1)
	{
		printf("\nEsperando dados do cliente...\n");
		fflush(stdout);
		// Verifica se recebeu dados
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)) == -1)
		{
			die("recvfrom()");
		}
		char *user;
		// Monta o nome do usuário com IP:PORTA
		asprintf(&user, "%s:%d", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		// Verifica se o usuário que enviou dados está conectado
		if (is_user_logged_in(user))
		{
			printf("\nRecebido pacote de %s\n", user);
			printf("Mensagem recebida: %s\n", buf);

			char *value;
			asprintf(&value, "%s -> %s", user, buf);
			// Reabre o arquivo de texto para escrever
			insert_text_file = fopen("arquivoTexto.txt", "a");
			// Insere texto recebido no arquivo
			fprintf(insert_text_file, "%s", value);
			// Fecha o arquivo de texto
			fclose(insert_text_file);
			if (sendto(s, buf, recv_len, 0, (struct sockaddr *)&si_other, slen) == -1)
			{
				die("sendto()");
			}
		}
		// Verifica se o usuário enviou comando de login
		else
		{
			char *return_message;
			// Verifica se foi recebida mensagem
			if (strlen(buf) > 0)
			{
				// Realiza quebra da mensagem pelo separador de comando
				char *token = strtok(buf, ":");
				// Verifica se foi recebido comando de Login
				if (strncmp(token, "l", strlen(token)) == 0)
				{
					asprintf(&return_message, try_login(user));
				}
				else
				{
					asprintf(&return_message, "Comando Invalido");
				}
			}
			else
			{
				asprintf(&return_message, "Mensagem Invalida");
			}
			// Devolve mensagem para o cliente (sucesso ou falha)
			if (sendto(s, return_message, strlen(return_message), 0, (struct sockaddr *)&si_other, slen) == -1)
			{
				die("sendto()");
			}
			free(user);
		}
		memset(buf, '\0', BUFLEN);
	}
	close(s);
	return 0;
}
