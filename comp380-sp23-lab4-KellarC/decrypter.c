#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <openssl/des.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

/**
 * decrypt_des - decrypts a block of memory with the DES algorithm
 * @param key - character string containing decryption key
 * @param msg - array of bytes to decrypt
 * @param size - size of msg
 * @returns decrypted array of bytes
 */
char *decrypt_des(char *key, char *msg, int size) {
  static char*    res;
  int             n=0;

  DES_cblock      key2;
  DES_key_schedule schedule;

  res = (char *) malloc(size);

  /* Prepare the key for use with DES_cfb64_encrypt */
  memcpy(key2, key,8);
  DES_set_odd_parity(&key2);
  DES_set_key_checked(&key2, &schedule);

  /* Decryption occurs here */
  DES_cfb64_encrypt((unsigned char *) msg, (unsigned char *) res,
      size, &schedule, &key2, &n, DES_DECRYPT);
  return (res);
}


/*
 *  main function
 */
int main(int argc, char **argv) {
  char* pass = "zyxyz";  // my pass = Sh03s 
  char *buf, *cbuf;  // buffers for encrypt/decrypt
  struct stat st;
  char *fileext = ".txt";   // file extension for encrypted file
  int rfd, wfd;             // file descriptors
  char *filename, *cfilename;  // encrypted and text filenames
  int len, extlen, clen;    // buffer lengths


  /*
   * check arguments for sanity
   */
  if (argc != 2) {
    printf("\tusage: decrypter <filename>\n");
    exit(1);
  }

   /*
   * read file, decrypt with password string and store in output file
   */
  filename = argv[1];
  len = strnlen(filename, 1024); // max length 1024
  extlen = strnlen(fileext, 1024);
  clen = len + extlen + 1; // account for \0 terminator
  cfilename = malloc(clen);
  strncpy(cfilename, filename, len);
  strncat(cfilename, fileext, extlen); // append .des to input file
  printf("source: %s crypted: %s\n", filename, cfilename);

  // get input file size
  if (stat(filename, &st) != 0) {
    perror("stat");
    exit(1);
  }

  // allocate a buffer for the input file
  buf  = malloc(st.st_size+1);

  // open input file
  if ((rfd = open(filename, O_RDONLY)) == -1) {
    perror("open (reading)");
    exit(1);
  }

  // open output file
  if ((wfd = open(cfilename, O_CREAT|O_WRONLY|O_TRUNC, st.st_mode)) == -1) {
    perror("open (writing)");
    exit(1);
  }


  read(rfd, buf, st.st_size); // read entire file into buffer
  buf[st.st_size] = '\0';     // null terminate string

  // encrypt file data
  cbuf = decrypt_des(pass, buf, st.st_size);

  // save it in output file
  write(wfd, cbuf, st.st_size);

  close(rfd);
  close(wfd);

  free(cfilename);
  free(buf);
  free(cbuf);
}
