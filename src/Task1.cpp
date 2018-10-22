#include "Utils.hpp"

class Task1
{
  private:
    const aiScene *scene;

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

            aiNode *skeletonNode = scene->mRootNode->FindNode(node->mNodeName);

            skeletonNode->mTransformation = positionMatrix * rotationMatrix;
        }
    }

    void display()
    {
        gluLookAt(0, 0, 3, 0, 0, -5, 0, 1, 0);

        float pos[4] = {50, 50, 50, 1};
        glLightfv(GL_LIGHT0, GL_POSITION, pos);

        aiNode *root = this->scene->mRootNode;

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
