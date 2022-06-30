#include <iostream>
#include <vector>
#include <ctime>
#include <fstream>
#include <iomanip>

#include "rtweekend.h"

#include "color.h"
#include "bvh.h"

#include "scene_generator.h"
#include "renderer.h"

int main()
{
    std::vector<shared_ptr<scene_generator>> scenes;
    scenes.push_back(make_shared<random_scene>());
    scenes.push_back(make_shared<two_spheres>());
    scenes.push_back(make_shared<two_perlin_spheres>());
    scenes.push_back(make_shared<earth>());
    scenes.push_back(make_shared<simple_light>());
    scenes.push_back(make_shared<cornell_box>());

    const int scene_index = 2;

    std::cerr << std::setiosflags(std::ios::fixed) << std::setprecision(2);

    // rendering ===================================================================================================
    // multi_thread_renderer renderer(4, 4);
    single_thread_renderer renderer;

    clock_t start = clock();
    renderer.render(scenes[scene_index]);
    clock_t end = clock();

    // generate image ==============================================================================================
    std::ofstream output(scenes[scene_index]->output_filename());
    output << "P3\n"
           << scenes[scene_index]->image_width << ' ' << scenes[scene_index]->image_height << "\n255\n";

    auto frame_buffer = renderer.get_frame_buffer();
    for (int i = 0; i < frame_buffer.size(); i++)
    {
        write_color(output, frame_buffer[i], scenes[scene_index]->samples_per_pixel);
    }

    std::cerr << "\nDone, cost time: " << ((end - start) / CLOCKS_PER_SEC) << " seconds";

    return 0;
}
