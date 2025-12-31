#ifndef LOAD_MESH_H
#define LOAD_MESH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
FILE* gpMeshFile = NULL;

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

int loadMesh(void)
{
    // Variable Declarations
    char *space = " ", *slash = "/", *firstToken = NULL, *token = NULL;
    char *faceEntries[3] = { NULL, NULL, NULL };
    int numPositionCords = 0, numTexcoords = 0, numNormalCords = 0, numFaces = 0;

    // Code
    gpMeshFile = fopen("../Assets/Models/Monkey-Head.obj.model", "r");
    if (gpMeshFile == NULL)
        return FAILURE;

    gpVertex = createFloatVector();
    gpTexture = createFloatVector();
    gpNormal = createFloatVector();

    gpVertexIndices = createIntVector();
    gpTextureIndices = createIntVector();
    gpNormalIndices = createIntVector();

    while(fgets(buffer, BUFFER_SIZE, gpMeshFile) != NULL)
    {
        firstToken = strtok(buffer, space);

        if (strcmp(firstToken, "v") == 0)
        {
            numPositionCords++;
            while ((token = strtok(NULL, space)) != NULL)
                pushBackToFloatVector(gpVertex, atof(token));
        }

        else if (strcmp(firstToken, "vt") == 0)
        {
            numTexcoords++;
            while ((token = strtok(NULL, space)) != NULL)
                pushBackToFloatVector(gpTexture, atof(token));
        }

        else if (strcmp(firstToken, "vn") == 0)
        {
            numNormalCords++;
            while ((token = strtok(NULL, space)) != NULL)
                pushBackToFloatVector(gpNormal, atof(token));
        }

        else if (strcmp(firstToken, "f") == 0)
        {
            numFaces++;
            for (int i = 0; i < 3; i++)
                faceEntries[i] = strtok(NULL, space);

            for (int i = 0; i < 3; i++)
            {
                token = strtok(faceEntries[i], slash);
                pushBackToIntVector(gpVertexIndices, atoi(token) - 1);
                token = strtok(NULL, slash);
                pushBackToIntVector(gpTextureIndices, atoi(token) - 1);
                token = strtok(NULL, slash);
                pushBackToIntVector(gpNormalIndices, atoi(token) - 1);
            }
        }
    }

    fclose(gpMeshFile);
    gpMeshFile = NULL;

    return SUCCESS;

}


#endif // LOAD_MESH_H