#include <vector>
#include <iostream>
#include "Utils.hpp"

class Task3
{
  private:
    const aiScene *scene;
    aiVector3D rootPosition;
    std::vector<Mesh> initial_state;

  public:
    void init()
    {
        scene = loadScene("models/Model3_X/wuson.x"); //<<<-------------Specify input file name heres

        if (scene == nullptr)
        {
            cout << "Could not read model file for Task 3." << endl;
            exit(1);
        }
        // save initial state of the mesh so that mesh transformations can be applied.
        for (int i = 0; i < scene->mNumMeshes; i++)
        {
            initial_state.push_back(Mesh(std::vector<aiVector3D>(), std::vector<aiVector3D>()));
            Mesh &newMesh = initial_state[i];
            aiMesh *mesh = scene->mMeshes[i];
            for (int j = 0; j < mesh->mNumVertices; j++)
            {
                newMesh.vertices.push_back(mesh->mVertices[j]);
                newMesh.normals.push_back(mesh->mNormals[j]);
            }
        }
    }

    void update(int millisSinceStart)
    {
        aiAnimation *anim = scene->mAnimations[0];

        double tick = fmod((millisSinceStart * anim->mTicksPerSecond) / 1000.0, anim->mDuration);

        for (uint i = 0; i < anim->mNumChannels; i++)
        {
            aiNodeAnim *node = anim->mChannels[i];

            aiMatrix4x4 rotationMatrix = get_interpolated_rotation(tick, node);
            aiMatrix4x4 positionMatrix = get_interpolated_position(tick, node);

            aiNode *skeletonNode = scene->mRootNode->FindNode(node->mNodeName);

            skeletonNode->mTransformation = positionMatrix * rotationMatrix;
        }

        for (uint idx_mesh = 0; idx_mesh < scene->mNumMeshes; idx_mesh++)
        {
            aiMesh *mesh = scene->mMeshes[idx_mesh];
            for (uint idx_vert = 0; idx_vert < mesh->mNumVertices; idx_vert++)
            {
                mesh->mVertices[idx_vert] = aiVector3D(0);
                mesh->mNormals[idx_vert] = aiVector3D(0);
            }

            for (uint idx_bone = 0; idx_bone < mesh->mNumBones; idx_bone++)
            {
                aiBone *bone = mesh->mBones[idx_bone];
                aiNode *node = scene->mRootNode->FindNode(bone->mName);

                aiMatrix4x4 boneTransform = bone->mOffsetMatrix;
                while (node != nullptr)
                {
                    boneTransform = node->mTransformation * boneTransform;
                    node = node->mParent;
                }

                aiMatrix4x4 boneTransformTranspose = boneTransform;
                boneTransformTranspose.Transpose();

                for (uint idx_weight = 0; idx_weight < bone->mNumWeights; idx_weight++)
                {
                    aiVertexWeight weight = bone->mWeights[idx_weight];
                    mesh->mVertices[weight.mVertexId] += weight.mWeight * (boneTransform * initial_state[idx_mesh].vertices[weight.mVertexId]);
                    mesh->mNormals[weight.mVertexId] += weight.mWeight * (boneTransformTranspose * initial_state[idx_mesh].normals[weight.mVertexId]);
                }
            }
        }
    }

    void display()
    {

        float pos[4] = {50, 50, 50, 1};
        glLightfv(GL_LIGHT0, GL_POSITION, pos);

        aiNode *root = this->scene->mRootNode;

        gluLookAt(4, 2, 6, rootPosition.x, rootPosition.y, rootPosition.z, 0, 1, 0);

        glRotatef(90, 1, 0, 0);
        glRotatef(-90, 0, 0, 1);

        render(this->scene, root);
    }

    void keyboard()
    {
    }

    void cleanup()
    {
    }
};
