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
int ecb(unsigned char* buff, int octetiCititi, unsigned char* key);
int cfb (int q, unsigned char* key, unsigned char* iv);
int XOR(unsigned char* text1, unsigned char* text2, unsigned char* result);

int main(int argc, char* argv[])
{

	int p[2], q[2];
   	unsigned char decryptedtext[128];
	unsigned char *K3 = (unsigned char *)"0123456789012345";
	unsigned char *iv = (unsigned char *)"0123456789101112";
	int decryptedtext_len;
	
	int octetiCititi;
	unsigned char buff[50000];
	
	if (-1==(q[0] = open("A_TO_B",O_RDONLY)))
 	{
		perror("Eroare la open");exit(8);
	}
	
	read(q[0], &octetiCititi, sizeof(int));
	read(q[0], buff, octetiCititi);
	
	int mod;
	if (strcmp((char*)buff, (char*) "ecb") == 0)
		mod = 0;
	else
		mod = 1;
	printf("SUNT IN MODUL: %d\n", mod);
	
	
	read(q[0], &octetiCititi, sizeof(int));
	read(q[0], buff, octetiCititi);
	close(q[0]);
	buff[octetiCititi]=0;
	
	printf("Am citit cheia de la KM:\n");
	printf("%s\n", buff);
	for (int i=0;i<octetiCititi;i++)
		printf("%02x", buff[i]);
		
	decryptedtext_len = decrypt(buff, octetiCititi, K3, NULL, decryptedtext);
	decryptedtext[decryptedtext_len] = '\0';
	printf("\nDecrypted text is:\n");
	printf("%s\n", decryptedtext);
	
	char mesaj[100] = "Sunt gata!";
	
	octetiCititi=strlen(mesaj);
	if (-1==(q[1] = open("B_TO_A",O_WRONLY)))
 	{
		perror("Eroare la open");exit(8);
	}
	write(q[1], &octetiCititi, sizeof(int));
	write(q[1], mesaj, octetiCititi);
	close(q[1]);
	
	printf("Sunt gata sa primesc!\n");
	
	
	if (-1==(q[0] = open("A_TO_B",O_RDONLY)))
 	{
		perror("Eroare la open");exit(8);
	}
	//unsigned char block_from_A[1000];
	//int dec_len;
	if (mod == 1)
		cfb(q[0], decryptedtext, iv);
	else
		{
			while (read(q[0], &octetiCititi, sizeof(int))!=0)
			{
				read(q[0], buff, octetiCititi);
				if (octetiCititi == 1)
				{
					continue;
				}
				//read(q[0], buff, octetiCititi);
				buff[octetiCititi] = 0;
				printf("%s!\n", buff);
				ecb(buff, octetiCititi, decryptedtext);
				
			}
			close(q[0]);
		}
	
	
	return 0;	
}
int cfb (int q, unsigned char* key, unsigned char* iv)
{
	int octetiCititi;
	unsigned char buff[50000];
	unsigned char buff2[50000];
	unsigned char decrypted_block[128];
	unsigned char xor_result[1000];
	int buff2_len, dec_len;
	int xor_len=0;
	while (read(q, &octetiCititi, sizeof(int))!=0)
	{
		read(q, buff, octetiCititi);
		if (octetiCititi == 1)
		{
			xor_len = 0;
			continue;
		}
		buff[octetiCititi] = 0;
		printf("Bloc criptat citit:\n");
		printf("%s!\n", buff);
		//printf("EU SUNT BUFF2: %s!\n", buff2);
		buff2_len = octetiCititi;
		printf("LG BUFF CITIT: %d\n", octetiCititi);
		if (xor_len == 0)
			//dec_len = encrypt (iv, 16, key, NULL, decrypted_block);
			dec_len = XOR(iv, key, decrypted_block);
		else
			//dec_len = encrypt (buff2, 16, key, NULL, decrypted_block);
			dec_len = XOR(buff2, key, decrypted_block);
		decrypted_block[dec_len]=0;
		
		printf("LG encrypted: %d\n", dec_len);
		printf("Bloc criptat:\n");
		printf("%s!\n", decrypted_block);
		strcpy((char*)buff2, (char*)buff);
		
		xor_len = XOR(decrypted_block, buff, xor_result);
		printf("Rezultat cu XOR are lg %d:\n", xor_len);
		printf("%s!\n\n", xor_result);
		
		write(q, &xor_len, sizeof(int));
		write(q, xor_result, xor_len);
		
	}

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


int ecb(unsigned char* buff, int octetiCititi, unsigned char* key)
{
	unsigned char block_from_A[1000];
	int dec_len;
	
	//dec_len = decrypt(buff, octetiCititi, key, NULL, block_from_A);
	dec_len = XOR(buff, key, block_from_A);
	block_from_A[dec_len] = 0;
	printf("%s!\n\n", block_from_A);
	
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
