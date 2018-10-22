#include "Utils.hpp"

class Task1
{
  private:
    const aiScene *scene;
    aiVector3D rootPosition;

  public:
    void init()
    {
        scene = loadScene("models/Model1_BVH/Boxing.bvh"); //<<<-------------Specify input file name heres

        if (scene == nullptr)
        {
            cout << "Could not read model file for Task 1." << endl;
            exit(1);
        }
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
            aiMatrix4x4 positionMatrix = get_interpolated_position(tick, node);
            // we assume that the only node with multiple position keyframes is the root node of the skeleton
            if (node->mNumPositionKeys > 1)
            {
                aiVector3D vec1(1.0f);
                vec1 *= positionMatrix;
                rootPosition = vec1;
            }

            aiNode *skeletonNode = scene->mRootNode->FindNode(node->mNodeName);

            skeletonNode->mTransformation = positionMatrix * rotationMatrix;
        }
    }

    void display()
    {

        float pos[4] = {50, 50, 50, 1};
        glLightfv(GL_LIGHT0, GL_POSITION, pos);

        aiNode *root = this->scene->mRootNode;

        gluLookAt(0, 220, -500, rootPosition.x, rootPosition.y, rootPosition.z, 0, 1, 0);

        render(this->scene, root);
    }

    void keyboard()
    {
    }

    void cleanup()
    {
        aiReleaseImport(scene);
    }
};
