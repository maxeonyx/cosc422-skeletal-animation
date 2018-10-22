#include <vector>
#include <iostream>

class Task2
{
  private:
    const aiScene *scene;
    const aiScene *sceneJump;
    aiVector3D scene_min;
    aiVector3D scene_max;
    aiVector3D rootPosition;
    std::vector<Mesh> initial_state;

  public:
    void init()
    {
        scene = loadScene("models/Model2_FBX/mannequin.fbx"); //<<<-------------Specify input file name heres
        sceneJump = loadScene("models/Model2_FBX/jump.fbx");

        if (scene == nullptr)
        {
            cout << "Could not read model file for Task 2." << endl;
            exit(1);
        }

        initial_state = std::vector<Mesh>();
        // save initial state of the mesh so that mesh transformations can be applied.
        for (int i = 0; i < scene->mNumMeshes; i++)
        {
            Mesh newMesh = Mesh(std::vector<aiVector3D>(), std::vector<aiVector3D>());
            aiMesh *mesh = scene->mMeshes[i];
            for (int j = 0; j < mesh->mNumVertices; j++)
            {
                newMesh.vertices.push_back(mesh->mVertices[j]);
                newMesh.normals.push_back(mesh->mNormals[j]);
            }
            initial_state.push_back(newMesh);
        }
    }

    void update(int millisSinceStart)
    {
        aiAnimation *anim = sceneJump->mAnimations[0];

        double tick = fmod((millisSinceStart * anim->mTicksPerSecond) / 1000.0, anim->mDuration);

        for (uint i = 0; i < anim->mNumChannels; i++)
        {
            aiNodeAnim *node = anim->mChannels[i];

            aiMatrix4x4 rotationMatrix = get_interpolated_rotation(tick, node);
            aiMatrix4x4 positionMatrix = get_interpolated_position(tick, node);
            // we assume the only node with multiple position keyframes is the root node
            if (node->mNumPositionKeys > 1)
            {
                positionMatrix = aiMatrix4x4();
                //std::cout << node->mNodeName.C_Str() << " has multiple position keyframes" << std::endl;
                aiVector3D vec1(1.0f);
                vec1 *= positionMatrix;
                rootPosition = vec1;
            }

            aiNode *skeletonNode = scene->mRootNode->FindNode(node->mNodeName);
            if (skeletonNode == nullptr)
            {
                std::cout << "could not find a node for the animation channel" << std::endl;
            }

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
                if (node == nullptr)
                {
                    std::cout << "could not find a node for the animation channel" << std::endl;
                }

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
                    mesh->mVertices[weight.mVertexId] = 1.0f * (boneTransform * initial_state[idx_mesh].vertices[weight.mVertexId]);
                    mesh->mNormals[weight.mVertexId] = 1.0f * (boneTransformTranspose * initial_state[idx_mesh].normals[weight.mVertexId]);
                }
            }
        }
        //get_bounding_box(scene, &scene_min, &scene_max);
    }

    void display()
    {
        float pos[4] = {50, 50, 50, 1};
        glLightfv(GL_LIGHT0, GL_POSITION, pos);

        aiNode *root = this->scene->mRootNode;

        std::cout << rootPosition.x << " " << rootPosition.y << " " << rootPosition.z << std::endl;

        // scale the whole asset to fit into our view frustum
        // float tmp = scene_max.x - scene_min.x;
        // tmp = aisgl_max(scene_max.y - scene_min.y, tmp);
        // tmp = aisgl_max(scene_max.z - scene_min.z, tmp);
        // tmp = 1.f / tmp;
        // glScalef(tmp, tmp, tmp);
        gluLookAt(-0.5, 6.5, 8.5, -0.5, 2.5, 1.5, 0, 1, 0);

        render_only_meshes(this->scene, root);
    }

    void keyboard()
    {
    }

    void cleanup()
    {
    }
};
