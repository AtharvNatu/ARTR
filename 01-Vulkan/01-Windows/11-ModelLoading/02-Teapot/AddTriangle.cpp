void addTriangle(float single_vertex[3][3], float single_normal[3][3], float single_texCoord[3][2])
{
	// function declarations
	BOOL closeEnough(const float, const float, const float);
	void normalizeVector(float[3]);

	// code
	unsigned int maxElements = numFaceIndices * 3;
	const float e = 0.00001f; // How small a difference to equate

	// First thing we do is make sure the normals are unit length!
	// It's almost always a good idea to work with pre-normalized normals
	normalizeVector(single_normal[0]);
	normalizeVector(single_normal[1]);
	normalizeVector(single_normal[2]);

	// Search for match - triangle consists of three verts
	for (unsigned int i = 0; i < 3; i++)
	{
		unsigned int j = 0;
		for (j = 0; j < numVerts; j++)
		{
			// If the vertex positions are the same
			if (closeEnough(pPositions[j * 3], single_vertex[i][0], e) &&
				closeEnough(pPositions[(j * 3) + 1], single_vertex[i][1], e) &&
				closeEnough(pPositions[(j * 3) + 2], single_vertex[i][2], e) &&

				// AND the Normal is the same...
				closeEnough(pNormals[j * 3], single_normal[i][0], e) &&
				closeEnough(pNormals[(j * 3) + 1], single_normal[i][1], e) &&
				closeEnough(pNormals[(j * 3) + 2], single_normal[i][2], e) &&

				// And Texture is the same...
				closeEnough(pTexCoords[j * 2], single_texCoord[i][0], e) &&
				closeEnough(pTexCoords[(j * 2) + 1], single_texCoord[i][1], e))
			{
				// Then add the index only
				pElements[numElements] = j;
				numElements++;
				break;
			}
		}

		// No match for this vertex, add to end of list
		if (j == numVerts && numVerts < maxElements && numElements < maxElements)
		{
			pPositions[numVerts * 3] = single_vertex[i][0];
			pPositions[(numVerts * 3) + 1] = single_vertex[i][1];
			pPositions[(numVerts * 3) + 2] = single_vertex[i][2];

			pNormals[numVerts * 3] = single_normal[i][0];
			pNormals[(numVerts * 3) + 1] = single_normal[i][1];
			pNormals[(numVerts * 3) + 2] = single_normal[i][2];

			pTexCoords[numVerts * 2] = single_texCoord[i][0];
			pTexCoords[(numVerts * 2) + 1] = single_texCoord[i][1];

			pElements[numElements] = numVerts;
			numElements++;
			numVerts++;
		}
	}
}

void normalizeVector(float u[3])
{
	// function declarations
	void scaleVector(float[3], const float);
	float getVectorLength(const float[3]);

	// code
	scaleVector(u, 1.0f / getVectorLength(u));
}

void scaleVector(float v[3], const float scale)
{
	// code
	v[0] *= scale;
	v[1] *= scale;
	v[2] *= scale;
}

float getVectorLength(const float u[3])
{
	// function declarations
	float getVectorLengthSquared(const float[3]);

	// code
	return(sqrtf(getVectorLengthSquared(u)));
}

float getVectorLengthSquared(const float u[3])
{
	// code
	return((u[0] * u[0]) + (u[1] * u[1]) + (u[2] * u[2]));
}

BOOL closeEnough(const float fCandidate, const float fCompare, const float fEpsilon)
{
	// code
	return((fabs(fCandidate - fCompare) < fEpsilon));
}
