/**
 * csv_file header
 */
#ifndef _CSV_FILE_H_
#define _CSV_FILE_H_

#include "ade9000.h"

extern int File_open(char *fileName);

extern int File_close();

extern int File_append(RealValue_t *portA, RealValue_t *portB, RealValue_t *portC);

#endif