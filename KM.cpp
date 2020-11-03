#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
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



using namespace std;

void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext);


int main(int argc, char* argv[])
{

	int p[2], q[2];
  	int octetiCititi;
	char buff[50000];
	unsigned char ciphertext[128];
   	unsigned char decryptedtext[128];
   	int decryptedtext_len, ciphertext_len;



	while (1)
	{
	
		if(-1 == mkfifo("A_TO_KM", 0600) )
		{
			if(errno != EEXIST)
				fprintf(stdout,"Nota: canalul fifo exista deja !\n");
			else
				perror("Eroare la crearea canalului fifo. Cauza erorii");  exit(1);
		}
		if(-1 == mkfifo("KM_TO_A", 0600) )
		{
			if(errno != EEXIST)   // errno=17 for "File already exists"
				fprintf(stdout,"Nota: canalul fifo exista deja !\n");
			else
				perror("Eroare la crearea canalului fifo. Cauza erorii");  exit(2);
		}
		if (-1==(q[0] = open("A_TO_KM",O_RDONLY)))
	 	{
			perror("Eroare la open");exit(8);
		}
		while (read(q[0], &octetiCititi, sizeof(int))!=0)
	 	{
	       	read(q[0], buff, octetiCititi);
	       	buff[octetiCititi]=0;
	       	//Am citit cheia de la KM
	       	printf("Am citit modul dorit:\n");
	       }
	       close(q[0]);
	       unsigned char key[17];
	       
	       if (!RAND_bytes(key, sizeof key)) {
			return (-1);
		}
		key[16]=0;
		printf("Cheia: %s\n", key);


		unsigned char *plaintext = key;
		unsigned char *K3 = (unsigned char *)"0123456789012345";


	    	ciphertext_len = encrypt (plaintext, strlen ((char *)plaintext), K3, NULL,
		                      ciphertext);
		ciphertext[ciphertext_len]=0;
	       
	       
	       if(-1==(q[1] = open("KM_TO_A",O_WRONLY)))
		    {
			perror("eroare la open");exit(8);
		    }
		//char mesaj[100]= "Va trimit cheia";
		octetiCititi = ciphertext_len;
		//printf("%s\n", mesaj);
		printf("Cheia criptata cu K3: %s\n", ciphertext);
		for (int i=0;i<ciphertext_len;i++)
			printf("%02x", ciphertext[i]);
		printf("\n");
		
		write(q[1], &octetiCititi, sizeof(int));
		write(q[1], ciphertext, octetiCititi);
		close(q[1]);
		
		unlink("A_TO_KM");
		unlink("KM_TO_A");

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

