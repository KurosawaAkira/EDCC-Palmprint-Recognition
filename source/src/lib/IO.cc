/*************************************************************************
    > File Name: IO..cc
    > Author: Leosocy
    > Mail: 513887568@qq.com 
    > Created Time: 2017/07/26 21:29:10
 ************************************************************************/
#include <IO.h>
#include <iostream>
#include <assert.h>

#define CODING_FIELD "coding"

#define PALMPRINT_GROUP_FORMAT  "{\n\
\"identity\" : [\n\
    \"path1\"\n\
    \"path2\"\n\
    ]\n\
}"
EDCC::IO::IO()
{
    paramsSet.insert(IMAGE_SIZE_W);
    paramsSet.insert(IMAGE_SIZE_H);
    paramsSet.insert(GABOR_KERNEL_SIZE);
    paramsSet.insert(GABOR_DIRECTIONS);
    paramsSet.insert(LAPLACE_KERNEL_SIZE);

    memset(&config, 0, sizeof(config));
}

EDCC::IO::~IO()
{

}

int EDCC::IO::loadConfig(_IN ifstream &in)
{
    Json::Value root;
    Json::Reader reader;
    Json::Value::Members members;
    if(!reader.parse(in, root)) {
        EDCC_Log("IO::loadConfig Parse %s failed, please confirm the file is exists.");
        return EDCC_LOAD_CONFIG_FAIL;
    }
    members = root.getMemberNames();
    for(Json::Value::Members::iterator it = members.begin(); 
            it != members.end(); ++it) {
        if(!root[*it].isObject() || root[*it]["default"].isNull()) {
            EDCC_Log("Parse config.json failed, you can only change the value of \
                     \"default\" label in this file.");
            return EDCC_LOAD_CONFIG_FAIL;
        }
        if(paramsSet.find(*it) == paramsSet.end()
            || !genConfig(*it, root[*it]["default"].asInt())) {
            EDCC_Log("Illegal configuration parameters.");
            return EDCC_LOAD_CONFIG_FAIL;
        }
    }

    return EDCC_SUCCESS;
}

int EDCC::IO::loadPalmprintGroup(_IN ifstream &in, _INOUT vector<PalmprintCode> &groupVec)
{
    Json::Value root;
    Json::Reader reader;
    Json::Value::Members members;

    if(!reader.parse(in, root)) {
        EDCC_Log("Parse json failed. Don't change the json format. You need to confirm the format like this.");
        EDCC_Log(PALMPRINT_GROUP_FORMAT"");
        return EDCC_LOAD_TAINING_SET_FAIL;
    }
    members = root.getMemberNames();
    for(Json::Value::Members::iterator it = members.begin(); 
            it != members.end(); ++it) {
        if(!root[*it].isArray()) {
            EDCC_Log("Don't change the json format. You need to confirm the format like this.");
            EDCC_Log(PALMPRINT_GROUP_FORMAT);
            return EDCC_LOAD_TAINING_SET_FAIL;
        }
        Json::Value imageList = root[*it];
        for(size_t imageIndex = 0; imageIndex < imageList.size(); ++imageIndex) {
            if(imageList[(unsigned)imageIndex].isString()) {
                PalmprintCode newOne((*it).c_str(), imageList[(unsigned)imageIndex].asString().c_str());
                groupVec.push_back(newOne);
            }
        }
    }

    return EDCC_SUCCESS;
}

int EDCC::IO::loadPalmprintFeatureData(_IN ifstream &in, _INOUT vector<PalmprintCode> &data)
{
    Json::Value root;
    Json::Reader reader;
    Json::Value::Members members;
    if(!reader.parse(in, root)) {
        EDCC_Log("Load Palmprint Features Data failed. Don't change the json format.");
        return EDCC_LOAD_FEATURES_FAIL;
    }
    for(set<string>::iterator it = paramsSet.begin(); it != paramsSet.end(); ++it) {
        if(root[*it].isNull() || !root[*it].isInt()) {
            EDCC_Log("Load EDCC config from features data failed. Make sure json file has config.");
            return EDCC_LOAD_CONFIG_FAIL;
        }
        if(!genConfig(*it, root[*it].asInt())) {
            EDCC_Log("Load EDCC config from features data failed. Make sure json file has config.");
            return EDCC_LOAD_CONFIG_FAIL;
        }
    }
    members = root.getMemberNames();
    for(Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it) {
        if(paramsSet.find(*it) == paramsSet.end()
           && root[*it].isObject()) {
            if(!loadOneIdentityAllPalmprintFeatureData(*it, root[*it], data)) {
                EDCC_Log("Load EDCC features data failed. Make sure features data has not been changed.");
                return EDCC_LOAD_FEATURES_FAIL;
            }
        }
    }

    return EDCC_SUCCESS;
}

int EDCC::IO::savePalmprintFeatureData(_IN ofstream &out, _IN vector<PalmprintCode> &data)
{
    if(!out.is_open()) {
        EDCC_Log("Output stream can't open.");
        return EDCC_SAVE_FEATURES_FAIL;
    }
    Json::Value root;
    for(set<string>::iterator it = paramsSet.begin(); it != paramsSet.end(); ++it) {
        int value = 0;
        if(!getConfig(*it, value)) {
            EDCC_Log("If you want to train/predict, load config.json first.\
                    Or if you want incremental training/prediction, load trainData.json first.");
            return EDCC_SAVE_FEATURES_FAIL;
        } else {
            root[*it] = value;
        }
    }

    for(vector<PalmprintCode>::iterator it = data.begin(); it != data.end(); ++it) {
        insert2JsonValue(*it, root);
    }
    out << root.toStyledString();

    return EDCC_SUCCESS;
}

bool EDCC::IO::genConfig(const string &configKey, int configValue)
{
    if(configKey == IMAGE_SIZE_W) {
        config.imageSizeW = configValue;
    } else if(configKey == IMAGE_SIZE_H) {
        config.imageSizeH = configValue;
    } else if(configKey == GABOR_KERNEL_SIZE) {
        config.gaborSize = configValue;
    } else if(configKey == LAPLACE_KERNEL_SIZE) {
        config.laplaceSize = configValue;  
    } else if(configKey == GABOR_DIRECTIONS) {
        config.directions = configValue;
    } else {
        return false;
    }
    return true;
}

bool EDCC::IO::getConfig(const string &configKey, int &configValue)
{
    if(configKey == IMAGE_SIZE_W) {
        configValue = config.imageSizeW;
    } else if(configKey == IMAGE_SIZE_H) {
        configValue = config.imageSizeH;
    } else if(configKey == GABOR_KERNEL_SIZE) {
        configValue = config.gaborSize;
    } else if(configKey == LAPLACE_KERNEL_SIZE) {
        configValue = config.laplaceSize;
    } else if(configKey == GABOR_DIRECTIONS) {
        configValue = config.directions;
    } else {
        return false;
    }
    return true;
}

bool EDCC::IO::loadOneIdentityAllPalmprintFeatureData(_IN const string &identity,
                                                      _IN const Json::Value &value,
                                                      _INOUT vector<PalmprintCode> &data)
{
    Json::Value::Members imagePathMembers;
    Json::Value::Members::iterator it;
    imagePathMembers = value.getMemberNames();
    
    for(it = imagePathMembers.begin(); it != imagePathMembers.end(); ++it) {
        PalmprintCode instanceCode(identity.c_str(), (*it).c_str());
        if(getEDCCoding(value[*it], instanceCode)) {
            data.push_back(instanceCode);
        } else {
            return false;
        }
    }

    return true;
}

bool EDCC::IO::getEDCCoding(_IN const Json::Value &value, _INOUT PalmprintCode &coding)
{
    if(value.isNull() 
       || !value.isObject()
       || value[CODING_FIELD].isNull()
       || !value[CODING_FIELD].isString()) {
        EDCC_Log("Load Palmprint Features Data failed. Don't change the json format.");
        return false;
    }
    
    string codingHexStr = value[CODING_FIELD].asString();
    CHECK_EQ_RETURN(codingHexStr, "", false);
    CHECK_FALSE_RETURN(coding.decryptFromHexString(codingHexStr), false);

    return true;
}

bool EDCC::IO::insert2JsonValue(_IN PalmprintCode &code, _INOUT Json::Value &value)
{
    string identity = code.getIdentity();
    string imagePath = code.getImagePath();
    if(!value[identity].isNull() 
       && !value[identity][imagePath].isNull()) {
        EDCC_Log("identity: %s\timagepath: %s\tis already exists.", identity.c_str(), imagePath.c_str());
        return false;
    }
    Json::Value codeValue;
    CHECK_EQ_RETURN(setEDCCoding(code, codeValue), false, false);
    value[identity][imagePath] = codeValue;

    return true;
}

bool EDCC::IO::setEDCCoding(_IN PalmprintCode &coding, _INOUT Json::Value &value)
{
    string codingHexStr = coding.encryptToHexString(config);
    CHECK_EQ_RETURN(codingHexStr, "", false);
    value[CODING_FIELD] = codingHexStr;

    return true;
}

