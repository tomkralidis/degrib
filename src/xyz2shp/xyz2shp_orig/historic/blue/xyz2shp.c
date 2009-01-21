#include <stdio.h>
#include <string.h>
#include "myassert.h"
#include "memwatch.h"
#include "myutil.h"
#include "type.h"
#include "tendian.h"

typedef struct {
   double lat, lon, data;
   char **ptr;
   int numPtr;
} statInfo_type;

typedef struct {
   char label[12];
   char type;        /* 'C' for character.
                      * 'N' for number. */
   long int address; /* Address of data if it is in memory? */
   uChar fldLen;
   uChar fldDec;
} dbfCol_type;

/*****************************************************************************
 * checkFileSize() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   To make sure that a finished file is indeed the size we expect it to be.
 *
 * ARGUMENTS
 * filename = Name of file to check the size of. (Input)
 *      len = expected size. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *  -1 = error opening file for check.
 *  -2 = file was not the right size.
 *   0 = OK.
 *
 * HISTORY
 *   6/2004 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *****************************************************************************
 */
int checkFileSize (char *filename, unsigned long int len)
{
   FILE *fp;            /* The file to check the size of. */

   if ((fp = fopen (filename, "rb")) == NULL) {
      printf ("ERROR: Problems opening %s for file length check.", filename);
      return -1;
   }
   fseek (fp, 0L, SEEK_END);
   if (ftell (fp) != len) {
      printf ("ERROR: file %s is %ld not %ld bytes long.\n", filename,
              ftell (fp), len);
      fclose (fp);
      return -2;
   }
   fclose (fp);
   return 0;
}

/*****************************************************************************
 * stnCreateShpPnt() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   To create the spatial .shp and .shx files.
 *
 * ARGUMENTS
 * filename = Name of file to write to. (Input)
 *     stat = stations to write to file. (Input)
 *  numStat = Number of stations in stat. (Input)
 *   minLon = Minimum longitude in stat. (Input)
 *   minLat = Minimum latitude in stat. (Input)
 *   maxLon = Maximum longitude in stat. (Input)
 *   maxLat = Maximum latitude in stat. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *  -1 = error opening file, or a mistake creating the file caused it to be
 *       the wrong size.
 *   0 = ok.
 *
 * HISTORY
 *   6/2004 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *****************************************************************************
 */
int stnCreateShpPnt (char *filename, statInfo_type *stat, int numStat,
                     double minLon, double minLat, double maxLon,
                     double maxLat)
{
   FILE *sfp;           /* The open file pointer for .shp */
   FILE *xfp;           /* The open file pointer for .shx. */
   int i;               /* A counter used for the shx values. */
   long int Head1[7];   /* The Big endian part of the Header. */
   long int Head2[2];   /* The Little endian part of the Header. */
   double Bounds[] = {
      0., 0., 0., 0., 0., 0., 0., 0.
   };                   /* Spatial bounds of the data. minLon, minLat,
                         * maxLon, maxLat, ... */
   long int dataType = 1; /* Point shp type data. */
   long int curRec[2];  /* rec number, and content length. */
   int recLen;          /* Length in bytes of a record in the .shp file. */
   long int numRec = numStat; /* The total number of records. */
   int err = 0;         /* Internal err number. */

   strncpy (filename + strlen (filename) - 3, "shp", 3);
   if ((sfp = fopen (filename, "wb")) == NULL) {
      printf ("ERROR: Problems opening %s for write.", filename);
      return -1;
   }

   /* Start Writing header in first 100 bytes. */
   Head1[0] = 9994;     /* ArcView identifier. */
   memset ((Head1 + 1), 0, 5 * sizeof (long int)); /* set 5 unused to 0 */
   recLen = sizeof (long int) + 2 * sizeof (double);
   /* .shp file size (in 2 byte words). */
   /* Initial guess (there may be some stations without lat/lon values) */
   Head1[6] = (100 + (2 * sizeof (long int) + recLen) * numRec) / 2;
   FWRITE_BIG (Head1, sizeof (long int), 7, sfp);
   Head2[0] = 1000;     /* ArcView version identifier. */
   Head2[1] = dataType; /* Signal that these are point data. */
   FWRITE_LIT (Head2, sizeof (long int), 2, sfp);
   Bounds[0] = minLon;
   Bounds[1] = minLat;
   Bounds[2] = maxLon;
   Bounds[3] = maxLat;
   FWRITE_LIT (Bounds, sizeof (double), 8, sfp);

   /* Start Writing data. */
   curRec[1] = recLen / 2; /* Content length in (2 byte words) */
   curRec[0] = 0;
   for (i = 0; i < numStat; i++) {
      curRec[0]++;
      FWRITE_BIG (curRec, sizeof (long int), 2, sfp);
      FWRITE_LIT (&dataType, sizeof (long int), 1, sfp);
      FWRITE_LIT (&(stat[i].lon), sizeof (double), 1, sfp);
      FWRITE_LIT (&(stat[i].lat), sizeof (double), 1, sfp);
   }
   /* Store the updated file length. */
   /* .shp file size (in 2 byte words). */
   numRec = curRec[0];
   if (numStat != numRec) {
      Head1[6] = (100 + (2 * sizeof (long int) + recLen) * numRec) / 2;
      fseek (sfp, 24, SEEK_SET);
      FWRITE_BIG (&(Head1[6]), sizeof (long int), 1, sfp);
   }
   fclose (sfp);
   if ((err = checkFileSize (filename, Head1[6] * 2)) != 0) {
      return err;
   }

   /* Write ArcView header (.shx file). */
   filename[strlen (filename) - 1] = 'x';
   if ((xfp = fopen (filename, "wb")) == NULL) {
      printf ("ERROR: Problems opening %s for write.", filename);
      return -1;
   }
   Head1[6] = (100 + 8 * numRec) / 2; /* shx file size (in words). */
   FWRITE_BIG (Head1, sizeof (long int), 7, xfp);
   FWRITE_LIT (Head2, sizeof (long int), 2, xfp);
   FWRITE_LIT (Bounds, sizeof (double), 8, xfp);

   curRec[0] = 50;      /* 100 bytes / 2 = 50 words */
   curRec[1] = recLen / 2; /* Content length in words (2 bytes) */
   for (i = 0; i < numRec; i++) {
      FWRITE_BIG (curRec, sizeof (long int), 2, xfp);
      curRec[0] += (recLen + 2 * sizeof (long int)) / 2; /* (2 byte words) */
   }
   fclose (xfp);
   if ((err = checkFileSize (filename, Head1[6] * 2)) != 0) {
      return err;
   }
   return 0;
}

/*****************************************************************************
 * DbfInit() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   To initialize a .dbf file.
 *
 * ARGUMENTS
 * filename = Name of file to write to. (Input)
 *      col = The column structure information. (Input)
 *   numCol = Number of columns. (Input)
 *   numRec = Number of records. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *  -1 = error opening file, or a mistake creating the file caused it to be
 *       the wrong size.
 *   0 = ok.
 *
 * HISTORY
 *   6/2004 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *****************************************************************************
 */
int DbfInit (char *filename, dbfCol_type *col, int numCol, long int numRec)
{
   FILE *fp;            /* The open .dbf file pointer. */
   unsigned short int recLen; /* Size of one cell of data */
   int i;               /* Counter variable. */
   unsigned short int headLen; /* Size of the header. */
   unsigned long int totSize; /* Total size of the .dbf file. */
   uChar header[] = { 3, 101, 4, 20 }; /* Header info for dbf. */
   long int reserved[] = { 0, 0, 0, 0, 0 }; /* need 20 bytes of 0. */
   uChar uc_temp;       /* Store the last character in the header. */
   char *buffer;        /* Used to store the "dummy records" */

   strncpy (filename + strlen (filename) - 3, "dbf", 3);
   if ((fp = fopen (filename, "wb")) == NULL) {
      printf ("ERROR: Problems opening %s for write.", filename);
      return -1;
   }
   recLen = 1;
   for (i = 0; i < numCol; i++) {
      recLen += col[i].fldLen;
   }
   headLen = 1 + 32 + 32 * numCol;
   totSize = headLen + recLen * numRec;

   /* Start writing the header. */
   fwrite (header, sizeof (char), 4, fp);
   FWRITE_LIT (&numRec, sizeof (long int), 1, fp);
   FWRITE_LIT (&headLen, sizeof (short int), 1, fp);
   FWRITE_LIT (&recLen, sizeof (short int), 1, fp);
   fwrite (reserved, sizeof (char), 20, fp);

   /* Write first (label) field descriptors. */
   for (i = 0; i < numCol; i++) {
      fwrite (col[i].label, sizeof (char), 11, fp);
      fputc (col[i].type, fp);
      FWRITE_LIT (&(col[i].address), sizeof (long int), 1, fp);
      fputc (col[i].fldLen, fp);
      fputc (col[i].fldDec, fp);
      fwrite (reserved, sizeof (char), 14, fp);
   }

   /* Write trailing header character. */
   uc_temp = 13;
   fputc (uc_temp, fp);

   /* Start writing the body. */
   buffer = (char *) malloc (recLen * sizeof (char));
   memset (buffer, ' ', recLen);
   for (i = 0; i < numRec; i++) {
      fwrite (buffer, sizeof (char), recLen, fp);
   }

   free (buffer);
   fclose (fp);
   return checkFileSize (filename, totSize);
}

int ReadXYZ (char *filename, statInfo_type **Stat, int *numStat,
             dbfCol_type **Col, int *numCol)
{
   FILE *fp;
   int lineLen = 0; /* Allocated space of line. */
   char *line = NULL;  /* The current line of the U850 file. */
   int lineArgc = 0;
   char ** lineArgv = NULL;
   int i;
   int lineNum;
   int fldLen;
   int decLen;
   char *decPtr;

   myAssert (filename != NULL);
   if ((fp = fopen (filename, "rt")) == NULL) {
      printf ("Couldn't open %s\n", filename);
      return -1;
   }
   lineNum = 0;
   while (reallocFGets (&line, &lineLen, fp) != 0) {
      strTrim (line);
      if (strlen (line) == 0) {
         continue;
      }
      mySplit (line, ',', &lineArgc, &lineArgv, 1);
      myAssert (lineArgc >= 3);
      if (lineNum == 0) {
         *numCol = lineArgc;
         *Col = (dbfCol_type *) malloc (*numCol * sizeof (dbfCol_type));
         for (i = 0; i < lineArgc; i++) {
            (*Col)[i].address = 0;
            strToUpper (lineArgv[i]);
            strncpy ((*Col)[i].label, lineArgv[i], 11);
            (*Col)[i].label[11] = '\0';
            if (i >= 3) {
               (*Col)[i].type = 'C';
            } else {
               (*Col)[i].type = 'N';
            }
            (*Col)[i].fldLen = 0;
            (*Col)[i].fldDec = 0;
         }
      } else {
         *numStat = *numStat + 1;
         (*Stat) = (statInfo_type *) realloc ((void *)(*Stat),
                                            *numStat * sizeof (statInfo_type));
         myAssert (*numCol >= 3);
         (*Stat)[*numStat-1].ptr = (char **) malloc ((*numCol - 3) * sizeof (char *));
         (*Stat)[*numStat-1].numPtr = *numCol - 3;
         for (i = 0; i < lineArgc; i++) {
            strTrim (lineArgv[i]);
            fldLen = strlen (lineArgv[i]);
            if (fldLen > (*Col)[i].fldLen) {
               (*Col)[i].fldLen = fldLen;
            }
            if (i < 3) {
               decPtr = strchr (lineArgv[i], '.');
               if (decPtr != NULL) {
                  decLen = fldLen - (decPtr - lineArgv[i]);
               } else {
                  decLen = 0;
               }
               if (decLen > (*Col)[i].fldDec) {
                  (*Col)[i].fldDec = decLen;
               }
               if (i == 0) {
                  myAssert (strcmp ((*Col)[i].label, "LAT") == 0);
                  (*Stat)[*numStat-1].lat = atof (lineArgv[i]);
               } else if (i == 1) {
                  myAssert (strcmp ((*Col)[i].label, "LON") == 0);
                  (*Stat)[*numStat-1].lon = atof (lineArgv[i]);
               } else {
                  (*Stat)[*numStat-1].data = atof (lineArgv[i]);
               }
            } else {
               (*Stat)[*numStat-1].ptr[i-3] = (char *) malloc ((fldLen + 1) * sizeof (char));
               strcpy ((*Stat)[*numStat-1].ptr[i-3], lineArgv[i]);
            }
         }
      }
      for (i = 0; i < lineArgc; i++) {
         free (lineArgv[i]);
      }
      free (lineArgv);
      lineArgv = NULL;
      lineArgc = 0;
      lineNum ++;
   }
   free (line);
   fclose (fp);
   return 0;
}

int DbfWrite (char *filename, dbfCol_type *col, int numCol,
              statInfo_type *stat, int numStat)
{
   FILE *fp;            /* The open .dbf file pointer. */
   long int offset;
   char *buffer;        /* Used to store the records */
   int i;
   int j;
   int cur;
   unsigned short int recLen; /* Size of one cell of data */
   char format[100];
   char *ptr;

   strncpy (filename + strlen (filename) - 3, "dbf", 3);
   if ((fp = fopen (filename, "r+b")) == NULL) {
      printf ("ERROR: Problems opening %s for append.", filename);
      return -1;
   }
   recLen = 1;
   for (i = 0; i < numCol; i++) {
      recLen += col[i].fldLen;
   }
   buffer = (char *) malloc ((recLen + 1) * sizeof (char));
   offset = 1 + 32 + 32 * numCol;
   fseek (fp, offset, SEEK_SET);
   for (i = 0; i < numStat; i++) {
      buffer[0] = ' ';
      ptr = buffer + 1;
      cur = 1;
      for (j = 0; j < numCol; j++) {
         if (col[j].type == 'N') {
            sprintf (format, "%%%d.%df", col[j].fldLen, col[j].fldDec);
#ifdef DEBUG
            printf ("format :: %s\n", format);
#endif
            if (j == 0) {
               sprintf (ptr, format, stat[i].lat);
            } else if (j == 1) {
               sprintf (ptr, format, stat[i].lon);
            } else {
               sprintf (ptr, format, stat[i].data);
            }
         } else if (col[j].type == 'C') {
            sprintf (format, "%%%ds", col[j].fldLen);
            sprintf (ptr, format, stat[i].ptr[j-3]);
         } else {
            myAssert (1==2);
         }
         ptr += col[j].fldLen;
         cur += col[j].fldLen;
      }
#ifdef DEBUG
      printf ("%s\n", buffer);
#endif
      fwrite (buffer, sizeof (char), recLen, fp);
      myAssert (cur == recLen);
   }
   free (buffer);
   fclose (fp);
   return 0;
}

int main (int argc, char **argv) {
   statInfo_type *Stat = NULL;
   int numStat = 0;
   dbfCol_type *Col = NULL;
   int numCol = 0;
   int i, j;
   double minLon;
   double minLat;
   double maxLon;
   double maxLat;

   if (argc != 3) {
      printf ("usage %s <xyz file> <shapefile (with extension)>\n", argv[0]);
      return -1;
   }

   ReadXYZ (argv[1], &Stat, &numStat, &Col, &numCol);

#ifdef DEBUG
   for (i=0; i < numCol; i++) {
      printf ("%s %c %d %d\n", Col[i].label, Col[i].type, Col[i].fldLen, Col[i].fldDec);
   }
   for (i=0; i < numStat; i++) {
      printf ("%f %f %f", Stat[i].lat, Stat[i].lon, Stat[i].data);
      for (j=0; j < Stat[i].numPtr; j++) {
         printf ("(%d,%s)", j, Stat[i].ptr[j]);
      }
      printf ("\n");
   }
#endif

   minLon = maxLon = Stat[0].lon;
   minLat = maxLat = Stat[0].lat;
   for (i=1; i < numStat; i++) {
      if (minLon > Stat[i].lon) {
         minLon = Stat[i].lon;
      } else if (maxLon < Stat[i].lon) {
         maxLon = Stat[i].lon;
      }
      if (minLat > Stat[i].lat) {
         minLat = Stat[i].lat;
      } else if (maxLat < Stat[i].lat) {
         maxLat = Stat[i].lat;
      }
   }

   stnCreateShpPnt (argv[2], Stat, numStat, minLon, minLat, maxLon, maxLat);
   DbfInit (argv[2], Col, numCol, numStat);
   DbfWrite (argv[2], Col, numCol, Stat, numStat);

   free (Col);
   Col = NULL;
   numCol = 0;

   for (i=0; i < numStat; i++) {
      for (j=0; j < Stat[i].numPtr; j++) {
         free (Stat[i].ptr[j]);
      }
      free (Stat[i].ptr);
   }
   free (Stat);
   Stat = NULL;
   numStat = 0;

   return 0;
}