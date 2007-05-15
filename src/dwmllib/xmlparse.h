#ifndef XMLPARSE_H
#define XMLPARSE_H
#include "genprobe.h"
#include "type.h"
#include "sector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "xmlparse.h"
#include "genprobe.h"
#include "grpprobe.h"
#include "clock.h"
#ifdef MEMWATCH
#include "memwatch.h"
#endif
#include "myassert.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "myutil.h"
#include "solar.h"
#define NDFD_DAYS 6

/* Set all choices for the period names for those elements needing them in the 
 * time layouts. 
 */
enum
{ earlyMorning, morning12, afternoon12, earlyMorningMaxT, earlyMorningMinT,
  morning24, afternoon24, MAX_PERIODS
};

typedef struct                /* Denotes structure of the time layouts. */
{
   int period;
   uChar numRows;
   char fmtdStartTime[30];
} layouts;

typedef struct                /* Denotes structure of icon info. */
{
   double validTime;
   char str[500];
   sChar valueType;
} icon_def;

typedef struct                /* Denotes structure of element's Sky cover, 
                               * Temperature, Wind Speed, and Pop info.
			       * Used in derivation of icons and weather. 
			       */
{
   double validTime;
   int data;
   sChar valueType;
} elem_def;

typedef struct                /* Denotes structure of Weather info. Used in 
                               * derivation of icons and weather. 
                               */
{
   double validTime;
   char str[600];
   sChar valueType;
} WX;

typedef struct                /* Structure with info on the number of rows 
                               * skipped due to a startTime and/or endTime 
                               * effectively shortening the time data was
			       * retrieved for from NDFD. 
			       */
{
   int total; /* Total number of rows interested in. If a user shortens
                 the time period data is retrieved for, this value is 
                 reduced by these "skipped data". */
   int skipBeg; /* If a user shortens the time period data is retrieved
                   for, this value is the number of rows skipped at the
                   beginning. */
   int skipEnd; /* If a user shortens the time period data is retrieved
                   for, this value is the number of rows skipped at the
                   end. */
   double firstUserTime; /* First validTime of an element interested in. 
                            If a user does not shorten the time period
                            data is retrieved for, this value is simply
                            the first validTime for the element returned
                            from the match structure. */
   double lastUserTime;  /* Last validTime of an element interested in. 
                            If a user does not shorten the time period
                            data is retrieved for, this value is simply
                            the last validTime for the element returned
                            from the match structure. */
} numRowsInfo;

typedef struct                /* Denotes structure of sector info for use in a 
                               * multiple point call in which points lie in 
                               * different sectors. 
                               */
{
   int startNum;
   int endNum;
   char name[15];
   int enumNum;
} sectInfo;
 
/* Declare XMLParse() interface. */

int checkNeedForEndTime(uChar parameterName);

void checkNeedForPeriodName(int index, uChar * numPeriodNames,
                            sChar timeZoneOffset, uChar parameterName,
                            char *parsedDataTime, uChar * outputPeriodName, 
                            uChar issuanceType, char *periodName, 
                            char *currentHour, char *currentDay, 
                            double startTime_cml, double currentDoubTime,
                            double firstValidTime, int period);

void computeStartEndTimes(uChar parameterName, uChar numFmtdRows,
                          int periodLength, sChar TZoffset,
                          sChar f_observeDST, genMatchType * match,
                          uChar useEndTimes, char **startTimes, char **endTimes,
                          char *frequency, uChar f_XML, double startTime_cml, 
			  double currentDoubTime, numRowsInfo numRows, 
                          int startNum, int endNum);

void determineIconUsingPop(char *iconString, char *wxStrSection, 
		           char *jpgStrSection, int POP12ValToPOP3, 
			   char *baseURL);

void determineNonWeatherIcons(int windTimeEqualsWeatherTime, int itIsNightTime, 
                              elem_def *wsInfo, int wsIndex, char *baseURL, 
                              int numRowsWS, icon_def *iconInfo, int wxIndex, 
                              int numRowsTEMP, elem_def *tempInfo, 
                              int hourlyTempIndex, 
                              int hourlyTempTimeEqualsWeatherTime);

int determinePeriodLength(double startTime, double endTime, uChar numRows, 
                          uChar parameterName);

void determineSkyIcons(int skyCoverTimeEqualsWeatherTime, int itIsNightTime, 
                       int skyIndex, int wxIndex, elem_def *skyInfo, 
                       icon_def *iconInfo, char *baseURL, int numRowsSKY);

void determineWeatherIcons(icon_def *iconInfo, int numGroups, char **wxType,
                           int skyCoverTimeEqualsWeatherTime, int itIsNightTime, 
                           elem_def *skyInfo, char *baseURL, int numRowsSKY, 
                           int skyIndex, int wxIndex, 
                           int windTimeEqualsWeatherTime, elem_def *wsInfo, 
                           int wsIndex, int numRowsWS, int numRowsTEMP, 
                           int hourlyTempIndex,
                           int hourlyTempTimeEqualsWeatherTime,
                           elem_def *tempInfo, int POP12ValToPOP3);

void formatLocationInfo(size_t numPnts, Point * pnts, xmlNodePtr data);

void formatMetaDWML(uChar f_XML, xmlDocPtr * doc, xmlNodePtr * data,
                    xmlNodePtr * dwml);
 
int formatValidTime(double validTime, char *timeBuff, int size_timeBuff, 
                    sChar pntZoneOffSet, sChar f_dayCheck);

void genAppTempValues(size_t pnt, char *layoutKey, genMatchType *match,
                      xmlNodePtr parameters, numRowsInfo numRows, int startNum,
		      int endNum);

void genConvOutlookValues(size_t pnt, char *layoutKey, genMatchType *match, 
                          xmlNodePtr parameters, numRowsInfo numRows, 
                          int startNum, int endNum);

void genConvSevereCompValues(size_t pnt, char *layoutKey, uChar parameterName, 
                             genMatchType *match, char *severeCompType, 
                             char *severeCompName, xmlNodePtr parameters, 
                             numRowsInfo numRows, int startNum, int endNum);

void genDewPointTempValues(size_t pnt, char *layoutKey, genMatchType *match, 
                           xmlNodePtr parameters, numRowsInfo numRows, 
                           int startNum, int endNum);

void genIconLinks(icon_def *iconInfo, uChar numRows, char *layoutKey, 
                  xmlNodePtr parameters);

void genMaxTempValues(size_t pnt, char *layoutKey, genMatchType * match,
                      xmlNodePtr parameters, int f_formatNIL, uChar f_XML, 
                      double startTime_cml, numRowsInfo numRows, 
                      int numFmtdRows, int startNum, int endNum);

void genMinTempValues(size_t pnt, char *layoutKey, genMatchType *match,
                      xmlNodePtr parameters, uChar f_XML, double startTime_cml,
                      numRowsInfo numRows, char *currentDay, char *currentHour, 
                      sChar TZoffset, sChar f_observeDST, int numFmtdRows, 
                      int startNum, int endNum);

void genPopValues(size_t pnt, char *layoutKey, genMatchType *match,
                  xmlNodePtr parameters, numRowsInfo numRows, uChar f_XML,
		  double startTime_cml, int *maxDailyPop, int *numDays, 
                  double currentDoubTime, char *currentHour, int startNum, 
                  int endNum);

void genQPFValues(size_t pnt, char *layoutKey, genMatchType *match,
                  xmlNodePtr parameters, numRowsInfo numRows, int startNum, 
                  int endNum);

void genRelHumidityValues(size_t pnt, char *layoutKey, genMatchType * match, 
                           xmlNodePtr parameters, numRowsInfo numRows, 
                           int startNum, int endNum);

void genSkyCoverValues(size_t pnt, char *layoutKey, genMatchType * match,
                       xmlNodePtr parameters, char *startDate, int *maxSkyCover,
                       int *minSkyCover, int *averageSkyCover, 
                       int *numOutputLines, int timeInterval, sChar TZoffset, 
                       sChar f_observeDST, uChar parameterName,
                       numRowsInfo numRows, uChar f_XML, int *maxSkyNum,
                       int *minSkyNum, int *startPositions, int *endPositions, 
                       int *SKYintegerTime, char *currentHour, 
                       double timeUserStart, int f_6CycleFirst, 
                       double startTime, int startNum, int endNum);

void genSnowValues(size_t pnt, char *layoutKey, genMatchType *match,
                   xmlNodePtr parameters, numRowsInfo numRows, int startNum, 
                   int endNum);

void genTempValues(size_t pnt, char *layoutKey, genMatchType * match,
                   xmlNodePtr parameters, numRowsInfo numRows, int startNum, 
                   int endNum);

void genWaveHeightValues(size_t pnt, char *layoutKey, genMatchType * match, 
                         xmlNodePtr parameters, numRowsInfo numRows, 
                         int startNum, int endNum);

void genWeatherValues(size_t pnt, char *layoutKey, genMatchType *match,
                      uChar f_wx, int f_icon, numRowsInfo numRowsWS, 
                      numRowsInfo numRowsSKY, numRowsInfo numRowsTEMP, 
                      numRowsInfo numRowsWX, numRowsInfo numRowsPOP, 
                      xmlNodePtr parameters, double lat, double lon, 
                      int startNum, int endNum, sChar TZoffset, 
                      sChar f_observeDST);

void genWeatherValuesByDay(size_t pnt, char *layoutKey, 
		           genMatchType *match, size_t numMatch,
                           numRowsInfo numRowsWS, numRowsInfo numRowsPOP, 
                           numRowsInfo numRowsMAX, numRowsInfo numRowsMIN, 
                           numRowsInfo numRowsWX, xmlNodePtr parameters, 
                           int *numDays, sChar TZoffset,
			   sChar f_observeDST, char *frequency,
			   int f_useMinTempTimes, uChar f_XML,
			   int *numOutputLines, int *maxDailyPop, 
			   int *averageSkyCover, int *maxSkyCover, 
			   int *minSkyCover, int *maxSkyNum, int *minSkyNum, 
			   int *startPositions, int *endPositions, 
			   int *maxWindSpeed, int *maxWindDirection, 
			   int integerTime, int integerStartUserTime, 
			   double startTime_cml, int f_6CycleFirst, 
                           int format_value, int startNum, int endNum);

void genWindDirectionValues(size_t pnt, char *layoutKey, genMatchType * match,
                            xmlNodePtr parameters, int *maxWindDirection,
                            uChar f_XML, int *numOutputLines,
	                    double *valTimeForWindDirMatch, 
		            numRowsInfo numRows, int startNum, int endNum);

void genWindIncCumValues(size_t pnt, char *layoutKey, uChar parameterName,
		         genMatchType *match, char *windSpeedType, 
		         char *windSpeedName, xmlNodePtr parameters,
		         numRowsInfo numRows, int startNum, int endNum);

void genWindSpeedGustValues(size_t pnt, char *layoutKey,  genMatchType *match, 
                            xmlNodePtr parameters, numRowsInfo numRows, 
                            int startNum, int endNum);

void genWindSpeedValues(double timeUserStart, double timeUserEnd, size_t pnt, 
                        char *layoutKey, genMatchType * match,
                        xmlNodePtr parameters, char *startDate,
                        int *maxWindSpeed, int *numOutputLines, int timeInterval,
                        sChar TZoffset, sChar f_observeDST, uChar parameterName,
                        numRowsInfo numRows, uChar f_XML,
                        double *valTimeForWindDirMatch, int f_6CycleFirst, 
                        double startTime, int startNum, int endNum);

void generatePhraseAndIcons (int dayIndex, char *frequency, 
                             int timeLayoutHour, char *dominantWeather[4],
			     char *baseURL, int *maxDailyPop, 
			     int *averageSkyCover, int *maxSkyCover,
			     int *minSkyCover, int *maxSkyNum, 
			     int *minSkyNum, int *periodMaxTemp, 
			     double springDoubleDate, 
			     double fallDoubleDate,  int *maxWindSpeed, 
			     int *maxWindDirection, int integerTime, 
			     int integerStartUserTime, int *startPositions, 
			     int *endPositions, int f_isDrizzle, 
			     int f_isRain, int f_isRainShowers, 
			     int f_isIcePellets, int f_isSnow, 
			     int f_isSnowShowers, int f_isFreezingDrizzle, 
			     int f_isFreezingRain, int f_isBlowingSnow, 
                             icon_def *iconInfo, char **phrase, 
                             int *f_popIsNotAnIssue);

void generateTimeLayout(numRowsInfo numRows, uChar parameterName,
                        char *layoutKey, const char *timeCoordinate,
                        char *summarization, genMatchType * match,
                        size_t numMatch, uChar f_formatPeriodName,
                        sChar TZoffset, sChar f_observeDST,
                        size_t * numLayoutSoFar,
                        uChar * numCurrentLayout, char *currentHour,
                        char *currentDay, char *frequency,
                        xmlNodePtr data, double startTime_cml,
                        double currentDoubTime, int *numFmtdRows,
			uChar f_XML, int startNum, int endNum);

void getColdSeasonTimes(genMatchType *match, numRowsInfo numRowsWS,
                        sChar TZoffset, double **springDoubleDate, 
			double **fallDoubleDate, int startNum, int endNum);

void getFirstSecondValidTimes(double *firstValidTime, double *secondValidTime,
		              genMatchType *match, size_t numMatch, 
                              uChar parameterName, int startNum, int endNum, 
                              int numRows, int numRowsSkippedBeg, 
                              int numRowsSkippedEnd);

void getNumRows(numRowsInfo *numRowsForPoint, double *timeUserStart, 
		double *timeUserEnd, size_t numMatch, genMatchType *match, 
                uChar *wxParameters, uChar f_XML, sChar *f_icon, sChar TZoffset, 
                sChar f_observeDST, int startNum, int endNum, char *startDate, 
                int *numDays, double startTime, double endTime, 
                char currentHour[3], double *firstValidTime_pop, 
                int *f_6CycleFirst, double *firstValidTimeMatch, 
                int *f_formatIconForPnt, int pnt);

void getPeriodInfo(uChar parameterName, char *firstValidTime, char *currentHour, 
                   char *currentDay, uChar * issuanceType, 
                   uChar * numPeriodNames, int period, char *frequency);

void getSectorInfo(PntSectInfo *pntInfo, Point *pnts, size_t numPnts,
                   genMatchType *match, size_t numMatch, 
                   size_t numSector, char **sector, int f_nhemi, 
                   int *f_puertori, int *f_conus, int numNhemi);

void getStartDates(char **startDate, uChar f_XML, double startTime, 
		   double firstValidTimeMatch, double firstValidTime_maxt,
                   sChar TZoffset, sChar f_observeDST, int point);

void getTranslatedCoverage(char *uglyStr, char *transStr);

void getTranslatedIntensity(char *uglyStr, char *transStr);

void getTranslatedQualifier(char *uglyStr, char *transStr);

void getTranslatedRisk(int convHazCat, char *transStr);

void getTranslatedType(char *uglyStr, char *transStr);

void getTranslatedVisibility(char *uglyStr, char *transStr);

void getUserTimes(double **timeUserStart, double **timeUserEnd, 
                  int *f_POPUserStart, char *startDate, sChar TZ, 
                  double startTime, sChar f_observeDST, int *numDays, 
                  double *firstValidTime_pop, int **f_6CycleFirst, sChar f_XML, 
                  double *firstValidTimeMatch);

int isDominant(char *arg1, char *arg2, char *argType);

int isNewLayout(layouts newLayout, size_t * numLayoutSoFar,
                uChar * numCurrentLayout, int f_finalTimeLayout);

void monthDayYearTime(genMatchType * match, size_t numMatch,
                      char *currentLocalTime, char *currentDay,
                      sChar f_observeDST, double *firstMaxTValidTime_doub_adj,
                      double *currentLocalTime_doub_adj, sChar TZoffset,
		      int startNum, int endNum, numRowsInfo numRows);

void prepareDWMLgen(uChar f_XML, uChar * f_formatPeriodName, 
                    uChar **wxParameters, char *summarization,
                    uChar varFilter[NDFD_MATCHALL + 1], sChar * f_icon, 
                    size_t numPnts);

void prepareDWMLgenByDay(genMatchType *match, uChar f_XML, 
                         double *startTime_cml, double *endTime_cml,
			 double *firstValidTimeMatch, int *numDays, 
                         char *format, uChar *f_formatPeriodName,
                         uChar **wxParameters, int *timeInterval,
                         int *numOutputLines, char *summarization,
			 double currentDoubTime, size_t numPnts, 
                         PntSectInfo *pntInfo);

void prepareWeatherValuesByDay (genMatchType *match, sChar TZoffset,
		                sChar f_observeDST, char *frequency,
				int *numDays, int *numOutputLines,
				numRowsInfo numRowsWS, 
				numRowsInfo numRowsMIN,
 			        numRowsInfo numRowsMAX, uChar f_XML, 
                                numRowsInfo numRowsPOP, 
				numRowsInfo numRowsWX, size_t pnt, 
				int f_useMinTempTimes, 
				double startTime_cml,
				double *weatherDataTimes,
				int *periodMaxTemp, 
				double *periodStartTimes, 
				double *periodEndTimes,
				double *springDoubleDate, 
                                double *fallDoubleDate, 
				int *timeLayoutHour, int f_6CycleFirst, 
                                int startNum, int endNum);

void PrintDay1(genMatchType * match, size_t pntIndex, collateType *collate, 
               size_t numCollate, sChar pntTimeZone, sChar f_dayCheck);

void PrintTime(genMatchType * match, size_t pntIndex, int *allElem, 
               sChar pntTimeZone, sChar f_dayCheck);

int roundPopNearestTen(int num);

void setVarFilter(sChar f_XML, sChar f_icon, uChar *varFilter);

void skyPhrase(int *maxSkyCover, int *minSkyCover, int *averageSkyCover, 
	       int dayIndex, int f_isDayTime, int f_isNightTime, int *maxSkyNum,  
	       int *minSkyNum, int *startPositions, int *endPositions,
               char *baseURL, icon_def *iconInfo, char **phrase);

void spreadPOPsToWxTimes(int *POP12SpreadToPOP3, WX *wxInfo, int numRowsWX,
                         elem_def *popInfo, int numRowsPOP);

void tempExtremePhrase(int f_isDayTime, int *periodMaxTemp, int dayIndex, 
                       char *baseURL, icon_def *iconInfo, char **phrase);

int useNightPeriodName(char *dataTime);

void windExtremePhrase(int f_isDayTime, int f_isNightTime, int dayIndex, 
                       char *baseURL, double springDoubleDate, 
		       double fallDoubleDate, int *maxWindSpeed, 
		       int *maxWindDirection, int integerTime, 
		       int integerStartUserTime, int *periodMaxTemp, 
		       icon_def *iconInfo, char **phrase);

int XMLmatchCompare(const void *A, const void *B);

int XMLParse (uChar f_XML, size_t numPnts, Point * pnts, PntSectInfo *pntInfo,
              sChar f_pntType, char **labels, size_t numInFiles,
              char **inFiles, uChar f_fileType, sChar f_interp, sChar f_unit,
              double majEarth, double minEarth, sChar f_icon,
              sChar f_SimpleVer, sChar f_valTime, double startTime,
              double endTime, size_t numNdfdVars, uChar *ndfdVars,
              char *f_inTypes, char *gribFilter, size_t numSector,
              char ** sector, sChar f_ndfdConven);
#endif
