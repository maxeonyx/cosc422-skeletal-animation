#include "Utils.hpp"

class Task1
{
  private:
    const aiScene *scene;
    aiVector3D rootPosition;

  public:
    void init()
    {
        scene = aiImportFile("models/Model1_BVH/Boxing.bvh", aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_Debone); //<<<-------------Specify input file name heres

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
        float pos[4] = {-400, 620, 100, 1};
        glLightfv(GL_LIGHT0, GL_POSITION, pos);

        aiNode *root = this->scene->mRootNode;

        gluLookAt(200, 220, -500, rootPosition.x, rootPosition.y - 50, rootPosition.z, 0, 1, 0);

        render_stepped_pyramid();
        glColor3f(0.1, 0.1, 0.6);
        render(this->scene, root, std::map<int, int>());
        glPushMatrix();
        glColor3f(1, 1, 1);
        glScalef(1, 0.01, 1);
        render(this->scene, root, std::map<int, int>());
        glPopMatrix();
    }

    void keyboard()
    {
    }

    void cleanup()
    {
        aiReleaseImport(scene);
    }

  private:
    void render_stepped_pyramid()
    {
        glPushMatrix();
        glEnable(GL_COLOR_MATERIAL);
        glColor3f(0.4, 0.4, 0.4);
        glScalef(100, 100, 100); // scale everything to metres
        glTranslatef(-0.2, 0, 0.2);
        const int num_levels = 6;
        float waist_height = 0.5;
        float level_height = 0.6;
        float top_level_width = 2;
        float step_width = 0.8;
        for (int level = 0; level < num_levels; level++)
        {
            float platform_width = top_level_width + step_width * level;
            float platform_y_pos = -waist_height - level_height * level;
            glPushMatrix();
            glTranslatef(0, platform_y_pos, 0);
            glScalef(platform_width, 1, platform_width);
            glutSolidCube(1);
            glPopMatrix();
        }
        for (int side = 0; side < 4; side++)
        {
            float rail_length = sqrt(step_width * step_width + level_height * level_height) * num_levels - 0.66;

            float rail_position_x = (top_level_width / 2) + ((step_width / 2) * num_levels) / 2 + 0.2;
            float rail_position_y = -waist_height - (level_height * num_levels) / 2;
            glPushMatrix();
            glRotatef(90 * side, 0, 1, 0);
            glTranslatef(rail_position_x, rail_position_y, 0);
            glRotatef(-(180 / 3.14159665) * atan(step_width / level_height), 0, 0, 1);
            glScalef(rail_length, 0.5, 0.5);
            glutSolidCube(1);
            glPopMatrix();
        }
        glColor3f(0.5, 1, 0.4);
        glPushMatrix();
        glTranslatef(0, -7, 0);
        glScalef(300, 5, 300);
        glutSolidCube(1);
        glPopMatrix();
        glPopMatrix();
    }
};
