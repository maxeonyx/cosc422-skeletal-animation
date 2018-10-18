#include "Utils.hpp"

class Task1
{
  private:
    const aiScene *scene;

    aiVector3D scene_min;
    aiVector3D scene_max;

  public:
    void init()
    {
        scene = loadScene("models/Model1_BVH/Boxing.bvh"); //<<<-------------Specify input file name heres

        get_bounding_box(scene, &scene_min, &scene_max);

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
            aiMatrix4x4 postitionMatrix = get_interpolated_position(tick, node);

            aiNode *skeletonNode = scene->mRootNode->FindNode(node->mNodeName);

            skeletonNode->mTransformation = postitionMatrix * rotationMatrix;
        }
    }

    void display()
    {
        gluLookAt(0, 0, 3, 0, 0, -5, 0, 1, 0);

        float pos[4] = {50, 50, 50, 1};
        glLightfv(GL_LIGHT0, GL_POSITION, pos);

        // scale the whole asset to fit into our view frustum
        float tmp = scene_max.x - scene_min.x;
        tmp = aisgl_max(scene_max.y - scene_min.y, tmp);
        tmp = aisgl_max(scene_max.z - scene_min.z, tmp);
        tmp = 1.f / tmp;
        glScalef(tmp, tmp, tmp);

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
