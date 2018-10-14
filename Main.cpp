//  ========================================================================
//  COSC422: Advanced Computer Graphics;  University of Canterbury (2018)
//
//  FILE NAME: ModelLoader.cpp
//  Uses the Assimp and DevIL libraries.
//
//	Most models are created with the z-axis as the primary axis and will
//  require a 90 degs rotation  about the x-axis for proper display.
//  Press key '1' to toggle 90 degs model rotation about x-axis on/off.
//
//  See Ex11.pdf for details.
//  ========================================================================

#include <iostream>
#include <map>
#include <GL/freeglut.h>
#include <IL/il.h>
using namespace std;

#include <assimp/cimport.h>
#include <assimp/types.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "assimp_extras.h"

const aiScene *scene = NULL;
GLuint scene_list = 0;
float angle = 0;
aiVector3D scene_min, scene_max;
bool modelRotn = true;
std::map<int, int> texIdMap;

bool loadModel(const char *fileName)
{
	scene = aiImportFile(fileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_Debone);
	if (scene == NULL)
		exit(1);
	//printSceneInfo(scene);
	//printTreeInfo(scene->mRootNode);
	//printBoneInfo(scene);
	//printAnimInfo(scene);
	get_bounding_box(scene, &scene_min, &scene_max);
	return true;
}

void loadGLTextures(const aiScene *scene)
{

	/* initialization of DevIL */
	ilInit();
	if (scene->HasTextures())
	{
		std::cout << "Support for meshes with embedded textures is not implemented" << endl;
		exit(1);
	}

	/* scan scene's materials for textures */
	/* Simplified version: Retrieves only the first texture with index 0 if present*/
	for (uint m = 0; m < scene->mNumMaterials; ++m)
	{
		aiString path; // filename

		if (scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
		{
			glEnable(GL_TEXTURE_2D);
			ILuint imageId;
			GLuint texId;
			ilGenImages(1, &imageId);
			glGenTextures(1, &texId);
			texIdMap[m] = texId;  //store tex ID against material id in a hash map
			ilBindImage(imageId); /* Binding of DevIL image name */
			ilEnable(IL_ORIGIN_SET);
			ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
			if (ilLoadImage((ILstring)path.data)) //if success
			{
				/* Convert image to RGBA */
				ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

				/* Create and load textures to OpenGL */
				glBindTexture(GL_TEXTURE_2D, texId);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH),
							 ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE,
							 ilGetData());
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				cout << "Texture:" << path.data << " successfully loaded." << endl;
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
				glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
				glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
			}
			else
			{
				cout << "Couldn't load Image: %s\n"
					 << path.data << endl;
			}
		}
	} //loop for material
}

// ------A recursive function to traverse scene graph and render each mesh----------
void render(const aiScene *sc, const aiNode *nd)
{
	aiMatrix4x4 m = nd->mTransformation;
	aiMesh *mesh;
	aiFace *face;
	GLuint texId;
	int meshIndex, materialIndex;

	aiTransposeMatrix4(&m); //Convert to column-major order
	glPushMatrix();
	glMultMatrixf((float *)&m); //Multiply by the transformation matrix for this node

	// Draw all meshes assigned to this node
	for (uint n = 0; n < nd->mNumMeshes; n++)
	{
		meshIndex = nd->mMeshes[n];		  //Get the mesh indices from the current node
		mesh = scene->mMeshes[meshIndex]; //Using mesh index, get the mesh object

		apply_material(sc->mMaterials[mesh->mMaterialIndex]); //Change opengl state to that material's properties

		if (mesh->HasNormals())
			glEnable(GL_LIGHTING);
		else
			glDisable(GL_LIGHTING);

		if (mesh->HasVertexColors(0))
		{
			glEnable(GL_COLOR_MATERIAL);
			glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
		}
		else
			glDisable(GL_COLOR_MATERIAL);

		//Get the polygons from each mesh and draw them
		for (uint k = 0; k < mesh->mNumFaces; k++)
		{
			face = &mesh->mFaces[k];
			GLenum face_mode;

			switch (face->mNumIndices)
			{
			case 1:
				face_mode = GL_POINTS;
				break;
			case 2:
				face_mode = GL_LINES;
				break;
			case 3:
				face_mode = GL_TRIANGLES;
				break;
			default:
				face_mode = GL_POLYGON;
				break;
			}

			glBegin(face_mode);

			for (uint i = 0; i < face->mNumIndices; i++)
			{
				int vertexIndex = face->mIndices[i];

				if (mesh->HasVertexColors(0))
					glColor4fv((GLfloat *)&mesh->mColors[0][vertexIndex]);

				if (mesh->HasNormals())
					glNormal3fv(&mesh->mNormals[vertexIndex].x);

				glVertex3fv(&mesh->mVertices[vertexIndex].x);
			}

			glEnd();
		}
	}

	// Draw all children
	for (uint i = 0; i < nd->mNumChildren; i++)
		render(sc, nd->mChildren[i]);

	glPopMatrix();
}

//--------------------OpenGL initialization------------------------
void initialise()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	loadModel("Model1_BVH/Boxing.bvh"); //<<<-------------Specify input file name heres
	//loadGLTextures(scene);        //<<<-------------Uncomment when implementing texturing
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 1.0, 1000.0);
}

// Find the keyframes that the animation is currently between, and produce an interpolated rotation.
aiMatrix4x4 get_interpolated_rotation(double tick, aiNodeAnim *node)
{
	aiQuatKey prevKey;
	aiQuatKey nextKey;
	for (uint frame = 0; frame < node->mNumRotationKeys; frame++)
	{
		if (tick > node->mRotationKeys[frame].mTime)
		{
			prevKey = node->mRotationKeys[frame];
			continue;
		}

		nextKey = node->mRotationKeys[frame];

		double timeDifference = nextKey.mTime - prevKey.mTime;
		double currentTime = tick - prevKey.mTime;
		double timeLerp = currentTime / timeDifference;

		aiQuaternion outRotation;
		aiQuaternion::Interpolate(outRotation, prevKey.mValue, nextKey.mValue, timeLerp);
		return aiMatrix4x4(outRotation.GetMatrix());
	}
	// if there is only 1 keyframe, return its value
	return aiMatrix4x4(prevKey.mValue.GetMatrix());
}

// Find the keyframes that the animation is currently between, and produce an interpolated rotation.
aiMatrix4x4 get_interpolated_position(double tick, aiNodeAnim *node)
{
	aiVectorKey prevKey;
	aiVectorKey nextKey;
	for (uint frame = 0; frame < node->mNumPositionKeys; frame++)
	{
		if (tick > node->mPositionKeys[frame].mTime)
		{
			prevKey = node->mPositionKeys[frame];
			continue;
		}

		nextKey = node->mPositionKeys[frame];

		double timeDifference = nextKey.mTime - prevKey.mTime;
		double currentTime = tick - prevKey.mTime;
		double timeLerp = currentTime / timeDifference;

		aiVector3D outPosition;
		outPosition = prevKey.mValue + (nextKey.mValue - prevKey.mValue) * float(timeLerp);

		aiMatrix4x4 positionMatrix;
		aiMatrix4x4::Translation(outPosition, positionMatrix);
		return positionMatrix;
	}
	// if there is only 1 keyframe, return its value
	aiMatrix4x4 positionMatrix;
	aiMatrix4x4::Translation(prevKey.mValue, positionMatrix);
	return positionMatrix;
}

//----Timer callback for continuous rotation of the model about y-axis----
void update(int millisSinceStart)
{
	aiAnimation *anim = scene->mAnimations[0];

	double tick = fmod((millisSinceStart * anim->mTicksPerSecond) / 1000.0, anim->mDuration);

	for (uint i = 0; i < anim->mNumChannels; i++)
	{
		aiNodeAnim *node = anim->mChannels[i];

		aiMatrix4x4 rotationMatrix = get_interpolated_rotation(tick, node);
		aiMatrix4x4 postitionMatrix = get_interpolated_position(tick, node);

		aiNode *skeletonNode = scene->mRootNode->FindNode(node->mNodeName);

		skeletonNode->mTransformation = postitionMatrix * rotationMatrix;
	}

	glutPostRedisplay();
	glutTimerFunc(10, update, millisSinceStart + 10);
}

//----Keyboard callback to toggle initial model orientation---
void keyboard(unsigned char key, int x, int y)
{
	if (key == '1')
		modelRotn = !modelRotn; //Enable/disable initial model rotation
	glutPostRedisplay();
}

//------The main display function---------
//----The model is first drawn using a display list so that all GL commands are
//    stored for subsequent display updates.
void display()
{
	float pos[4] = {50, 50, 50, 1};
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 3, 0, 0, -5, 0, 1, 0);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);

	// scale the whole asset to fit into our view frustum
	float tmp = scene_max.x - scene_min.x;
	tmp = aisgl_max(scene_max.y - scene_min.y, tmp);
	tmp = aisgl_max(scene_max.z - scene_min.z, tmp);
	tmp = 1.f / tmp;
	glScalef(tmp, tmp, tmp);

	render(scene, scene->mRootNode);

	glutSwapBuffers();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Model Loader");
	glutInitContextVersion(4, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	initialise();
	glutDisplayFunc(display);
	glutTimerFunc(50, update, 50);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	aiReleaseImport(scene);
}
