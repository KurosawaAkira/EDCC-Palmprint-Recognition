/*************************************************************************
	> File Name: EDCC.h
	> Author: Leosocy
	> Mail: 513887568@qq.com 
	> Created Time: 2017/10/5 14:32:50
 ************************************************************************/
#ifndef __EDCC_H__
#define __EDCC_H__

#include <string>
#include <map>

#ifdef _WINDOWS

#ifdef EDCC_USER
#define EDCC_API __declspec(dllimport)
#else
#define EDCC_API __declspec(dllexport)
#endif

#else
#define EDCC_API

#endif

#define EDCC_SUCCESS 0
#define EDCC_NULL_POINTER_ERROR 1

#define EDCC_LOAD_CONFIG_FAIL 100

#define EDCC_LOAD_TAINING_SET_FAIL 200

#define EDCC_LOAD_FEATURES_FAIL 300
#define EDCC_SAVE_FEATURES_FAIL 301

#define EDCC_LOAD_PALMPRINT_IMAGE_FAIL 400

#define EDCC_CODING_BUFF_LEN_NOT_ENOUGH 500
#define EDCC_CODING_INVALID 501
#define EDCC_CODINGS_DIFF_CONFIG 502

#define _IN
#define _OUT
#define _INOUT

EDCC_API typedef struct  {
    std::string identity;
    std::string imagePath;
    double score;
    size_t rank;
} MatchResult;

#ifdef __cplusplus
extern "C"
{
#endif
    /* Some primitive operation */
    EDCC_API int GetEDCCCoding(_IN const char *palmprintImagePath,
                               _IN const char *configFileName,
                               _INOUT unsigned char *pCodingBuf,
                               _IN size_t bufMaxLen,
                               _OUT size_t &bufLen);

    EDCC_API int GetTwoPalmprintCodingMatchScore(_IN const unsigned char *firstPalmprintCodingBuf,
                                                 _IN const unsigned char *secondPalmprintCodingBuf,
                                                 _OUT double &score);

    EDCC_API int GetTwoPalmprintMatchScore(_IN const char *firstPalmprintImagePath,
                                           _IN const char *secondPalmprintImagePath,
                                           _IN const char *configFileName,
                                           _OUT double &score);
    /* primitive operation end */



    /* Some encapsulated APIs which can be used in small-scale data. */
    EDCC_API int GetTrainingSetFeatures(_IN const char *trainingSetPalmprintGroupFileName,
                                        _IN const char *configFileName,
                                        _IN const char *featuresOutputFileName,
                                        _IN bool isIncremental = false);

    EDCC_API int GetTopKMatchScore(_IN const char *palmprintImagePath,
                                  _IN const char *trainingSetFeaturesOrPalmprintGroupFileName,
                                  _IN const char *configFileName,
                                  _IN bool isFeatures,
                                  _IN size_t K,
                                  _OUT std::map<size_t, MatchResult> &topKResult);
    /* encapsulated APIs end */

#ifdef __cplusplus
}
#endif

#endif
