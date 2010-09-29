#include <stdio.h>

int main(int argc, char* argv[]) {
  char* fname_in;
  char* fname_out;
  FILE* fp_in;
  FILE* fp_out;
  const int BUFSIZE=0xFFFF;
  unsigned char buf[BUFSIZE];
  size_t size;
  int i;

  if (argc != 3) {
    printf("Usage: %s input.dat output.c\n", argv[0]);
    return 1;
  }
  fname_in = argv[1];
  fname_out = argv[2];
  fp_in = fopen(fname_in, "r");
  fp_out = fopen(fname_out, "w");
  fprintf(fp_out, "unsigned char resource_data[] = {\n");

  while (size = fread(buf, 1, BUFSIZE, fp_in)) {
    for (i = 0; i < size; ++i) {
      int last = feof(fp_in) && i == size - 1;
      fprintf(fp_out, "    0x%x%s\n", buf[i], last ? "" : ",");
    }
  }
  fprintf(fp_out, "};\n");
  fclose(fp_in);
  fclose(fp_out);
  return 0;
}
