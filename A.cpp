#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>


void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext);
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext);
            
int ecb(char *sir, int sir_len, unsigned char * key, int);
int cfb(char *sir, int sir_len, unsigned char * key, unsigned char* iv, int q);

int main(int argc, char* argv[])
{

	int p[2], q[2];
   	unsigned char decryptedtext[128];
	unsigned char *K3 = (unsigned char *)"0123456789012345";
	unsigned char *iv = (unsigned char *)"0123456789101112";

    	int decryptedtext_len;
	//Creare MKFIFO
	
	if(-1 == mkfifo("A_TO_B", 0600) )
	{
		if(errno != EEXIST)   // errno=17 for "File already exists"
			fprintf(stdout,"Nota: canalul fifo exista deja !\n");
		else
			perror("Eroare la crearea canalului fifo. Cauza erorii");  exit(3);
	}
	if(-1 == mkfifo("B_TO_A", 0600) )
	{
		if(errno != EEXIST)   // errno=17 for "File already exists"
			fprintf(stdout,"Nota: canalul fifo exista deja !\n");
		else
			perror("Eroare la crearea canalului fifo. Cauza erorii");  exit(3);
	}
	
	
	if(-1==(q[1] = open("A_TO_KM",O_WRONLY)))
	{
		perror("eroare la open");exit(8);
	}

	char mode[100];
	int mod;
	printf("Intoruceti modul de criptare dorit: ECB/CFB\n");
	scanf("%s", mode);
	int octetiCititi;
	int len_mode = strlen(mode);
	if (strcmp(mode, (char*)"ecb")==0)
		mod = 0;
	else 
		mod = 1;
	printf("SUNT IN MODUL: %d\n", mod);	

	write(q[1], &len_mode, sizeof(int));
	write(q[1], mode, len_mode);
	close(q[1]);

        
	unsigned char buff[50000];
	if (-1==(q[0] = open("KM_TO_A",O_RDONLY)))
 	{
		perror("Eroare la open");exit(8);
	}
	read(q[0], &octetiCititi, sizeof(int));
	read(q[0], buff, octetiCititi);
	close(q[0]);
	buff[octetiCititi]=0;
       
       //Am citit cheia de la KM
	printf("Am citit cheia de la KM:\n");
	printf("%s\n", buff);
	for (int i=0;i<octetiCititi;i++)
		printf("%02x", buff[i]);
		
	if (-1==(q[1] = open("A_TO_B",O_WRONLY)))
 	{
		perror("Eroare la open");exit(8);
	}
	write(q[1], &len_mode, sizeof(int));
	write(q[1], mode, len_mode);
	
	write(q[1], &octetiCititi, sizeof(int));
	write(q[1], buff, octetiCititi);
	close(q[1]);
		
		
		
	decryptedtext_len = decrypt(buff, octetiCititi, K3, NULL, decryptedtext);
	decryptedtext[decryptedtext_len] = '\0';
	printf("\nDecrypted text is:\n");
	printf("%s\n", decryptedtext);
	    

   	if (-1==(q[0] = open("B_TO_A",O_RDONLY)))
 	{
		perror("Eroare la open");exit(8);
	}
	read(q[0], &octetiCititi, sizeof(int));
	read(q[0], buff, octetiCititi);
       
	close(q[0]);
	
	printf("Incepem transmisiunea\n");
	
	
	if (-1==(q[1] = open("A_TO_B",O_WRONLY)))
 	{
		perror("Eroare la open");exit(8);
	}
	
	FILE *fin = fopen("mesajA.txt", "r");
	char sir[1000];
	char ch;
	int poz=0;
	char theMessage[1000000];
	while (fscanf(fin, "%c", &ch)!=EOF){
		theMessage[poz]=ch;
		poz++;
		
	}
	theMessage[poz-1]=0;
	printf("%s", theMessage);
	
	octetiCititi = poz-1;

	if (mod == 0)
		ecb(theMessage, octetiCititi, decryptedtext, q[1]);
	else
		cfb(theMessage, octetiCititi, decryptedtext, iv, q[1]);
		
	
	while (fgets(sir, 1000, stdin))
	{	
		
		if (strcmp(sir, "\n") == 0){
			octetiCititi = strlen(sir);
			write(q[1], &octetiCititi, sizeof(int));
			write(q[1], sir, octetiCititi);
			continue;
		}
		//printf("%s!\n",sir);
		if (strcmp(sir, "Done\n") == 0)
		{
			printf("Oprim procesul...\n");
			break;
		}
		

		octetiCititi = strlen(sir)-1;
		sir[octetiCititi]=0;
		if (mod == 0)
			ecb(sir, octetiCititi, decryptedtext, q[1]);
		else
			cfb(sir, octetiCititi, decryptedtext, iv, q[1]);
		
		//write(q[1], &octetiCititi, sizeof(int));
		//write(q[1], sir, octetiCititi);
	}
	close(q[1]);
	unlink("A_TO_B");
	unlink("B_TO_A");
	
	return 0;
}

int XOR(unsigned char* text1, unsigned char* text2, unsigned char* result)
{
	int i, j, rez, rez1;
	
	for (i=0;i<16;i++)
	{

		rez=0;
		//printf("%d\n", text1[i]);
		//printf("%d\n", text2[i]);
		for (j=7;j>=0;j--)
		{
			
			rez1 = ((int)text2[i] & (1<<j)) ^ ((int)text1[i] & (1<<j));
			rez += rez1;
		}
		result[i] = (unsigned char) rez;
	}
	result[16] = 0;
	return 16;
}



int cfb(char *sir, int sir_len, unsigned char * key, unsigned char* iv, int q)
{
	int i = 0, dec_len, xor_len=0;
	unsigned char block[17];
	unsigned char decrypted_block[1000];
	unsigned char xor_result[1000];
	block[16]=0;
	int rest = (16 - sir_len % 16) % 16;
	for (i = 0 ;i<rest;i++)
		strcat(sir, (char*)" ");
	for (i = 0; i<sir_len+rest; i++)
	{
		block[i%16] = sir[i];
		if (i%16 == 15)
		{
			printf("%s!\n", (char*)block);
			if (xor_len == 0)
				//dec_len = encrypt (iv, 16, key, NULL, decrypted_block);
				dec_len = XOR(iv, key, decrypted_block);
			else
				//dec_len = encrypt (xor_result, 16, key, NULL, decrypted_block);
				dec_len = XOR(xor_result, key, decrypted_block);
			decrypted_block[dec_len]=0;
			
			printf("LG encrypted: %d\n", dec_len);
			printf("Bloc criptat:\n");
			printf("%s!\n", decrypted_block);
			
			xor_len = XOR(decrypted_block, block, xor_result);
			printf("Rezultat cu XOR:\n");
			printf("%s!\n\n", xor_result);
			
			write(q, &xor_len, sizeof(int));
			write(q, xor_result, xor_len);
		}
	}
	return 0;
}

int ecb(char *sir, int sir_len, unsigned char * key, int q)
{
	int i = 0, dec_len;
	unsigned char block[17];
	unsigned char decrypted_block[1000];
	block[16]=0;
	int rest = 16 - sir_len % 16;
	for (i = 0 ;i<rest;i++)
		strcat(sir, (char*)" ");
	for (i = 0; i<sir_len+rest; i++)
	{
		block[i%16] = sir[i];
		if (i%16 == 15)
		{
			printf("%s!\n", (char*)block);
			//dec_len = encrypt ((unsigned char*)block, 16, key, NULL, decrypted_block);
			dec_len = XOR((unsigned char*)block, key, decrypted_block);
			decrypted_block[dec_len]=0;
			printf("%s\n", decrypted_block);
			write(q, &dec_len, sizeof(int));
			write(q, decrypted_block, dec_len);
			printf("\n");
		}
	}
	
	
	return 0;
}


int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, iv))
        handleErrors();

    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleErrors();
    ciphertext_len = len;

    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        handleErrors();
    ciphertext_len += len;
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}


int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;

    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();


    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, iv))
        handleErrors();

    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleErrors();
    plaintext_len = len;

    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        handleErrors();
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}
