#ifndef LOAD_MESH_H
#define LOAD_MESH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For Android Asset Manager
#include <android_native_app_glue.h>   

typedef struct 
{
    int* ptr;
    size_t size;
} vecInt;

typedef struct 
{
    float* ptr;
    size_t size;
} vecFloat;

#define SUCCESS         1
#define FAILURE         -1
#define BUFFER_SIZE     1024

char buffer[BUFFER_SIZE];

vecFloat *gpVertex = NULL, *gpTexture = NULL, *gpNormal = NULL;
vecInt *gpVertexIndices = NULL, *gpTextureIndices = NULL, *gpNormalIndices = NULL;

vecInt* createIntVector(void)
{
    // Code
    vecInt* ptr = (vecInt*)malloc(sizeof(vecInt));
    memset((void*)ptr, 0, sizeof(vecInt));
    return ptr;
}

vecFloat* createFloatVector(void)
{
    // Code
    vecFloat* ptr = (vecFloat*)malloc(sizeof(vecFloat));
    memset((void*)ptr, 0, sizeof(vecFloat));
    return ptr;
}

int pushBackToIntVector(vecInt* ptrVecInt, int data)
{
    // Code
    ptrVecInt->ptr = (int*)realloc(ptrVecInt->ptr, (ptrVecInt->size + 1) * sizeof(int));
    if (ptrVecInt->ptr == NULL)
        return FAILURE;

    ptrVecInt->size = ptrVecInt->size + 1;
    ptrVecInt->ptr[ptrVecInt->size - 1] = data;

    return SUCCESS;
}

int pushBackToFloatVector(vecFloat* ptrVecFloat, float data)
{
    // Code
    ptrVecFloat->ptr = (float*)realloc(ptrVecFloat->ptr, (ptrVecFloat->size + 1) * sizeof(float));
    if (ptrVecFloat->ptr == NULL)
        return FAILURE;

    ptrVecFloat->size = ptrVecFloat->size + 1;
    ptrVecFloat->ptr[ptrVecFloat->size - 1] = data;

    return SUCCESS;
}

int destroyIntVector(vecInt* ptrVecInt)
{
    // Code
    if (ptrVecInt->ptr)
    {
        free(ptrVecInt->ptr);
        free(ptrVecInt);
        return SUCCESS;
    }

    return FAILURE;
}

int destroyFloatVector(vecFloat* ptrVecFloat)
{
    // Code
    if (ptrVecFloat->ptr)
    {
        free(ptrVecFloat->ptr);
        free(ptrVecFloat);
        return SUCCESS;
    }

    return FAILURE;
}

int loadMesh(AAssetManager **assetManager)
{
    // Variable Declarations
    const char *space = " ";
    const char *slash = "/";
    char *firstToken = nullptr;
    char *token = nullptr;
    char *line = nullptr;

    char *lineSavePtr = NULL, *tokenSavePtr = NULL;
    char *faceEntries[3] = { NULL, NULL, NULL };
    int numPositionCords = 0, numTexcoords = 0, numNormalCords = 0, numFaces = 0;

    // Code
    AAsset *modelAsset = AAssetManager_open(*assetManager, "Monkey-Head.obj.model", AASSET_MODE_BUFFER);
    if (modelAsset == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Failed To Open Model Asset !!!", __func__);
        return -1;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Succeeded In Opening Model Asset\n", __func__);
    
    size_t size = AAsset_getLength(modelAsset);

    char* modelData = (char*)malloc(size + 1);
    if (modelData == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => malloc() Failed For modelData !!!\n", __func__);
        return -1;
    }
    modelData[size] = '\0'; 

    size_t retVal = AAsset_read(modelAsset, modelData, size);
    if (retVal < 0)
    {
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Failed To Read Model Data !!!", __func__);
        return -1;
    }
    else
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Successfully Read Read Model Data\n", __func__);
    
    if (modelAsset)
    {
        AAsset_close(modelAsset); 
        modelAsset = NULL;
        __android_log_print(ANDROID_LOG_INFO, "ADN:", "%s() => Closed Model File\n", __func__);
    }

    gpVertex = createFloatVector();
    gpTexture = createFloatVector();
    gpNormal = createFloatVector();

    gpVertexIndices = createIntVector();
    gpTextureIndices = createIntVector();
    gpNormalIndices = createIntVector();

    line = strtok_r(modelData, "\n", &lineSavePtr);
    while(line != NULL)
    {
        firstToken = strtok_r(line, space, &tokenSavePtr);

        if (firstToken != NULL) 
        {
            if (strcmp(firstToken, "v") == 0)
            {
                numPositionCords++;
                while ((token = strtok_r(NULL, space, &tokenSavePtr)) != NULL)
                    pushBackToFloatVector(gpVertex, atof(token));
            }

            else if (strcmp(firstToken, "vt") == 0)
            {
                numTexcoords++;
                while ((token = strtok_r(NULL, space, &tokenSavePtr)) != NULL)
                    pushBackToFloatVector(gpTexture, atof(token));
            }

            else if (strcmp(firstToken, "vn") == 0)
            {
                numNormalCords++;
                while ((token = strtok_r(NULL, space, &tokenSavePtr)) != NULL)
                    pushBackToFloatVector(gpNormal, atof(token));
            }

            else if (strcmp(firstToken, "f") == 0)
            {
                numFaces++;
                for (int i = 0; i < 3; i++)
                {
                    token = strtok_r(NULL, space, &tokenSavePtr);
                    if (token) 
                    {
                        int v, t, n;
                        if (sscanf(token, "%d/%d/%d", &v, &t, &n) == 3) 
                        {
                            pushBackToIntVector(gpVertexIndices, v - 1);
                            pushBackToIntVector(gpTextureIndices, t - 1);
                            pushBackToIntVector(gpNormalIndices, n - 1);
                        }
                    }
                }
            }
        }

        line = strtok_r(NULL, "\n", &lineSavePtr);
    }

    if (modelData)
    {
        free(modelData);
        modelData = NULL;
    }

    return SUCCESS;

}


#endif // LOAD_MESH_H