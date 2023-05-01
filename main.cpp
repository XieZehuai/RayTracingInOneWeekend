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
    scenes.push_back(make_shared<random_scene>());              // 0
    scenes.push_back(make_shared<two_spheres>());               // 1
    scenes.push_back(make_shared<two_perlin_spheres>());        // 2
    scenes.push_back(make_shared<earth>());                     // 3
    scenes.push_back(make_shared<simple_light>());              // 4
    scenes.push_back(make_shared<cornell_box>());               // 5
    scenes.push_back(make_shared<cornell_smoke>());             // 6
    scenes.push_back(make_shared<the_next_week_final_scene>()); // 7
    scenes.push_back(make_shared<test_scene>());                // 8

    auto selected_scene = scenes[5];

    // 设置 std::cerr 输出浮点数时保留 2 位精度
    std::cerr << std::setiosflags(std::ios::fixed) << std::setprecision(2);

    // rendering ===================================================================================================
    multi_thread_renderer renderer(4, 4);
    // single_thread_renderer renderer;

    clock_t start = clock();
    renderer.render(selected_scene, selected_scene->lights());
    clock_t end = clock();

    // generate image ==============================================================================================
    std::string path = "../../results/";
    std::string filename = selected_scene->output_filename();
    std::ofstream output(path + filename);

    output << "P3\n"
           << selected_scene->image_width << ' ' << selected_scene->image_height << "\n255\n";

    auto frame_buffer = renderer.get_frame_buffer();
    for (size_t i = 0; i < frame_buffer.size(); i++)
    {
        write_color(output, frame_buffer[i], selected_scene->samples_per_pixel);
    }

    auto time = (end - start) / CLOCKS_PER_SEC;
    auto minutes = time / 60;
    auto seconds = time % 60;
    std::cerr << "\nDone, cost time: " << minutes << " minutes, " << seconds << " seconds";

    return 0;
}
